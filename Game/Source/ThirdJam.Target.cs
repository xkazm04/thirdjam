using UnrealBuildTool;
using System.Collections.Generic;

public class ThirdJamTarget : TargetRules
{
	public ThirdJamTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V8;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_9;
		ExtraModuleNames.AddRange(new string[] { "JamCore", "JamGame", "JamDesigner", "JamNet" });
	}
}
