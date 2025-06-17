using UnrealBuildTool;
using System.Collections.Generic;

public class WebUIProjectTarget : TargetRules
{
	public WebUIProjectTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		ExtraModuleNames.Add("WebUIProject");
	}
}
