#include "WebUIPlayerController.h"
#include "WebUIWidget.h"
#include "WebUIGameInstance.h"
#include "Components/Widget.h"
#include "Blueprint/UserWidget.h"

AWebUIPlayerController::AWebUIPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void AWebUIPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Warning, TEXT("WebUIPlayerController: BeginPlay called"));
	
	// Small delay to ensure everything is initialized
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AWebUIPlayerController::SetupWebUI, 0.1f, false);
}

void AWebUIPlayerController::SetupWebUI()
{
	UE_LOG(LogTemp, Warning, TEXT("WebUIPlayerController: Setting up WebUI"));
	
	// Get the Game Instance
	UWebUIGameInstance* GameInstance = Cast<UWebUIGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("WebUIPlayerController: Game Instance is not UWebUIGameInstance!"));
		return;
	}

	// Create the WebUI widget
	if (WebUIWidgetClass)
	{
		WebUIWidget = CreateWidget<UWebUIWidget>(this, WebUIWidgetClass);
		if (WebUIWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("WebUIPlayerController: WebUI widget created"));
			
			// Add to viewport
			WebUIWidget->AddToViewport();
			
			// Register with the Game Instance - THIS IS THE KEY STEP
			GameInstance->RegisterWebUIWidget(WebUIWidget);
			
			UE_LOG(LogTemp, Warning, TEXT("WebUIPlayerController: WebUI widget registered with Game Instance"));
			
			// Set input mode to UI only
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(WebUIWidget->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("WebUIPlayerController: Failed to create WebUI widget"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("WebUIPlayerController: WebUIWidgetClass is not set!"));
	}
}