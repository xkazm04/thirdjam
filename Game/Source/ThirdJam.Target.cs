using UnrealBuildTool;
using System.Collections.Generic;

public class ThirdJamTarget : TargetRules
{
	public ThirdJamTarget(TargetInfo Target) : base(Target)
	{
		bUseLoggingInShipping = true;   // jam: the Shipping package writes a log so a packaged run can be measured (lane A48, R-0032 point 3)
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V8;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_9;
		ExtraModuleNames.AddRange(new string[] { "JamCore", "JamGame", "JamDesigner", "JamNet" });
	}
}
