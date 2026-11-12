using UnrealBuildTool;

public class JamEditor : ModuleRules
{
	public JamEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bUseUnity = false; // R-0022: every file compiles alone, as in a clean checkout (anonymous-namespace helpers collided in unity blobs)
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
		PrivateDependencyModuleNames.AddRange(new string[] { "UnrealEd", "BlueprintGraph", "AnimGraph", "AnimGraphRuntime", "Kismet" });
	}
}
