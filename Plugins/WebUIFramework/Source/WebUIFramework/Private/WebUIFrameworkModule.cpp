#include "WebUIFrameworkModule.h"
#include "Interfaces/IPluginManager.h"
#include "HAL/PlatformProcess.h"
#include "Sockets.h"
#include "SocketSubsystem.h"

#define LOCTEXT_NAMESPACE "FWebUIFrameworkModule"

void FWebUIFrameworkModule::StartupModule()
{
#if WITH_EDITOR
	// Find a free port for the server to use
	ServerPort = FindFreePort();
	if (ServerPort == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("WebUIFramework: Could not find a free port. Dev server will not start."));
		return;
	}

	// Get the path to the DevServer directory
	FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("WebUIFramework"))->GetBaseDir();
	FString DevServerPath = FPaths::Combine(PluginDir, TEXT("DevServer"));

	// Construct the command to run: node server.js [port]
	FString NodePath = TEXT("node"); // Assumes node is in the system's PATH
	FString ServerScriptPath = FPaths::Combine(DevServerPath, TEXT("server.js"));
	FString Params = FString::Printf(TEXT("\"%s\" %d"), *ServerScriptPath, ServerPort);

	// Launch the process
	DevServerProcessHandle = FPlatformProcess::CreateProc(
		*NodePath,
		*Params,
		false, // bLaunchDetached
		true,  // bLaunchHidden
		true,  // bLaunchReallyHidden
		nullptr, 0,
		*DevServerPath, // WorkingDirectory
		nullptr
	);

	if (DevServerProcessHandle.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("WebUIFramework: Dev server started on port %d."), ServerPort);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("WebUIFramework: Failed to start dev server."));
	}
#endif
}

void FWebUIFrameworkModule::ShutdownModule()
{
#if WITH_EDITOR
	// If the process handle is valid, it means our server is running
	if (DevServerProcessHandle.IsValid())
	{
		// Terminate the Node.js process
		FPlatformProcess::TerminateProc(DevServerProcessHandle);
		FPlatformProcess::CloseProc(DevServerProcessHandle);
		UE_LOG(LogTemp, Log, TEXT("WebUIFramework: Dev server stopped."));
	}
#endif
}

FString FWebUIFrameworkModule::GetDevServerURL()
{
	FWebUIFrameworkModule& Self = FWebUIFrameworkModule::Get();
	if (Self.ServerPort != 0)
	{
		return FString::Printf(TEXT("http://localhost:%d"), Self.ServerPort);
	}
	// Fallback URL if server isn't running
	return TEXT("about:blank");
}

FWebUIFrameworkModule& FWebUIFrameworkModule::Get()
{
	return FModuleManager::LoadModuleChecked<FWebUIFrameworkModule>("WebUIFramework");
}

int32 FWebUIFrameworkModule::FindFreePort()
{
	FSocket* Socket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("PortCheck"), false);
	if (!Socket)
	{
		return 0;
	}

	TSharedRef<FInternetAddr> Addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	Addr->SetIp(0x7F000001);



	// Start checking from port 3000
	for (int32 Port = 3000; Port < 4000; ++Port)
	{
		Addr->SetPort(Port);
		if (Socket->Bind(*Addr))
		{
			// This port is free
			Socket->Close();
			ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
			return Port;
		}
	}

	// No free port found in the range
	Socket->Close();
	ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
	return 0;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FWebUIFrameworkModule, WebUIFramework)