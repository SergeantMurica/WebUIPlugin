#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameFramework/SaveGame.h" // <-- Include the base class
#include "WebUIGameInstance.generated.h"

class UWebUIWidget;

UCLASS()
class WEBUIPROJECT_API UWebUIGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	// This is where we will tell Unreal which Blueprint to use.
	// We will set this in the editor.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SaveLoad")
	TSubclassOf<USaveGame> SaveGameClass;

	/** Called by the Player Controller to give the Game Instance a way to talk to the UI. */
	UFUNCTION(BlueprintCallable, Category = "WebUI")
	void RegisterWebUIWidget(UWebUIWidget* InWebUIWidget);

	/** Finds all save games and sends the list back to the UI. */
	UFUNCTION(BlueprintCallable, Category = "SaveLoad")
	void RequestSaveGamesList();

	/** Creates a new save file in the specified slot. */
	UFUNCTION(BlueprintCallable, Category = "SaveLoad")
	void SaveGame(const FString& SlotName);

	/** Loads a game from the specified slot and transitions to the level. */
	UFUNCTION(BlueprintCallable, Category = "SaveLoad")
	void LoadGame(const FString& SlotName);

	// This holds the save data after loading, so the new level can use it.
	// It now correctly uses the generic base class.
	UPROPERTY(BlueprintReadOnly, Category = "SaveLoad")
	TObjectPtr<USaveGame> CurrentSaveData;

	// --- Keep the other functions ---
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ApplyAllGameSettings(const FString& SettingsJsonString);
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void StartNewGame(FName LevelName);
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void QuitGame();

private:
	TWeakObjectPtr<UWebUIWidget> ActiveWebUIWidget;
};