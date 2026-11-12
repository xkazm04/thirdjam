using UnrealBuildTool;

public class JamDesigner : ModuleRules
{
	public JamDesigner(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "JamCore", "HTTP", "Json", "JsonUtilities" });
		PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}
