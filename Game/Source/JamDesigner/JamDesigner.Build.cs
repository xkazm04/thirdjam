using UnrealBuildTool;

public class JamDesigner : ModuleRules
{
	public JamDesigner(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bUseUnity = false; // R-0022: every file compiles alone, as in a clean checkout (anonymous-namespace helpers collided in unity blobs)
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "JamCore", "HTTP", "Json", "JsonUtilities" });
		PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}
