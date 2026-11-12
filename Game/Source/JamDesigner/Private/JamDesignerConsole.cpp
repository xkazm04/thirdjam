// QA console commands (brief A7 item 5): jam.playcut <name> (jam.cut collides with A1 cvar jam.Cut), jam.say "<sentence>", jam.call <key> <value>, jam.key <key> (dev only).
#include "JamDesignerSubsystem.h"
#include "JamCartridge.h"
#include "HAL/IConsoleManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Console.h"

namespace JamDesignerConsole
{
	static UJamDesignerSubsystem* Get(UWorld* World)
	{
		UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
		UJamDesignerSubsystem* S = GI ? GI->GetSubsystem<UJamDesignerSubsystem>() : nullptr;
		if (!S) UE_LOG(LogJamDesigner, Warning, TEXT("JAMDESIGNER no game instance here (run in PIE or the game)"));
		return S;
	}

	static FString Joined(const TArray<FString>& Args)
	{
		FString S = FString::Join(Args, TEXT(" ")).TrimStartAndEnd();
		if (S.Len() >= 2 && S.StartsWith(TEXT("\"")) && S.EndsWith(TEXT("\""))) S = S.Mid(1, S.Len() - 2);
		return S;
	}

	static FAutoConsoleCommandWithWorldAndArgs CutCmd(TEXT("jam.playcut"), TEXT("jam.playcut <name>: load a shipped or saved cut and rebuild the match; no name lists them (jam.cut is taken: lane A1 cvar jam.Cut, console names ignore case)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			UJamDesignerSubsystem* S = Get(World);
			if (!S) return;
			const FString Name = Joined(Args);
			if (Name.IsEmpty())
			{
				for (const FJamCutInfo& I : S->ListCuts()) UE_LOG(LogJamDesigner, Display, TEXT("JAMDESIGNER cut %s%s %s"), *I.Name, I.bSaved ? TEXT(" (saved)") : TEXT(""), *I.Sentence);
				return;
			}
			FString Err;
			if (!S->LoadCut(Name, Err)) UE_LOG(LogJamDesigner, Warning, TEXT("JAMDESIGNER jam.playcut %s: %s"), *Name, *Err);
		}));

	static FAutoConsoleCommandWithWorldAndArgs SayCmd(TEXT("jam.say"), TEXT("jam.say \"<sentence>\": ask the Designer for a cut, save it as My Director's Cut and play it"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			UJamDesignerSubsystem* S = Get(World);
			if (!S) return;
			TWeakObjectPtr<UJamDesignerSubsystem> Weak(S);
			S->RequestCut(Joined(Args), FString(), [Weak](const FJamDesignerResult& R)
			{
				UE_LOG(LogJamDesigner, Display, TEXT("JAMDESIGNER jam.say -> %s"), *R.Message);
				for (const FString& E : R.Errors) UE_LOG(LogJamDesigner, Display, TEXT("  %s"), *E);
				FString Err;
				if (R.HasCartridge() && Weak.IsValid() && !Weak->ApplyResult(R, Err)) UE_LOG(LogJamDesigner, Warning, TEXT("JAMDESIGNER apply failed: %s"), *Err);
			});
		}));

	static FAutoConsoleCommandWithWorldAndArgs CallCmd(TEXT("jam.call"), TEXT("jam.call <tunable_key> <value>: the Director's Call (one per period)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			UJamDesignerSubsystem* S = Get(World);
			if (!S) return;
			if (Args.Num() != 2 || !FCString::IsNumeric(*Args[1])) { UE_LOG(LogJamDesigner, Warning, TEXT("JAMDESIGNER usage: jam.call <tunable_key> <value>")); return; }
			const FJamDirectorsCall C = S->DirectorsCall(FName(*Args[0]), FCString::Atod(*Args[1]));
			UE_LOG(LogJamDesigner, Display, TEXT("JAMDESIGNER jam.call %s"), C.bApplied ? *C.Announcement : *(TEXT("refused: ") + C.Reason));
		}));

#if !UE_BUILD_SHIPPING
	// Dev only. The key never reaches a log; the command line is scrubbed from the game console's saved history.
	// The editor's Output Log keeps its own history (Saved/Config/ConsoleHistory.ini) with no API to remove a line: prefer
	// ANTHROPIC_API_KEY in the environment when working in the editor.
	static FAutoConsoleCommandWithWorldAndArgs KeyCmd(TEXT("jam.key"), TEXT("jam.key <key> | clear: (dev) store the Designer API key in Saved/Config"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			UJamDesignerSubsystem* S = Get(World);
			if (S) S->SetApiKey(Args.Num() == 1 && Args[0] != TEXT("clear") ? Args[0] : FString());
			if (GEngine && GEngine->GameViewport && GEngine->GameViewport->ViewportConsole)
			{
				UConsole* Con = GEngine->GameViewport->ViewportConsole;
				Con->HistoryBuffer.RemoveAll([](const FString& L) { return L.TrimStart().StartsWith(TEXT("jam.key"), ESearchCase::IgnoreCase); });
				Con->SaveConfig();
			}
		}));
#endif
}
