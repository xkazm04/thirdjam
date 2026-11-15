#include "MenuFlow.h"
#include "InputMapping.h"
#include "JamCartridge.h"
#include "JamJson.h"
#include "Progression.h"
#include "SettingsWidget.h"
#include "DifficultyResolve.h"
#include "DifficultyProfile.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/IConsoleManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "JamDesignerSubsystem.h"

const TCHAR* UJamMenuFlow::MenuMap = TEXT("/Game/Maps/Menu");
const TCHAR* UJamMenuFlow::RinkMap = TEXT("/Game/Maps/Rink");
const TCHAR* UJamMenuFlow::FirstCut = TEXT("hockey.regular");

DEFINE_LOG_CATEGORY_STATIC(LogJamMenu, Log, All);

void UJamMenuFlow::StartMatch(UObject* WorldContext, const FString& CutNameOrPath)
{
	StartMatchAtLevel(WorldContext, CutNameOrPath, FString());
}

// Lane A213 (A211's EYE LIST 3 item 2, the Director's ruling of 01:52: "Glow Ice plays PRO with ROOKIE lit on HOME and nothing says so"): the level
// lit on HOME is the level he plays, in every mode. A Director's mode ("modes/<name>") now travels with HOME's level as PLAY NOW does.
// jam.menu.ModeLevel 0 = a mode plays its cartridge's own level (the menu of before).
static TAutoConsoleVariable<int32> CVarJamMenuModeLevel(TEXT("jam.menu.ModeLevel"), 1,
	TEXT("Lane A213: 1 (default) a Director's mode plays the level lit on HOME; 0 = its cartridge's own level (Glow Ice played pro under ROOKIE)."), ECVF_Default);
bool UJamMenuFlow::ModeLevelFromHome() { return CVarJamMenuModeLevel.GetValueOnAnyThread() != 0; }

FString UJamMenuFlow::PlayerLevelFor(const FString& CutName, bool bSavedOrTake, const FJamSettings& Settings)
{
	if (bSavedOrTake) return FString();
	if (ModeLevelFromHome() && CutName.StartsWith(TEXT("modes/"))) return FirstMatchLevel(Settings);
	// A shipped cut is a plain name; a mode is "modes/<name>", a take or a preview is a path.
	if (CutName.Contains(TEXT("/")) || CutName.Contains(TEXT("\\")) || CutName.Contains(TEXT(":"))) return FString();
	return FirstMatchLevel(Settings);
}

void UJamMenuFlow::StartMatchAtLevel(UObject* WorldContext, const FString& CutNameOrPath, const FString& Level)
{
	if (IConsoleVariable* V = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.Cut")))
	{
		V->Set(*CutNameOrPath, ECVF_SetByCode);
	}
	else
	{
		UE_LOG(LogJamMenu, Warning, TEXT("JAMMENU the jam.Cut console variable does not exist; the rink will load its default cut"));
	}
	UE_LOG(LogJamMenu, Log, TEXT("JAMMENU start match cut=%s level=%s"), *CutNameOrPath, Level.IsEmpty() ? TEXT("cartridge") : *Level);
	// Plain names also travel as a URL option (the game mode reads either); paths stay in the console variable only.
	const bool bPlainName = !CutNameOrPath.Contains(TEXT("/")) && !CutNameOrPath.Contains(TEXT("\\")) && !CutNameOrPath.Contains(TEXT(":")) && !CutNameOrPath.Contains(TEXT(" "));
	// Lane A144: the level is an option of this open only (never a console variable that outlives it): the next match that names none plays its
	// cartridge's own.
	// Director, 14 Nov (lane A155, the real window): the rink this call opens loads the cut itself, with the level asked for here. A Designer
	// LoadCut just before this call had scheduled the same load for the next frame: it arrived in the new map and built the match a second time
	// at the cartridge's own level (PLAY NOW with ROOKIE lit played PRO). The scheduled apply is taken back.
	if (const UWorld* W = GEngine ? GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull) : nullptr)
	{
		if (UGameInstance* GI = W->GetGameInstance()) if (UJamDesignerSubsystem* D = GI->GetSubsystem<UJamDesignerSubsystem>()) D->CancelScheduledApply();
	}
	TArray<FString> Options;
	if (bPlainName) Options.Add(FString::Printf(TEXT("cut=%s"), *CutNameOrPath));
	if (!Level.IsEmpty()) Options.Add(FString::Printf(TEXT("level=%s"), *Level));
	UGameplayStatics::OpenLevel(WorldContext, FName(RinkMap), true, FString::Join(Options, TEXT("?")));
}

void UJamMenuFlow::ReturnToMenu(UObject* WorldContext)
{
	UGameplayStatics::SetGamePaused(WorldContext, false);
	UE_LOG(LogJamMenu, Log, TEXT("JAMMENU return to menu"));
	// Lane A17: back at the menu the season run ends (its results stay); a cut chosen from the list is a free play.
	if (UJamProgression* P = UJamProgression::Get(WorldContext)) P->EndSeasonRun();
	UGameplayStatics::OpenLevel(WorldContext, FName(MenuMap), true);
}

FString UJamMenuFlow::DisplayNameFor(const FJamCutInfo& Cut)
{
	if (Cut.bSaved) return Cut.Name;
	static const TMap<FString, FString> Known = {
		{ TEXT("hockey.regular"), TEXT("Regular Cut") },
		{ TEXT("hockey.raw"), TEXT("Raw Cut") },
		{ TEXT("hockey.boost"), TEXT("Boost Cut") },
		{ TEXT("hockey.arena.large"), TEXT("Large Rink Cut") },
		{ TEXT("hockey.arena.wide"), TEXT("Wide Cut") },
	};
	if (const FString* K = Known.Find(Cut.Name)) return *K;
	FString Rest = Cut.Name;
	Rest.RemoveFromStart(TEXT("hockey."));
	TArray<FString> Parts;
	Rest.ParseIntoArray(Parts, TEXT("."));
	FString Out;
	for (FString& P : Parts)
	{
		if (P.IsEmpty()) continue;
		P[0] = FChar::ToUpper(P[0]);
		Out += (Out.IsEmpty() ? TEXT("") : TEXT(" ")) + P;
	}
	return Out.IsEmpty() ? Cut.Name : Out + TEXT(" Cut");
}

FString UJamMenuFlow::DescribeCut(const UJamCartridge& C, bool bWithName)
{
	const FJamGameConfig& G = C.Game;
	TArray<FString> Bits;
	// The envelope has no description key (R-0010): every cut prints its own rink size (lane A47, B-to-A-0030 fault 10: Raw and Boost are 120 x 60,
	// the old fixed strings said 90 x 45); the shipped cuts add a word, keyed by cartridge id.
	static const TMap<FString, FString> ById = {
		{ TEXT("hockey-regular"), TEXT("fast") },
		{ TEXT("hockey-arena-large"), TEXT("open ice") },
		{ TEXT("hockey-raw"), TEXT("no rules") },
		{ TEXT("hockey-boost"), TEXT("pickups on the ice") },
	};
	const FString* Lead = ById.Find(C.Id);
	const FString Size = FString::Printf(TEXT("%g x %g m"), G.Rink.LengthM, G.Rink.WidthM);
	Bits.Add(Lead ? FString::Printf(TEXT("%s, %s"), *Size, **Lead) : Size);
	if (G.Rules.Mode == FName(TEXT("timer")))
	{
		const int32 Secs = G.Rules.TimerFrames / 60;
		Bits.Add(FString::Printf(TEXT("%d:%02d on the clock"), Secs / 60, Secs % 60));
	}
	else
	{
		Bits.Add(FString::Printf(TEXT("first to %d"), G.Rules.ScoreToWin));
	}
	if (G.Skater.bCheckEnabled) Bits.Add(TEXT("body checks"));
	if (G.Pickups.Num() == 1) Bits.Add(TEXT("1 pickup kind"));
	else if (G.Pickups.Num() > 1) Bits.Add(FString::Printf(TEXT("%d pickup kinds"), G.Pickups.Num()));
	if (G.Rink.Obstacles.Num()) Bits.Add(FString::Printf(TEXT("%d obstacles"), G.Rink.Obstacles.Num()));
	if (!Lead && G.Skater.Speed >= 10) Bits.Add(TEXT("fast skaters"));
	if (!bWithName) return FString::Join(Bits, TEXT(", "));
	const FString Name = C.ThemeName.IsEmpty() ? C.Title : C.ThemeName;
	return FString::Printf(TEXT("%s: %s"), *Name, *FString::Join(Bits, TEXT(", ")));
}

FString UJamMenuFlow::KeyTunables(const UJamCartridge& C)
{
	const FJamGameConfig& G = C.Game;
	return FString::Printf(TEXT("speed %s m/s   shot %s m/s   ice %s   bounce %s   bots %s"),
		*FJamJson::NumberToString(G.Skater.Speed), *FJamJson::NumberToString(G.Shot.Speed), *FJamJson::NumberToString(G.Rink.IceFriction),
		*FJamJson::NumberToString(G.Rink.BoardRestitution), *FJamJson::NumberToString(G.Ai.RedSkill));
}

TArray<FJamMenuCutRow> UJamMenuFlow::BuildCutRows(const TArray<FJamCutInfo>& Cuts)
{
	TArray<FJamMenuCutRow> Rows;
	for (const FJamCutInfo& I : Cuts)
	{
		FJamMenuCutRow R;
		R.Name = I.Name;
		R.Path = I.Path;
		R.bSaved = I.bSaved;
		R.Sentence = I.Sentence;
		R.Display = DisplayNameFor(I);
		TArray<FString> Errors;
		if (UJamCartridge* C = UJamCartridge::LoadFromFile(I.Path, Errors, GetTransientPackage()))
		{
			R.Description = DescribeCut(*C, I.bSaved);
			R.Tunables = KeyTunables(*C);
			if (I.bSaved && !I.Sentence.IsEmpty()) R.Description = FString::Printf(TEXT("\"%s\"  %s"), *I.Sentence, *R.Description);
		}
		else
		{
			R.Description = Errors.Num() ? FString::Printf(TEXT("cannot load: %s"), *Errors[0]) : TEXT("cannot load");
		}
		Rows.Add(R);
	}
	// Shipped order: Regular, Raw, Boost, Large Rink, Wide (GAME.md, R-0010), then the saved cuts.
	static const TArray<FString> Order = { TEXT("hockey.regular"), TEXT("hockey.raw"), TEXT("hockey.boost"), TEXT("hockey.arena.large"), TEXT("hockey.arena.wide") };
	Rows.StableSort([](const FJamMenuCutRow& A, const FJamMenuCutRow& B)
	{
		const int32 Ia = A.bSaved ? 100 : (Order.Contains(A.Name) ? Order.IndexOfByKey(A.Name) : 50);
		const int32 Ib = B.bSaved ? 100 : (Order.Contains(B.Name) ? Order.IndexOfByKey(B.Name) : 50);
		return Ia < Ib;
	});
	return Rows;
}

namespace
{
	struct FJamDiffLeaf { FString Path; TSharedPtr<FJamJson> Value; };

	void CollectLeaves(const TSharedPtr<FJamJson>& Node, const FString& Path, TArray<FJamDiffLeaf>& Out)
	{
		if (!Node) return;
		if (Node->IsObject())
		{
			for (const TPair<FString, TSharedPtr<FJamJson>>& P : Node->Object) CollectLeaves(P.Value, Path.IsEmpty() ? P.Key : Path + TEXT(".") + P.Key, Out);
		}
		else if (Node->IsArray())
		{
			// Arrays are compared as a whole (team lists, pickups): their length is the leaf.
			FJamDiffLeaf L;
			L.Path = Path + TEXT(" count");
			L.Value = MakeShared<FJamJson>();
			L.Value->Type = FJamJson::EType::Number;
			L.Value->Number = Node->Array.Num();
			Out.Add(L);
		}
		else
		{
			Out.Add({ Path, Node });
		}
	}

	FString LeafText(const TSharedPtr<FJamJson>& V)
	{
		if (!V) return TEXT("absent");
		if (V->IsNumber()) return FJamJson::NumberToString(V->Number);
		if (V->IsString()) return V->String;
		if (V->IsBool()) return V->Bool ? TEXT("on") : TEXT("off");
		return TEXT("?");
	}

	bool SameLeaf(const TSharedPtr<FJamJson>& A, const TSharedPtr<FJamJson>& B)
	{
		if (!A || !B) return !A && !B;
		if (A->Type != B->Type) return false;
		if (A->IsNumber()) return A->Number == B->Number;
		if (A->IsBool()) return A->Bool == B->Bool;
		if (A->IsString()) return A->String == B->String;
		return true;
	}
}

TArray<FString> UJamMenuFlow::BiggestChanges(const FString& BaseJson, const FString& NewJson, int32 N)
{
	TArray<FString> Out;
	FString Err;
	TSharedPtr<FJamJson> A = FJamJson::Parse(BaseJson, Err), B = FJamJson::Parse(NewJson, Err);
	if (!B || !B->IsObject()) return Out;
	TArray<FJamDiffLeaf> La, Lb;
	if (A && A->IsObject()) CollectLeaves(A->Get(TEXT("game")), FString(), La);
	CollectLeaves(B->Get(TEXT("game")), FString(), Lb);
	struct FChange { FString Text; double Weight; };
	TArray<FChange> Changes;
	for (const FJamDiffLeaf& Nb : Lb)
	{
		const FJamDiffLeaf* Na = La.FindByPredicate([&](const FJamDiffLeaf& L) { return L.Path == Nb.Path; });
		const TSharedPtr<FJamJson> Va = Na ? Na->Value : nullptr;
		if (SameLeaf(Va, Nb.Value)) continue;
		double W = 0.5;
		if (Va && Va->IsNumber() && Nb.Value->IsNumber())
		{
			const double O = Va->Number, Nn = Nb.Value->Number;
			W = FMath::Abs(O) > 1e-9 ? FMath::Abs(Nn - O) / FMath::Abs(O) : (Nn != 0 ? 1.0 : 0.0);
		}
		Changes.Add({ FString::Printf(TEXT("%s %s -> %s"), *LabelFor(Nb.Path), *LeafText(Va), *LeafText(Nb.Value)), W });
	}
	// Removed leaves.
	for (const FJamDiffLeaf& Na : La)
	{
		if (!Lb.ContainsByPredicate([&](const FJamDiffLeaf& L) { return L.Path == Na.Path; }))
		{
			Changes.Add({ FString::Printf(TEXT("%s %s -> absent"), *LabelFor(Na.Path), *LeafText(Na.Value)), 0.4 });
		}
	}
	// A renamed cut is worth a line too, after the numbers.
	const TSharedPtr<FJamJson> Ta = A && A->Get(TEXT("theme")) ? A->Get(TEXT("theme"))->Get(TEXT("name")) : nullptr;
	const TSharedPtr<FJamJson> Tb = B->Get(TEXT("theme")) ? B->Get(TEXT("theme"))->Get(TEXT("name")) : nullptr;
	if (Tb && Tb->IsString() && (!Ta || !Ta->IsString() || Ta->String != Tb->String))
	{
		Changes.Add({ FString::Printf(TEXT("named \"%s\""), *Tb->String), 0.01 });
	}
	Changes.StableSort([](const FChange& X, const FChange& Y) { return X.Weight > Y.Weight; });
	for (int32 i = 0; i < Changes.Num() && i < N; ++i) Out.Add(Changes[i].Text);
	return Out;
}

FString UJamMenuFlow::LabelFor(const FString& KeyOrPath)
{
	static const TMap<FString, FString> Labels = {
		{ TEXT("skater_speed"), TEXT("skater speed") }, { TEXT("skater.speed"), TEXT("skater speed") },
		{ TEXT("skater_turn_rate"), TEXT("turn rate") }, { TEXT("skater.turn_rate"), TEXT("turn rate") },
		{ TEXT("shot_speed"), TEXT("shot power") }, { TEXT("shot.speed"), TEXT("shot power") },
		{ TEXT("puck_max_speed"), TEXT("puck top speed") }, { TEXT("puck.max_speed"), TEXT("puck top speed") },
		{ TEXT("ice_friction"), TEXT("ice friction") }, { TEXT("rink.ice_friction"), TEXT("ice friction") },
		{ TEXT("board_restitution"), TEXT("board bounce") }, { TEXT("rink.board_restitution"), TEXT("board bounce") },
		{ TEXT("ai_speed_scale"), TEXT("opponents' speed") }, { TEXT("ai.speed_scale"), TEXT("opponents' speed") },
		{ TEXT("ai_red_skill"), TEXT("opponents' skill") }, { TEXT("ai.red_skill"), TEXT("opponents' skill") },
		{ TEXT("goal_pause_frames"), TEXT("goal pause") }, { TEXT("rules.goal_pause_frames"), TEXT("goal pause") },
		{ TEXT("pickup_rate"), TEXT("pickup rate") }, { TEXT("pickup_spawn_every_frames"), TEXT("pickup rate") },
		{ TEXT("rink.length_m"), TEXT("rink length") }, { TEXT("rink.width_m"), TEXT("rink width") },
		{ TEXT("skater.check_enabled"), TEXT("body checks") }, { TEXT("look.tier"), TEXT("look") },
		{ TEXT("rules.score_to_win"), TEXT("goals to win") }, { TEXT("rules.timer_frames"), TEXT("clock frames") },
		{ TEXT("pickups count"), TEXT("pickup kinds") }, { TEXT("rink.obstacles count"), TEXT("obstacles") },
	};
	if (const FString* L = Labels.Find(KeyOrPath)) return *L;
	FString S = KeyOrPath.Replace(TEXT("_"), TEXT(" ")).Replace(TEXT("."), TEXT(" "));
	return S;
}

FString UJamMenuFlow::WritePreviewCut(const FString& Json)
{
	const FString Dir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Preview"));
	IFileManager::Get().MakeDirectory(*Dir, true);
	const FString Path = FPaths::Combine(Dir, TEXT("directors-preview.cartridge.json"));
	if (!FFileHelper::SaveStringToFile(Json, *Path, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		UE_LOG(LogJamMenu, Warning, TEXT("JAMMENU cannot write the preview cut to %s"), *Path);
		return FString();
	}
	return Path;
}

FString UJamMenuFlow::BaseCartridgeJson(const UJamDesignerSubsystem* Designer)
{
	if (Designer && Designer->GetCurrentCartridge() && !Designer->GetCurrentCartridge()->RawJson.IsEmpty()) return Designer->GetCurrentCartridge()->RawJson;
	FString T;
	FFileHelper::LoadFileToString(T, *UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")));
	return T;
}

// ---------------------------------------------------------------- Director's choices (lane A67, R-0036)

const TArray<FJamModeCard>& UJamMenuFlow::ModeCards()
{
	// Lane A81: the words come from each cartridge's game.meta (pitch, facts) and its envelope title; this table keeps only the menu order
	// (A67's six, then A81's six: two rows of six). A mode file not named here is appended in file order, so a new mode is a dropped file.
	// Numbers behind the facts: docs/experiments/MODES.md, MODES-2.md (tools/playtest/modes.mjs).
	static const TCHAR* Order[] = {
		TEXT("pond"), TEXT("cage"), TEXT("sniper"), TEXT("marathon"), TEXT("power-play"), TEXT("shotgun"),
		TEXT("sudden-death"), TEXT("outnumbered"), TEXT("glass-house"), TEXT("arsenal"), TEXT("bruisers"), TEXT("fog-of-war"),
	};
	static TArray<FJamModeCard> Cards;
	static bool bBuilt = false;
	if (bBuilt) return Cards;
	bBuilt = true;
	TArray<FString> Names;
	for (const TCHAR* Slug : Order) Names.Add(FString::Printf(TEXT("modes/hockey.mode.%s"), Slug));
	TArray<FString> Files;
	IFileManager::Get().FindFiles(Files, *FPaths::Combine(UJamCartridge::GetCartridgeDir(), TEXT("modes"), TEXT("*.cartridge.json")), true, false);
	Files.Sort();
	for (const FString& F : Files) Names.AddUnique(TEXT("modes/") + F.LeftChop(FString(TEXT(".cartridge.json")).Len()));
	for (const FString& Name : Names)
	{
		const FString Path = UJamCartridge::ResolveCartridgePath(Name);
		if (!FPaths::FileExists(Path)) continue;
		TArray<FString> Errors;
		const UJamCartridge* C = UJamCartridge::LoadFromFile(Path, Errors);
		if (!C || C->MetaPitch.IsEmpty()) continue;   // a mode without game.meta has no card words
		FJamModeCard M;
		M.Name = Name;
		M.Id = C->Id;
		M.Display = C->Title;
		M.Pitch = C->MetaPitch;
		M.Facts = C->MetaFacts;
		if (C->Game.Teams.Num() > 0) M.HomeSkaters = C->Game.Teams[0].Skaters.Num();
		if (C->Game.Teams.Num() > 1) M.AwaySkaters = C->Game.Teams[1].Skaters.Num();
		Cards.Add(M);
	}
	return Cards;
}

TArray<FJamModeCard> UJamMenuFlow::AvailableModeCards()
{
	TArray<FJamModeCard> Out;
	for (const FJamModeCard& C : ModeCards())
	{
		if (FPaths::FileExists(UJamCartridge::ResolveCartridgePath(C.Name))) Out.Add(C);
	}
	return Out;
}

FString UJamMenuFlow::ModeDisplayForId(const FString& CartridgeId)
{
	for (const FJamModeCard& C : ModeCards())
	{
		if (C.Id == CartridgeId) return C.Display;
	}
	return FString();
}

// ---------------------------------------------------------------- the first screen and the controls (lane A144, track U2)

FString UJamMenuFlow::WhatItIsLine()
{
	return TEXT("Five-a-side hockey. WASD skates, the mouse is your stick.");
}

FString UJamMenuFlow::BrandLineFor(bool bFirstCutHasWhistle)
{
	// R-0054 point 5: the banner is "Hockey with the whistle taken out" where it is true; the law itself where the first cut still has a whistle.
	return bFirstCutHasWhistle ? TEXT("The speed you bring is the hit you get.") : TEXT("Hockey with the whistle taken out.");
}

FString UJamMenuFlow::BrandLine()
{
	// Read from the cartridge each time the first screen is built, so the line is right on every tree (a cut that cannot be read keeps the law).
	TArray<FString> Errors;
	const UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(FirstCut), Errors);
	return BrandLineFor(!C || C->Game.Rules.bPenalties);
}

// ---------------------------------------------------------------- the player's level (lane A144: evolution 2, then the level wire)

FString UJamMenuFlow::FirstMatchLevel(const FJamSettings& Settings)
{
	return FJamSettings::DifficultyString(Settings.EffectiveDifficulty());
}

FString UJamMenuFlow::FirstMatchLine(const FString& Level)
{
	EJamMenuDifficulty D = EJamMenuDifficulty::Pro;
	FJamSettings::ParseDifficulty(Level, D);
	return FString::Printf(TEXT("ONE MATCH, %s.  ENTER / A, OR CLICK."), *FJamSettings::DifficultyLabel(D));
}

FString UJamMenuFlow::LevelMeaning(const FString& Level)
{
	// Lane A176: who each level is for, in one line (the design review: a regular player cleans up rookie, a newcomer is buried at pro, and the first
	// PLAY NOW is rookie with nothing saying it is the newcomer's level). No number that the curve could make stale.
	if (Level == TEXT("rookie")) return TEXT("New to hockey games? Start here: rookies rarely hit.");
	if (Level == TEXT("all_star")) return TEXT("The wall. All-stars hunt the puck. Beat pro first.");
	return TEXT("Played one before? Pros hit as a habit.");
}

FString UJamMenuFlow::CutLevel(const UJamCartridge& C)
{
	JamAI::FJamDifficultyResolution R;
	JamAI::ResolveDifficulty(C, R);
	switch (R.Level)
	{
	case JamAI::EDifficulty::Rookie: return TEXT("rookie");
	case JamAI::EDifficulty::AllStar: return TEXT("all_star");
	default: return TEXT("pro");
	}
}

FString UJamMenuFlow::CutCardLine(const FJamMenuCutRow& Row)
{
	if (Row.bSaved && !Row.Sentence.IsEmpty()) return FString::Printf(TEXT("\"%s\""), *Row.Sentence);
	TArray<FString> Bits;
	Row.Description.ParseIntoArray(Bits, TEXT(", "));
	if (Bits.Num() > 4) Bits.SetNum(4);
	return FString::Join(Bits, TEXT(", "));
}

const TArray<FJamControlRow>& UJamMenuFlow::ControlRows()
{
	// Lane A183: from the live bindings (UJamInputMap::ControlRowsFromBindings); Jam.Menu.ControlsComplete holds them against the mapping context.
	static TArray<FJamControlRow> Rows;
	Rows = UJamInputMap::ControlRowsFromBindings();
	return Rows;
}

// ---------------------------------------------------------------- lane A195 (U2 evolution 7): the last match, a rink's name

namespace
{
	struct FJamLastMatchNote { bool bSet = false; int32 Mine = 0, Theirs = 0; FString Level, Rink; };
	FJamLastMatchNote GJamLastMatch;   // game thread only; the process's last match (Progress.json keeps takes, not scores)
}

void UJamMenuFlow::NoteLastMatch(int32 Mine, int32 Theirs, const FString& LevelLabel, const FString& RinkNameIn)
{
	GJamLastMatch.bSet = true;
	GJamLastMatch.Mine = Mine;
	GJamLastMatch.Theirs = Theirs;
	GJamLastMatch.Level = LevelLabel;
	GJamLastMatch.Rink = RinkNameIn;
}

bool UJamMenuFlow::GetLastMatch(int32& OutMine, int32& OutTheirs, FString& OutLevel, FString& OutRink)
{
	if (!GJamLastMatch.bSet) return false;
	OutMine = GJamLastMatch.Mine;
	OutTheirs = GJamLastMatch.Theirs;
	OutLevel = GJamLastMatch.Level;
	OutRink = GJamLastMatch.Rink;
	return true;
}

void UJamMenuFlow::ClearLastMatchForTest() { GJamLastMatch = FJamLastMatchNote(); }

FString UJamMenuFlow::RinkName(const FString& CutName)
{
	FJamCutInfo I;
	I.Name = CutName;
	FString Shown = DisplayNameFor(I).ToUpper();
	if (Shown.EndsWith(TEXT(" CUT"))) Shown.LeftChopInline(4);
	return Shown;
}
