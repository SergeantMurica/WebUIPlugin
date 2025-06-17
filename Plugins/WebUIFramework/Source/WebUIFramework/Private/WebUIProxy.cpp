#include "WebUIProxy.h"
#include "Engine/GameInstance.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

void UWebUIProxy::SetGameInstance(UGameInstance* InGameInstance)
{
	GameInstancePtr = InGameInstance;
}

void UWebUIProxy::CallGameInstanceFunction(const FString& FunctionName, const FString& ParamsJson)
{
	if (UGameInstance* GI = GameInstancePtr.Get())
	{
		if (UFunction* Function = GI->FindFunction(*FunctionName))
		{
			void* Parms = FMemory_Alloca(Function->ParmsSize);
			FMemory::Memzero(Parms, Function->ParmsSize);

			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ParamsJson);
			if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
			{
				for (TFieldIterator<FProperty> It(Function); It; ++It)
				{
					FProperty* Prop = *It;
					if (Prop->HasAnyPropertyFlags(CPF_Parm) && JsonObject->HasField(Prop->GetName()))
					{
						const TSharedPtr<FJsonValue> JsonValue = JsonObject->GetField<EJson::None>(Prop->GetName());
						Prop->ImportText_Direct(*JsonValue->AsString(), Prop->ContainerPtrToValuePtr<void>(Parms), GI, 0);
					}
				}
			}
			
			GI->ProcessEvent(Function, Parms);
		}
	}
}
