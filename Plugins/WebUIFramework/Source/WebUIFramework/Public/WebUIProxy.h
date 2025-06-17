#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WebUIProxy.generated.h"

class UGameInstance;

UCLASS()
class WEBUIFRAMEWORK_API UWebUIProxy : public UObject
{
	GENERATED_BODY()

public:
	// This function will be given our real Game Instance to talk to.
	void SetGameInstance(UGameInstance* InGameInstance);

	// This is the SINGLE function that all JavaScript calls will go through.
	UFUNCTION()
	void CallGameInstanceFunction(const FString& FunctionName, const FString& ParamsJson);

private:
	// A pointer to the real Game Instance that holds all our game logic.
	UPROPERTY()
	TWeakObjectPtr<UGameInstance> GameInstancePtr;
};
