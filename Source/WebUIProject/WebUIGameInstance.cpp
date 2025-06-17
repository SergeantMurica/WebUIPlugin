
#include "WebUIGameInstance.h"
#include "WebUIWidget.h" 
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameUserSettings.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Kismet/KismetSystemLibrary.h"


#define SAVE_GAME_BLUEPRINT_PATH TEXT("/Game/BP_SaveGameObject.BP_SaveGameObject_C")

void UWebUIGameInstance::RegisterWebUIWidget(UWebUIWidget* InWebUIWidget)
{
	ActiveWebUIWidget = InWebUIWidget;
	
	// FIX: Bind this game instance to JavaScript as 'ue'
	if (InWebUIWidget)
	{
		InWebUIWidget->BindUObjectToJavaScript(TEXT("ue"), this);
	}
}

void UWebUIGameInstance::RequestSaveGamesList()
{
	TArray<TSharedPtr<FJsonValue>> SaveSlotsJsonArray;
	const int32 MaxSaveSlots = 5;

	for (int32 i = 0; i < MaxSaveSlots; ++i)
	{
		const FString SlotName = FString::Printf(TEXT("SaveSlot_%d"), i);
		TSharedPtr<FJsonObject> SlotJson = MakeShareable(new FJsonObject());
		SlotJson->SetStringField(TEXT("slotName"), SlotName);

		if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
		{
			USaveGame* LoadedSlot = UGameplayStatics::LoadGameFromSlot(SlotName, 0);
			
			// To get specific properties, we must use reflection since C++ doesn't know the BP class.
			if (FProperty* PlayerNameProp = LoadedSlot->GetClass()->FindPropertyByName(TEXT("PlayerName")))
			{
				if (FStrProperty* StrProp = CastField<FStrProperty>(PlayerNameProp))
				{
					FString PlayerName = StrProp->GetPropertyValue_InContainer(LoadedSlot);
					SlotJson->SetStringField(TEXT("playerName"), PlayerName);
				}
			}
			if (FProperty* TimestampProp = LoadedSlot->GetClass()->FindPropertyByName(TEXT("SaveTimestamp")))
			{
				if (FStructProperty* StructProp = CastField<FStructProperty>(TimestampProp))
				{
					FDateTime Timestamp = *StructProp->ContainerPtrToValuePtr<FDateTime>(LoadedSlot);
					SlotJson->SetStringField(TEXT("timestamp"), Timestamp.ToString());
				}
			}
			SlotJson->SetBoolField(TEXT("exists"), true);
		}
		else
		{
			SlotJson->SetBoolField(TEXT("exists"), false);
			SlotJson->SetStringField(TEXT("playerName"), TEXT("Empty Slot"));
			SlotJson->SetStringField(TEXT("timestamp"), TEXT(""));
		}
		SaveSlotsJsonArray.Add(MakeShareable(new FJsonValueObject(SlotJson)));
	}

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(SaveSlotsJsonArray, Writer);

	// FIX: Check if the weak pointer is valid and get the object before using it.
	if (UWebUIWidget* Widget = ActiveWebUIWidget.Get())
	{
		const FString JSScript = FString::Printf(TEXT("window.populateSaveSlots(%s);"), *JsonString);
		Widget->ExecuteJavaScript(JSScript);
	}
}

void UWebUIGameInstance::SaveGame(const FString& SlotName)
{
	if (!SaveGameClass)
	{
		UE_LOG(LogTemp, Error, TEXT("SaveGameClass is not set in the GameInstance Blueprint!"));
		return;
	}

	USaveGame* SaveGameObject = UGameplayStatics::CreateSaveGameObject(SaveGameClass);
	if (!SaveGameObject) return;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn) return;

	// FIX: Use reflection to set properties on the generic USaveGame object.
	if (FProperty* PlayerNameProp = SaveGameObject->GetClass()->FindPropertyByName(TEXT("PlayerName")))
	{
		if (FStrProperty* StrProp = CastField<FStrProperty>(PlayerNameProp))
		{
			StrProp->SetPropertyValue_InContainer(SaveGameObject, TEXT("PlayerOne"));
		}
	}
	if (FProperty* LevelNameProp = SaveGameObject->GetClass()->FindPropertyByName(TEXT("LevelName")))
	{
		if (FNameProperty* NameProp = CastField<FNameProperty>(LevelNameProp))
		{
			NameProp->SetPropertyValue_InContainer(SaveGameObject, FName(*UGameplayStatics::GetCurrentLevelName(this)));
		}
	}
	if (FProperty* TimestampProp = SaveGameObject->GetClass()->FindPropertyByName(TEXT("SaveTimestamp")))
	{
		if (FStructProperty* StructProp = CastField<FStructProperty>(TimestampProp))
		{
			*StructProp->ContainerPtrToValuePtr<FDateTime>(SaveGameObject) = FDateTime::Now();
		}
	}
	if (FProperty* TransformProp = SaveGameObject->GetClass()->FindPropertyByName(TEXT("PlayerTransform")))
	{
		if (FStructProperty* StructProp = CastField<FStructProperty>(TransformProp))
		{
			*StructProp->ContainerPtrToValuePtr<FTransform>(SaveGameObject) = PlayerPawn->GetActorTransform();
		}
	}

	UGameplayStatics::SaveGameToSlot(SaveGameObject, SlotName, 0);
	UE_LOG(LogTemp, Log, TEXT("Game saved to slot: %s"), *SlotName);
	RequestSaveGamesList();
}

void UWebUIGameInstance::LoadGame(const FString& SlotName)
{
	CurrentSaveData = UGameplayStatics::LoadGameFromSlot(SlotName, 0);

	if (CurrentSaveData)
	{
		// FIX: Use reflection to get the LevelName property correctly.
		if (FProperty* LevelNameProp = CurrentSaveData->GetClass()->FindPropertyByName(TEXT("LevelName")))
		{
			if (const FNameProperty* NameProp = CastField<const FNameProperty>(LevelNameProp))
			{
				const FName LevelToLoad = NameProp->GetPropertyValue_InContainer(CurrentSaveData);
				UGameplayStatics::OpenLevel(this, LevelToLoad);
				return;
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("Could not find LevelName property in SaveGame object."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load game from slot: %s"), *SlotName);
	}
}

void UWebUIGameInstance::ApplyAllGameSettings(const FString& SettingsJsonString)
{
	UE_LOG(LogTemp, Log, TEXT("ApplyAllGameSettings called with: %s"), *SettingsJsonString);
	
	UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!Settings) 
	{
		UE_LOG(LogTemp, Error, TEXT("Could not get GameUserSettings"));
		return;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(SettingsJsonString);
	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid()) 
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse settings JSON"));
		return;
	}

	int32 Quality;
	if (JsonObject->TryGetNumberField(TEXT("textureQuality"), Quality)) 
	{
		Settings->SetTextureQuality(Quality);
		UE_LOG(LogTemp, Log, TEXT("Set texture quality to: %d"), Quality);
	}
	if (JsonObject->TryGetNumberField(TEXT("shadowQuality"), Quality)) 
	{
		Settings->SetShadowQuality(Quality);
		UE_LOG(LogTemp, Log, TEXT("Set shadow quality to: %d"), Quality);
	}
	if (JsonObject->TryGetNumberField(TEXT("postProcessQuality"), Quality)) 
	{
		Settings->SetPostProcessingQuality(Quality);
		UE_LOG(LogTemp, Log, TEXT("Set post process quality to: %d"), Quality);
	}
	if (JsonObject->TryGetNumberField(TEXT("antiAliasingQuality"), Quality)) 
	{
		Settings->SetAntiAliasingQuality(Quality);
		UE_LOG(LogTemp, Log, TEXT("Set anti-aliasing quality to: %d"), Quality);
	}

	bool bVsync;
	if (JsonObject->TryGetBoolField(TEXT("vsync"), bVsync)) 
	{
		Settings->SetVSyncEnabled(bVsync);
		UE_LOG(LogTemp, Log, TEXT("Set VSync to: %s"), bVsync ? TEXT("true") : TEXT("false"));
	}

	int32 ResX, ResY;
	if (JsonObject->TryGetNumberField(TEXT("resolutionX"), ResX) && JsonObject->TryGetNumberField(TEXT("resolutionY"), ResY))
	{
		Settings->SetScreenResolution(FIntPoint(ResX, ResY));
		UE_LOG(LogTemp, Log, TEXT("Set resolution to: %dx%d"), ResX, ResY);
	}

	int32 FullscreenMode;
	if (JsonObject->TryGetNumberField(TEXT("fullscreenMode"), FullscreenMode))
	{
		Settings->SetFullscreenMode(static_cast<EWindowMode::Type>(FullscreenMode));
		UE_LOG(LogTemp, Log, TEXT("Set fullscreen mode to: %d"), FullscreenMode);
	}

	Settings->ConfirmVideoMode();
	Settings->ApplySettings(false);
	Settings->SaveSettings();
	
	UE_LOG(LogTemp, Log, TEXT("Settings applied and saved successfully"));
}

void UWebUIGameInstance::StartNewGame(FName LevelName)
{
	UE_LOG(LogTemp, Log, TEXT("StartNewGame called with level: %s"), *LevelName.ToString());
	UGameplayStatics::OpenLevel(this, LevelName);
}

void UWebUIGameInstance::QuitGame()
{
	UE_LOG(LogTemp, Log, TEXT("QuitGame called"));
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}