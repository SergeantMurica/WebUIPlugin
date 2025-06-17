using UnrealBuildTool;

public class WebUIFramework : ModuleRules
{
	public WebUIFramework(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"UMG",
				"WebBrowserWidget",
				"WebBrowser"  // This module contains SWebBrowser implementation
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"Json",
				"Sockets",      
				"Networking",
				"Projects" 
			}
		);
	}
}