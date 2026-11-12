using UnrealBuildTool;

public class JamCore : ModuleRules
{
	public JamCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		// Lane A2's bot brain headers (UJamBrain, JamAI::*) are reached by the game mode in JamGame.
		PublicIncludePaths.Add(System.IO.Path.Combine(ModuleDirectory, "AI"));
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
		PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}
