using UnrealBuildTool;
using System.Collections.Generic;

public class ThirdJamEditorTarget : TargetRules
{
	public ThirdJamEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V8;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_9;
		ExtraModuleNames.AddRange(new string[] { "JamCore", "JamGame", "JamDesigner" });
		ExtraModuleNames.Add("JamEditor");
	}
}
