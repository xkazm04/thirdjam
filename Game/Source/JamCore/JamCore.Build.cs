using UnrealBuildTool;

public class JamCore : ModuleRules
{
	public JamCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		// Lane A2's bot brain headers (UJamBrain, JamAI::*) are reached by the game mode in JamGame.
		PublicIncludePaths.Add(System.IO.Path.Combine(ModuleDirectory, "AI"));
		// Lane A14 (C-0005): the effect system and the pickup actor live in Effects/ (UJamEffectSystem, AJamPickupActor).
		PublicIncludePaths.Add(System.IO.Path.Combine(ModuleDirectory, "Effects"));
		// Lane A15 (C-0005): rule hooks, variants and zones (JamRulesConfig.h is reached by JamCartridge.h; UJamRuleSystem by the game mode).
		PublicIncludePaths.Add(System.IO.Path.Combine(ModuleDirectory, "Rules"));
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
		// Lane A16 (R-0016): the puck actor spawns device B's NS_puck_trail.
		PrivateDependencyModuleNames.AddRange(new string[] { "Niagara" });
	}
}
