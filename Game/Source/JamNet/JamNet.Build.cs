using UnrealBuildTool;

// Lane A9: multiplayer by invite. Deterministic lockstep on UJamSim over a UDP socket pair (docs/experiments/MULTIPLAYER.md).
// Nothing else in the project depends on this module; it depends on JamCore (the sim) and JamGame (the game mode it subclasses).
public class JamNet : ModuleRules
{
	public JamNet(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "JamCore", "JamGame", "Sockets", "Networking", "UMG", "Slate", "SlateCore", "InputCore" });
		PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}
