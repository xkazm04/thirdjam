using UnrealBuildTool;

public class JamDesigner : ModuleRules
{
	public JamDesigner(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "JamCore" });
		PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}
