#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FWebUIFrameworkModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	// Blueprint-callable function to get the dynamic URL
	UFUNCTION(BlueprintPure, Category = "WebUI")
	static FString GetDevServerURL();

	// C++ getter for the module instance
	static FWebUIFrameworkModule& Get();

private:
	// Finds a free TCP port on the local machine
	int32 FindFreePort();

	// Handle to the running Node.js process
	FProcHandle DevServerProcessHandle;

	// The port the server is running on
	int32 ServerPort = 0;
};