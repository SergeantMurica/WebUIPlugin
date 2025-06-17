#include "WebUIWidget.h"
#include "SWebBrowser.h"
#include "Interfaces/IPluginManager.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Blueprint/UserWidget.h"

UWebUIWidget::UWebUIWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UWebUIWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UE_LOG(LogTemp, Warning, TEXT("WebUIWidget: NativeConstruct called"));
}

// This function just stores the object and name for later.
void UWebUIWidget::BindUObjectToJavaScript(const FString& Name, UObject* Object)
{
	UE_LOG(LogTemp, Warning, TEXT("WebUIWidget: BindUObjectToJavaScript called - Name: %s, Object: %s"), 
		*Name, Object ? *Object->GetName() : TEXT("NULL"));
		
	NameToBind = Name;
	ObjectToBind = Object;
	
	// If the web browser is already created and loaded, bind immediately
	if (WebBrowserWidget.IsValid() && ObjectToBind)
	{
		WebBrowserWidget->BindUObject(NameToBind, ObjectToBind, true);
		UE_LOG(LogTemp, Warning, TEXT("WebUIWidget: Immediate bind - Object '%s' bound to JavaScript window."), *NameToBind);
	}
}

void UWebUIWidget::ExecuteJavaScript(const FString& ScriptText)
{
	if (WebBrowserWidget.IsValid())
	{
		WebBrowserWidget->ExecuteJavascript(ScriptText);
		UE_LOG(LogTemp, Log, TEXT("WebUIWidget: Executing JavaScript: %s"), *ScriptText);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("WebUIWidget: Cannot execute JavaScript - WebBrowserWidget is not valid"));
	}
}

TSharedRef<SWidget> UWebUIWidget::RebuildWidget()
{
	UE_LOG(LogTemp, Warning, TEXT("WebUIWidget: RebuildWidget called"));
	
	if (IsDesignTime())
	{
		return SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(FText::FromString(TEXT("WebUI Widget")))
			];
	}
	else
	{
		FString URLToLoad = bIsDevMode ? InitialURL : GetProductionURL();
		UE_LOG(LogTemp, Warning, TEXT("WebUIWidget: Loading URL: %s"), *URLToLoad);
		
		WebBrowserWidget = SNew(SWebBrowser)
			.InitialURL(URLToLoad)
			.SupportsTransparency(true)
			.OnLoadCompleted(FSimpleDelegate::CreateUObject(this, &UWebUIWidget::HandleOnLoadCompleted))
			.OnLoadStarted(FSimpleDelegate::CreateLambda([this]() {
				UE_LOG(LogTemp, Warning, TEXT("WebUIWidget: Page load started"));
			}))
			.OnLoadError(FSimpleDelegate::CreateLambda([this]() {
				UE_LOG(LogTemp, Error, TEXT("WebUIWidget: Page load error"));
			}));
			
		return WebBrowserWidget.ToSharedRef();
	}
}

// This is the new, reliable binding logic.
void UWebUIWidget::HandleOnLoadCompleted()
{
	UE_LOG(LogTemp, Warning, TEXT("WebUIWidget: Page load completed"));
	
	if (ObjectToBind && WebBrowserWidget.IsValid())
	{
		// Bind the stored object directly to the browser window.
		// The 'true' exposes all of its UFUNCTIONs.
		WebBrowserWidget->BindUObject(NameToBind, ObjectToBind, true);
		UE_LOG(LogTemp, Warning, TEXT("WebUIWidget: Object '%s' successfully bound to JavaScript window."), *NameToBind);
		
		// Execute a test script to verify the binding worked
		FString TestScript = FString::Printf(TEXT("console.log('UE Object bound as %s:', window.%s); if(window.%s) { console.log('Functions available:', Object.getOwnPropertyNames(window.%s)); }"), 
			*NameToBind, *NameToBind, *NameToBind, *NameToBind);
		WebBrowserWidget->ExecuteJavascript(TestScript);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("WebUIWidget: Failed to bind object - ObjectToBind: %s, WebBrowserWidget: %s"), 
			ObjectToBind ? TEXT("Valid") : TEXT("NULL"), 
			WebBrowserWidget.IsValid() ? TEXT("Valid") : TEXT("Invalid"));
	}
}

FString UWebUIWidget::GetProductionURL() const
{
	FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("WebUIFramework"))->GetBaseDir();
	FString HtmlPath = FPaths::Combine(PluginDir, TEXT("DevServer/src/templates/index.html"));
	FPaths::MakeStandardFilename(HtmlPath);
	return TEXT("file:///") + HtmlPath;
}