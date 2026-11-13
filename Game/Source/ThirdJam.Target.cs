using UnrealBuildTool;
using System.Collections.Generic;

public class ThirdJamTarget : TargetRules
{
	public ThirdJamTarget(TargetInfo Target) : base(Target)
	{
		// jam: bUseLoggingInShipping stays off: the installed engine shares its build environment with the Shipping target, so the flag fails UBT (H19, 00:06). Packaged proofs use the Development game build (A49, A48).
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V8;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_9;
		ExtraModuleNames.AddRange(new string[] { "JamCore", "JamGame", "JamDesigner", "JamNet" });
	}
}
