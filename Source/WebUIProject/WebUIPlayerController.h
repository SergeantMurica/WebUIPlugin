
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WebUIPlayerController.generated.h"

class UWebUIWidget;

UCLASS()
class WEBUIPROJECT_API AWebUIPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AWebUIPlayerController();

protected:
	virtual void BeginPlay() override;

	// The WebUI widget class to create
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WebUI")
	TSubclassOf<UWebUIWidget> WebUIWidgetClass;

	// The created widget instance
	UPROPERTY(BlueprintReadOnly, Category = "WebUI")
	TObjectPtr<UWebUIWidget> WebUIWidget;

private:
	void SetupWebUI();
};