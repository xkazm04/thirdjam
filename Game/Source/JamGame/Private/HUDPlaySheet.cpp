// Lane A213, wave 3 (the play sheet for the package he plays at 10:00): the controls table of docs/experiments/PLAY-ME-DRAFT.md is generated from the
// LIVE bindings, never typed. `jam.ui.playsheet <file>` writes the CONTROLS screen's rows (UJamInputMap::ControlRowsFromBindings: the same rows the
// menu shows, with tonight's words under jam.hud.Words) as a Markdown table; tools/qa/screen/a213/play-sheet.mjs pastes it between the draft's markers.
// Run headless: UnrealEditor-Cmd <uproject> -ExecCmds="jam.ui.playsheet <abs path>, Quit" (a comma: the arguments run to the end of the command) -nullrhi -Multiprocess -unattended.
#include "CoreMinimal.h"
#include "HAL/IConsoleManager.h"
#include "Misc/FileHelper.h"
#include "InputMapping.h"
#include "MenuFlow.h"

namespace JamPlaySheet
{
	FString Cell(const FString& S) { return S.Replace(TEXT("|"), TEXT("/")); }

	FString ControlsTable()
	{
		FString Out = TEXT("| Control | Mouse and keys | Pad | What it does |\n|---|---|---|---|\n");
		for (const FJamControlRow& R : UJamInputMap::ControlRowsFromBindings())
		{
			Out += FString::Printf(TEXT("| %s | %s | %s | %s |\n"), *Cell(R.Verb), *Cell(R.Keys), *Cell(R.Pad), *Cell(R.Note));
		}
		return Out;
	}
}

static FAutoConsoleCommand GJamPlaySheet(TEXT("jam.ui.playsheet"),
	TEXT("Lane A213: writes the controls table (the live bindings, the CONTROLS screen's rows) as Markdown to the file named: jam.ui.playsheet <path>."),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
	{
		const FString Path = Args.Num() ? FString::Join(Args, TEXT(" ")) : FString(TEXT("play-sheet-controls.md"));
		const FString Table = JamPlaySheet::ControlsTable();
		const bool bOk = FFileHelper::SaveStringToFile(Table, *Path, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
		UE_LOG(LogTemp, Log, TEXT("JAMPLAYSHEET wrote=%d rows=%d path=%s"), bOk ? 1 : 0, UJamInputMap::ControlRowsFromBindings().Num(), *Path);
	}));
