using UnrealBuildTool;

public class JamGame : ModuleRules
{
	public JamGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "JamCore", "UMG", "Slate", "SlateCore", "Niagara", "NiagaraCore", "ProceduralMeshComponent", "JamDesigner" });
		PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}
