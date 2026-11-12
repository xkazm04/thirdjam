using UnrealBuildTool;

public class JamGame : ModuleRules
{
	public JamGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bUseUnity = false; // R-0022: every file compiles alone, as in a clean checkout (anonymous-namespace helpers collided in unity blobs)
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "JamCore", "UMG", "Slate", "SlateCore", "Niagara", "NiagaraCore", "ProceduralMeshComponent", "JamDesigner" });
		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Lane A11: NVIDIA DLSS (Game/Plugins/DLSS, gitignored, fetched by tools/dlss/fetch-dlss.ps1). Absent plugin = DLSS compiled out.
		bool bHasDlss = Target.Platform == UnrealTargetPlatform.Win64
			&& System.IO.File.Exists(System.IO.Path.Combine(PluginDirectoryOrProject(), "Plugins", "DLSS", "DLSS.uplugin"));
		if (bHasDlss) { PrivateDependencyModuleNames.Add("DLSSBlueprint"); }
		PublicDefinitions.Add("JAM_WITH_DLSS=" + (bHasDlss ? "1" : "0"));
	}

	private string PluginDirectoryOrProject()
	{
		return Target.ProjectFile != null ? Target.ProjectFile.Directory.FullName : System.IO.Path.Combine(ModuleDirectory, "..", "..");
	}
}
