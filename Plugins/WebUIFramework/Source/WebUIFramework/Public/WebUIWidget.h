
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WebUIWidget.generated.h"

class SWebBrowser;

UCLASS()
class WEBUIFRAMEWORK_API UWebUIWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UWebUIWidget(const FObjectInitializer& ObjectInitializer);

	// This is the main function to connect an object to the UI
	UFUNCTION(BlueprintCallable, Category = "WebUI")
	void BindUObjectToJavaScript(const FString& Name, UObject* Object);

	// Execute JavaScript function
	UFUNCTION(BlueprintCallable, Category = "WebUI")
	void ExecuteJavaScript(const FString& ScriptText);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebUI")
	bool bIsDevMode = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebUI", meta = (EditCondition = "bIsDevMode"))
	FString InitialURL = TEXT("http://localhost:3000");

protected:
	virtual void NativeConstruct() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void HandleOnLoadCompleted();
	FString GetProductionURL() const;

	TSharedPtr<SWebBrowser> WebBrowserWidget;

	// We store the object and name here until the page is loaded
	UPROPERTY(Transient)
	TObjectPtr<UObject> ObjectToBind;
	FString NameToBind;
};