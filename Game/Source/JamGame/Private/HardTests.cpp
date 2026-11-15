// Lane A198 (track S3 hardening, evolution 5): the hardening tests of 14 Nov (docs/experiments/HARDENING-5.md).
//
//   Jam.Hard.InputFuzzSmoke   in the gate: 4 matches of the input fuzz below, 3,000 frames each.
//   Jam.Hard.BindingsFile     in the gate: the saved bindings as a damaged file gives them (missing, empty, cut in half, unknown keys, two verbs on one key, a key
//                             that no longer exists): never a crash, always a table that is valid and keeps every verb reachable; a rebinding round-trips.
//   Bar.Hard.InputFuzz        the long run: JAM_HARD_MATCHES matches (default 200) of random presses on every rebindable verb, through random rebindings and
//                             the two-role button resolver (FJamButtonResolver, the held cross-check FJamInput::bCrossHold included), on one skater of each
//                             team while the brain plays the rest; every match played twice. Laws: nothing not finite, nobody and no puck outside the rink,
//                             every goal pause ends on its frame (goal_pause_frames; a wind-up runs out inside it), the match ends by its hard stop, the
//                             lockstep hash of the two plays equal, every rebinding keeps the table valid. Optional env JAM_HARD_OUT: faults.jsonl there.
//   Lane A212 (the night of 14 to 15 Nov): a quarter of the hand's presses are long (about a second: the pass's power hold, game.pass.power.full_frames, the
//                             held shot, the held cross-check), and every match is scanned for the puck escaping (ScanEscape below).
//   Bar.Hard.PuckEscape       the brain alone (no hands) over the case list, JAM_HARD_SEEDS seeds a case (default 10): the puck outside the boards, sitting in a net
//                             while play runs, scored through a net's side or back, inside a standing skater who does not carry it for more than 2 frames.
//                             One JAMHARD escape line a class with the count and the first case and frame; it fails on any.
//   Bar.Hard.AcceptRows       lane A212: the acceptance drill's rows of steps 1 to 3 and 7 to 10 replayed in the sim through the mapping layer (see JamHardRows).
//   Bar.Hard.BindingsWrite / Bar.Hard.BindingsRead   the rebinding survives a restart: Write rebinds and saves through FJamSettings (the profile's file;
//                             run it with -MultiprocessSaveConfig so a -Multiprocess process writes to disk), Read, in a NEW process, loads and compares.
// The tests report; they fix nothing in another lane's family.
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformMisc.h"
#include "Math/RandomStream.h"
#include "InputMapping.h"
#include "SettingsWidget.h"
#include "JamSim.h"
#include "JamCartridge.h"
#include "JamBotMatch.h"
#include "JamBrain.h"
#include "JamSimAdapter.h"
#include "DifficultyResolve.h"
#include "MenuGameMode.h"
#include "MenuWidget.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "HAL/IConsoleManager.h"
#include "Framework/Application/SlateApplication.h"
#include "UObject/UObjectIterator.h"
#include "Components/Widget.h"
#include "HokejGameMode.h"   // lane A212: AHokejGameMode::CursorAimFromHit (the acceptance rows)
#include "JamEventBus.h"

// Lane A198: the front-end fuzz's eye (tools/fuzz/menu-fuzz.mjs). One line: the map, the game mode, the menu's screen, the widget that holds the
// keyboard focus and whether it sits inside the menu. QA only (development builds with automation).
static FAutoConsoleCommandWithWorldAndArgs CmdJamHardMenuFocus(TEXT("jam.hard.menufocus"),
	TEXT("Lane A198 QA: prints JAMHARD focus map=<map> mode=<game mode> screen=<menu screen|none> focused=<widget type|none> in_menu=<0|1>"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		const AGameModeBase* GMB = World ? World->GetAuthGameMode() : nullptr;
		const AJamMenuGameMode* GM = Cast<AJamMenuGameMode>(GMB);
		UJamMenuWidget* Menu = GM ? GM->GetMenu() : nullptr;
		TSharedPtr<SWidget> F = FSlateApplication::IsInitialized() ? FSlateApplication::Get().GetUserFocusedWidget(0) : nullptr;
		bool bIn = false;
		const TSharedPtr<SWidget> Root = Menu ? Menu->GetCachedWidget() : nullptr;
		for (TSharedPtr<SWidget> W = F; W.IsValid() && Root.IsValid(); W = W->GetParentWidget()) if (W == Root) { bIn = true; break; }
		// the card or row that holds the focus (its payload: the cut a card starts), when the focus sits on one
		FString Card = TEXT("none");
		for (TObjectIterator<UJamMenuRowButton> It; It && F.IsValid(); ++It)
		{
			const TSharedPtr<SWidget> C = It->GetCachedWidget();
			if (!C.IsValid() || It->GetWorld() != World) continue;
			for (TSharedPtr<SWidget> W = F; W.IsValid(); W = W->GetParentWidget()) if (W == C) { Card = It->Payload.IsEmpty() ? TEXT("(row)") : It->Payload; break; }
			if (Card != TEXT("none")) break;
		}
		// the UMG widgets around the focus (nearest first: its name and class), and whether the focused widget and every parent are visible
		TArray<FString> Owners;
		bool bShown = F.IsValid();
		for (TSharedPtr<SWidget> W = F; W.IsValid() && Owners.Num() < 3; W = W->GetParentWidget())
		{
			if (!W->GetVisibility().IsVisible()) bShown = false;
			for (TObjectIterator<UWidget> It; It; ++It)
			{
				if (It->GetWorld() != World || It->GetCachedWidget() != W) continue;
				Owners.Add(FString::Printf(TEXT("%s:%s"), *It->GetName(), *It->GetClass()->GetName()));
				break;
			}
		}
		for (TSharedPtr<SWidget> W = F; W.IsValid(); W = W->GetParentWidget()) if (!W->GetVisibility().IsVisible()) bShown = false;
		const FVector2D Size = F.IsValid() ? FVector2D(F->GetTickSpaceGeometry().GetLocalSize()) : FVector2D::ZeroVector;
		UE_LOG(LogTemp, Display, TEXT("JAMHARD focus map=%s mode=%s screen=%s focused=%s in_menu=%d card=%s owners=%s shown=%d size=%.0fx%.0f"), World ? *World->GetMapName() : TEXT("none"),
			GMB ? *GMB->GetClass()->GetName() : TEXT("none"), Menu ? *UJamMenuWidget::ScreenName(Menu->GetScreen()) : TEXT("none"),
			F.IsValid() ? *F->GetTypeAsString() : TEXT("none"), bIn ? 1 : 0, *Card, Owners.Num() ? *FString::Join(Owners, TEXT(">")) : TEXT("none"), bShown ? 1 : 0, Size.X, Size.Y);
	}));

namespace JamHard
{
	constexpr EAutomationTestFlags GateFlags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;
	constexpr EAutomationTestFlags LongFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::CommandletContext | EAutomationTestFlags::EngineFilter;

	// The gate's tests log a verb left without any key as a finding (JAMHARD finding ...); the Bar runs fail on it (the fault probe).
	bool GStrictReach = false;

	int32 EnvInt(const TCHAR* Name, int32 Default) { const FString V = FPlatformMisc::GetEnvironmentVariable(Name); return V.IsEmpty() ? Default : FCString::Atoi(*V); }

	void Fault(FAutomationTestBase& T, const FString& Class, const FString& Case, const FString& Detail)
	{
		T.AddError(FString::Printf(TEXT("HARDFAULT %s %s: %s"), *Class, *Case, *Detail));
		const FString Out = FPlatformMisc::GetEnvironmentVariable(TEXT("JAM_HARD_OUT"));
		if (Out.IsEmpty()) return;
		const FString Line = FString::Printf(TEXT("{\"class\":\"%s\",\"case\":\"%s\",\"detail\":\"%s\"}\n"), *Class, *Case, *Detail.Replace(TEXT("\""), TEXT("'")));
		FFileHelper::SaveStringToFile(Line, *FPaths::Combine(Out, TEXT("faults.jsonl")), FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
	}

	// A verb is reachable when it has a key or a pad button (the skate verbs: a key; the stick moves them on a pad).
	bool Reachable(const FJamBindings& B, EJamVerb V) { return B[V].Keys.Num() > 0 || B[V].Pads.Num() > 0; }
	FString Unreachable(const FJamBindings& B)
	{
		TArray<FString> Out;
		for (int32 I = 0; I < FJamBindings::NumVerbs; ++I) if (!Reachable(B, static_cast<EJamVerb>(I))) Out.Add(FJamBindings::VerbId(static_cast<EJamVerb>(I)));
		return FString::Join(Out, TEXT(","));
	}

	// The keys a fuzzing hand reaches for: the defaults' keys and pads, some free keys, and reserved ones (Rebind must refuse those).
	TArray<FKey> KeyPool()
	{
		TArray<FKey> P;
		const FJamBindings D = FJamBindings::Defaults();
		for (int32 I = 0; I < FJamBindings::NumVerbs; ++I) { for (const FKey& K : D.Verbs[I].Keys) P.AddUnique(K); for (const FKey& K : D.Verbs[I].Pads) P.AddUnique(K); }
		for (const FKey& K : { EKeys::Z, EKeys::X, EKeys::C, EKeys::V, EKeys::G, EKeys::T, EKeys::Tab, EKeys::LeftAlt, EKeys::Gamepad_FaceButton_Left, EKeys::Gamepad_RightTriggerAxis,
			EKeys::Escape, EKeys::One, EKeys::Gamepad_DPad_Up, EKeys::MouseX, EKeys::Gamepad_Special_Right }) P.AddUnique(K);
		return P;
	}

	// One fuzzing hand: random presses and releases of the pool's keys, resolved into the sim's verbs exactly as the game does
	// (JamControls::ApplyButtons for the two-role buttons, the bindings for the plain verbs).
	struct FHand
	{
		int32 Skater = -1;
		FJamBindings B;
		FJamButtonResolver Resolver;
		TArray<FKey> Pool;
		TArray<bool> Down, NewPress, Long;   // lane A212: Long = this press is held about a second
		int32 PassHoldRun = 0, PassFullHolds = 0, ShotFullHolds = 0, ShotHoldRun = 0;   // lane A212: read-outs (holds of game.pass.power.full_frames or more)
		int32 HeldCross = 0, CrossHoldFrames = 0;   // read-outs for the report

		void Init(FRandomStream& R, int32 InSkater, FAutomationTestBase& T, const FString& Case)
		{
			Skater = InSkater;
			B = FJamBindings::Defaults();
			Pool = KeyPool();
			Down.Init(false, Pool.Num());
			NewPress.Init(false, Pool.Num());
			Long.Init(false, Pool.Num());
			Resolver.Reset();
			// random rebindings of every rebindable verb: the table must stay valid and every verb reachable after each
			const int32 N = R.RandRange(0, 12);
			for (int32 I = 0; I < N; ++I)
			{
				const EJamVerb V = static_cast<EJamVerb>(R.RandRange(0, FJamBindings::NumVerbs - 1));
				const FKey K = Pool[R.RandRange(0, Pool.Num() - 1)];
				const FJamBindings Before = B;
				EJamVerb Swapped = EJamVerb::Count;
				const bool bOk = B.Rebind(V, K, &Swapped);
				FString Why;
				if (!B.IsValid(&Why)) { Fault(T, TEXT("rebind-invalid"), Case, FString::Printf(TEXT("Rebind(%s, %s) left the table invalid: %s"), FJamBindings::VerbId(V), *K.ToString(), *Why)); B = Before; continue; }
				const FString Lost = Unreachable(B);
				if (!Lost.IsEmpty() && Unreachable(Before).IsEmpty() && !GStrictReach) { UE_LOG(LogTemp, Display, TEXT("JAMHARD finding rebind-unreachable %s: Rebind(%s, %s) left verbs without a key or pad: %s"), *Case, FJamBindings::VerbId(V), *K.ToString(), *Lost); }
				else if (!Lost.IsEmpty() && Unreachable(Before).IsEmpty()) { Fault(T, TEXT("rebind-unreachable"), Case, FString::Printf(TEXT("Rebind(%s, %s) ok=%d swapped=%s left verbs without a key or pad: %s"), FJamBindings::VerbId(V), *K.ToString(), bOk ? 1 : 0, FJamBindings::VerbId(Swapped), *Lost)); }
				if (bOk && FJamBindings::IsReserved(K)) { Fault(T, TEXT("rebind-reserved"), Case, FString::Printf(TEXT("Rebind(%s, %s) accepted a reserved key"), FJamBindings::VerbId(V), *K.ToString())); }
			}
			// the saved form round-trips (what a restart reads back)
			if (!(FJamBindings::FromSaved(B.ToSaved()) == B)) { Fault(T, TEXT("bindings-roundtrip"), Case, TEXT("FromSaved(ToSaved(B)) != B after random rebindings")); }
		}

		bool Held(EJamVerb V) const { for (int32 I = 0; I < Pool.Num(); ++I) if (Down[I] && B.Has(V, Pool[I])) return true; return false; }
		bool Pressed(EJamVerb V) const { for (int32 I = 0; I < Pool.Num(); ++I) if (NewPress[I] && B.Has(V, Pool[I])) return true; return false; }

		void Step(FRandomStream& R, const UJamSim& Sim, JamAI::FJamAIInput& In)
		{
			// a burst of mashing now and then, otherwise held presses of about a fifth of a second
			const bool bMash = R.FRand() < 0.05f;
			for (int32 I = 0; I < Pool.Num(); ++I)
			{
				NewPress[I] = false;
				if (Down[I]) { if (R.FRand() < (bMash ? 0.5f : (Long[I] ? 0.015f : 0.08f))) { Down[I] = false; Resolver.Release(Pool[I]); } }
				else if (R.FRand() < (bMash ? 0.3f : 0.025f)) { Down[I] = true; NewPress[I] = true; Long[I] = !bMash && R.FRand() < 0.25f; Resolver.Press(Pool[I]); }
				// a tap: pressed and let go inside one step
				if (NewPress[I] && R.FRand() < 0.2f) { Down[I] = false; Resolver.Release(Pool[I]); }
			}
			const bool bWith = JamControls::HasPuckForButtons(Sim, Skater);
			const FJamButtonVerbs V = Resolver.Step(bWith, B);
			JamAI::FJamAIInput O;
			const double X = (Held(EJamVerb::SkateRight) ? 1.0 : 0.0) - (Held(EJamVerb::SkateLeft) ? 1.0 : 0.0);
			const double Y = (Held(EJamVerb::SkateForward) ? 1.0 : 0.0) - (Held(EJamVerb::SkateBack) ? 1.0 : 0.0);
			const double L = FMath::Sqrt(X * X + Y * Y);
			O.Move = L > 0.0 ? JamAI::FVec2(X / L, Y / L) : JamAI::FVec2(0.0, 0.0);
			if (R.FRand() < 0.5f) { const double A = R.FRandRange(0.f, 2.f * PI); const double M = R.FRand(); O.Stick = JamAI::FVec2(FMath::Cos(A) * M, FMath::Sin(A) * M); }
			O.bShootHeld = V.bShootHeld;
			O.bPassHeld = V.bPassHeld;
			O.bCheck = V.bCheck;
			O.bLift = V.bLift;
			O.bCrossHold = V.bCrossHeld && Sim.CarrierIndex() != Skater;   // JamControls::SetCrossHold as ApplyButtons calls it
			O.bPoke = Pressed(EJamVerb::Poke);
			O.bDeke = Pressed(EJamVerb::Deke);
			O.bSpin = Pressed(EJamVerb::Spin);
			O.bAbility = Held(EJamVerb::Ability);
			O.bSprint = Held(EJamVerb::Sprint);
			O.bBrake = Held(EJamVerb::Brake);
			if (O.bCrossHold) { ++CrossHoldFrames; }
			const int32 Full = FMath::Max(1, Sim.GetConfig().Pass.PowerFullFrames);
			if (O.bPassHeld) ++PassHoldRun; else { if (PassHoldRun >= Full) ++PassFullHolds; PassHoldRun = 0; }
			if (O.bShootHeld) ++ShotHoldRun; else { if (ShotHoldRun >= Full) ++ShotFullHolds; ShotHoldRun = 0; }
			if (V.bLift) { ++HeldCross; }
			In = O;
		}
	};

	// The brain plays everyone; two skaters (one a team) are the fuzzing hands. Watches the laws after every step.
	class FFuzzSim : public JamAI::IJamAISim
	{
	public:
		JamAI::FJamSimAdapter Inner;
		FAutomationTestBase& T;
		FString Case;
		uint64 FuzzSeed = 0;
		FRandomStream R;
		FHand Hands[2];
		int64 PauseStart = -1;
		int32 Pauses = 0, PauseFaults = 0, WorstPause = 0, GoalLen = 0, FoulLen = 0;
		bool bNaN = false, bOut = false;
		bool bHands = true;   // lane A212: false = the brain alone (Bar.Hard.PuckEscape)
		// Lane A212: the escape scan. Classes: 0 board (the puck's centre past the boards under the glass), 1 net (inside a net's box while play runs, more than 2
		// frames), 2 wall goal (a goal whose puck was beside or behind the net the frame before), 3 body (inside a standing skater who is not the carrier, more than 2 frames).
		static constexpr int32 NumEscape = 4;
		int32 Escape[NumEscape] = {}, EscapeFrames[NumEscape] = {};
		FString EscapeFirst[NumEscape];
		int32 NetRun = 0, BodyRun = 0, BodySkater = -1, BoardRun = 0, BodyDeke = 0;   // BodyDeke: body events while the carrier dekes (A208: a deke goes round the man by design)
		FVector2D LastPuck = FVector2D::ZeroVector;
		int32 LastGoals = 0;
		bool bHaveLast = false;

		FFuzzSim(const UJamCartridge& Cart, FAutomationTestBase& InT, const FString& InCase, uint64 InSeed) : Inner(Cart), T(InT), Case(InCase), FuzzSeed(InSeed) {}

		virtual void Reset(const JamAI::FJamMatchConfig& C, uint64_t Seed) override
		{
			Inner.Reset(C, Seed);
			R.Initialize(static_cast<int32>(FuzzSeed * 2654435761ull));
			const UJamSim* S = Inner.GetSim();
			int32 A = -1, Bk = -1;
			for (const FJamSkaterState& K : S->GetSkaters()) { if (K.Team == 0 && A < 0) A = K.Index; if (K.Team == 1 && Bk < 0) Bk = K.Index; }
			Hands[0].Init(R, A, T, Case);
			Hands[1].Init(R, Bk, T, Case);
			PauseStart = -1; Pauses = 0; PauseFaults = 0; WorstPause = 0; GoalLen = 0; FoulLen = 0; bNaN = false; bOut = false;
			for (int32 E = 0; E < NumEscape; ++E) { Escape[E] = 0; EscapeFrames[E] = 0; EscapeFirst[E].Reset(); }
			NetRun = 0; BodyRun = 0; BoardRun = 0; BodyDeke = 0; BodySkater = -1; bHaveLast = false; LastGoals = 0;
		}
		virtual void View(JamAI::FJamAIWorld& W) const override { Inner.View(W); }
		virtual JamAI::FJamAIStepEvents Step(const JamAI::FJamAIInput* In) override
		{
			const UJamSim* S = Inner.GetSim();
			const int32 N = S->GetSkaters().Num();
			TArray<JamAI::FJamAIInput> Mine;
			Mine.SetNum(N);
			for (int32 I = 0; I < N; ++I) Mine[I] = In[I];
			if (bHands) for (FHand& H : Hands) if (H.Skater >= 0 && H.Skater < N) H.Step(R, *S, Mine[H.Skater]);
			const JamAI::FJamAIStepEvents Ev = Inner.Step(Mine.GetData());
			Look();
			ScanEscape();
			return Ev;
		}
		virtual bool IsOver() const override { return Inner.IsOver(); }
		virtual int Winner() const override { return Inner.Winner(); }
		virtual int Score(int Team) const override { return Inner.Score(Team); }
		virtual uint64_t StateHash() const override { return Inner.StateHash(); }
		virtual double BoardDistance(const JamAI::FVec2& P) const override { return Inner.BoardDistance(P); }

		// Lane A212: env JAM_HARD_TRACE="<case>@<frame>" (the case as a JAMHARD line names it, e.g. hockey.regular.cartridge.json@all_star#1@2127): every
		// frame within 8 of it, the puck and every skater within 1.5 m of it, one JAMHARD trace line each.
		void Trace(const UJamSim& S)
		{
			static const FString Want = FPlatformMisc::GetEnvironmentVariable(TEXT("JAM_HARD_TRACE"));
			if (Want.IsEmpty()) return;
			FString C, Fs;
			if (!Want.Split(TEXT("@"), &C, &Fs, ESearchCase::CaseSensitive, ESearchDir::FromEnd) || C != Case) return;
			const int64 F = S.GetFrame(), At = FCString::Atoi64(*Fs);
			if (F < At - 8 || F > At + 8) return;
			const FJamPuckState& P = S.GetPuck();
			UE_LOG(LogTemp, Display, TEXT("JAMHARD trace f%lld puck pos=(%.3f,%.3f) z=%.2f vel=(%.2f,%.2f) last=%d carrier=%d pass=%d/%d phase=%d"), static_cast<long long>(F), P.Pos.X, P.Pos.Y, P.Z, P.Vel.X, P.Vel.Y, P.LastTouch, S.CarrierIndex(), P.PassFrames, P.PassBy, static_cast<int32>(S.GetPhase()));
			for (const FJamSkaterState& K : S.GetSkaters())
			{
				const double D = FVector2D::Distance(K.Pos, P.Pos);
				if (D > 1.5) continue;
				UE_LOG(LogTemp, Display, TEXT("JAMHARD trace f%lld   skater %d team %d role %s d=%.3f pos=(%.3f,%.3f) vel=(%.2f,%.2f) blade=(%.3f,%.3f) down=%d stumble=%d box=%d human=%d deke=%d settle=%d"), static_cast<long long>(F), K.Index, K.Team, *K.Role.ToString(), D,
					K.Pos.X, K.Pos.Y, K.Vel.X, K.Vel.Y, K.BladePos.X, K.BladePos.Y, K.KnockFrames, K.StumbleFrames, K.PenaltyFrames, K.bHuman ? 1 : 0, K.DekeFrames, K.SettleFrames);
			}
		}

		void Note(int32 E, int64 F, const FString& What)
		{
			if (Escape[E]++ == 0) EscapeFirst[E] = FString::Printf(TEXT("%s f%lld %s"), *Case, static_cast<long long>(F), *What);
		}

		void ScanEscape()
		{
			const UJamSim* S = Inner.GetSim();
			const int64 F = S->GetFrame();
			const FJamPuckState& P = S->GetPuck();
			Trace(*S);
			if (!FMath::IsFinite(P.Pos.X) || !FMath::IsFinite(P.Pos.Y)) return;   // Look's NaN law
			const bool bPlay = S->GetPhase() == EJamSimPhase::Playing;
			FVector2D N;
			const double Bd = S->BoardDistance(P.Pos, N);
			const bool bPast = bPlay && P.Z < UJamSim::GlassHeightM && Bd > 0.05;
			BoardRun = bPast ? BoardRun + 1 : 0;
			if (bPast) ++EscapeFrames[0];
			if (BoardRun == 1) { Note(0, F, FString::Printf(TEXT("puck %.2f m past the boards at (%.2f, %.2f) z %.2f v %.1f"), Bd, P.Pos.X, P.Pos.Y, P.Z, P.Vel.Size())); }
			const bool bInNet = S->InGoal(P.Pos, 1, P.Z) || S->InGoal(P.Pos, -1, P.Z);
			NetRun = (bPlay && bInNet) ? NetRun + 1 : 0;
			if (NetRun == 3) Note(1, F, FString::Printf(TEXT("puck in a net 3 frames in play at (%.2f, %.2f) z %.2f v %.1f"), P.Pos.X, P.Pos.Y, P.Z, P.Vel.Size()));
			if (NetRun >= 3) ++EscapeFrames[1];
			const int32 Goals = S->GetScore(0) + S->GetScore(1);
			if (bHaveLast && Goals > LastGoals)
			{
				const double GL = S->GoalLineX(), R0 = 0.05;
				const double Ax = FMath::Abs(LastPuck.X), HalfW = S->GetConfig().Rink.GoalWidthM * 0.5;
				// beside the posts or behind the back of the net, past the goal line (a puck on the line between the posts came in from the front)
				const bool bBeside = Ax > GL + R0 && (FMath::Abs(LastPuck.Y) >= HalfW + R0 || Ax >= GL + UJamSim::NetDepth);
				if (bBeside) { ++EscapeFrames[2]; Note(2, F, FString::Printf(TEXT("goal with the puck at (%.2f, %.2f) the frame before (goal line %.2f, net depth %.2f, half width %.2f)"), LastPuck.X, LastPuck.Y, GL, UJamSim::NetDepth, S->GetConfig().Rink.GoalWidthM * 0.5)); }
			}
			LastGoals = Goals; LastPuck = P.Pos; bHaveLast = true;
			int32 Inside = -1;
			if (bPlay && P.Z < UJamSim::BodyHeightM)
			{
				const int32 Car = S->CarrierIndex();
				for (const FJamSkaterState& K : S->GetSkaters())
					if (K.Index != Car && !K.IsDown() && FVector2D::Distance(K.Pos, P.Pos) < UJamSim::SkaterRadius - 0.05) { Inside = K.Index; break; }
			}
			BodyRun = (Inside >= 0 && Inside == BodySkater) ? BodyRun + 1 : (Inside >= 0 ? 1 : 0);
			BodySkater = Inside;
			const int32 Car = S->CarrierIndex();
			const bool bDeke = S->GetSkaters().IsValidIndex(Car) && S->GetSkaters()[Car].DekeFrames > 0;
			if (BodyRun == 3) { if (bDeke) ++BodyDeke; Note(3, F, FString::Printf(TEXT("puck inside skater %d for 3 frames (%.2f m from his centre, v %.1f, last touch %d, carrier %d deking %d, pass in flight %d)"), Inside,
				FVector2D::Distance(S->GetSkaters()[Inside].Pos, P.Pos), P.Vel.Size(), P.LastTouch, Car, bDeke ? 1 : 0, P.PassFrames)); }
			if (BodyRun >= 3) ++EscapeFrames[3];
		}

		void Look()
		{
			const UJamSim* S = Inner.GetSim();
			const int64 F = S->GetFrame();
			// the goal pause ends on its frame: goal_pause_frames steps after the step that began it
			const int32 G = S->GetConfig().Rules.GoalPauseFrames;
			if (S->GetPhase() == EJamSimPhase::GoalPause) { if (PauseStart < 0) { PauseStart = F; ++Pauses; } }
			else if (PauseStart >= 0)
			{
				const int32 Len = static_cast<int32>(F - PauseStart);
				WorstPause = FMath::Max(WorstPause, Len);
				// a goal's pause is goal_pause_frames; a foul's (JamSim.cpp: the trip, the box) is UJamSim::FoulPauseFrames
				if (Len == G) ++GoalLen; else if (Len == UJamSim::FoulPauseFrames) ++FoulLen;
				else if (PauseFaults++ < 3) Fault(T, TEXT("pause-length"), Case, FString::Printf(TEXT("a pause began at frame %lld and ended after %d frames (goal_pause_frames %d, foul pause %d)"), PauseStart, Len, G, UJamSim::FoulPauseFrames));
				PauseStart = -1;
			}
			if (PauseStart >= 0 && F - PauseStart > FMath::Max(G, static_cast<int32>(UJamSim::FoulPauseFrames)) + 2 && PauseFaults++ < 3) Fault(T, TEXT("pause-never-ends"), Case, FString::Printf(TEXT("a goal pause from frame %lld still runs at frame %lld (goal_pause_frames %d)"), PauseStart, F, G));
			if (!bNaN)
			{
				const FJamPuckState& P = S->GetPuck();
				bool b = !FMath::IsFinite(P.Pos.X) || !FMath::IsFinite(P.Pos.Y) || !FMath::IsFinite(P.Vel.X) || !FMath::IsFinite(P.Vel.Y);
				for (const FJamSkaterState& K : S->GetSkaters()) b |= !FMath::IsFinite(K.Pos.X) || !FMath::IsFinite(K.Pos.Y) || !FMath::IsFinite(K.Vel.X) || !FMath::IsFinite(K.Vel.Y);
				if (b) { bNaN = true; Fault(T, TEXT("nan"), Case, FString::Printf(TEXT("not finite at frame %lld"), F)); }
			}
			if (!bOut && !bNaN)
			{
				const FJamPuckState& P = S->GetPuck();
				if (Inner.BoardDistance(JamAI::FVec2(P.Pos.X, P.Pos.Y)) > 1.5) { bOut = true; Fault(T, TEXT("puck-outside"), Case, FString::Printf(TEXT("puck outside the boards at frame %lld"), F)); }
				for (const FJamSkaterState& K : S->GetSkaters())
				{
					if (!bOut && Inner.BoardDistance(JamAI::FVec2(K.Pos.X, K.Pos.Y)) > 0.75) { bOut = true; Fault(T, TEXT("skater-outside"), Case, FString::Printf(TEXT("skater %d outside the boards at frame %lld"), K.Index, F)); }
				}
			}
		}
	};

	struct FCaseSpec { FString File, Level; };

	TArray<FCaseSpec> CaseList()
	{
		TArray<FCaseSpec> Out;
		const FString Dir = UJamCartridge::GetCartridgeDir();
		TArray<FString> Files = { TEXT("hockey.regular.cartridge.json"), TEXT("hockey.arena.large.cartridge.json"), TEXT("hockey.boost.cartridge.json"), TEXT("hockey.chaos.cartridge.json") };
		TArray<FString> Modes;
		IFileManager::Get().FindFiles(Modes, *FPaths::Combine(Dir, TEXT("modes"), TEXT("*.cartridge.json")), true, false);
		Modes.Sort();
		for (const FString& M : Modes) Files.Add(FString(TEXT("modes/")) + M);
		for (const FString& F : Files) for (const TCHAR* L : { TEXT("rookie"), TEXT("pro"), TEXT("all_star") }) Out.Add({ F, L });
		return Out;
	}

	// One match twice. Returns false when the case could not run.
	// Lane A212: the escape scan's totals over a run (per class: matches with it, events, frames, the first case).
	struct FEscapeTotals
	{
		int32 Matches[4] = {}, Events[4] = {}, Frames[4] = {};
		FString First[4];
		int32 PassFull = 0, ShotFull = 0, BodyDeke = 0;
		void Add(const int32* Ev, const int32* Fr, const FString* Fi)
		{
			for (int32 E = 0; E < 4; ++E) if (Ev[E] > 0) { ++Matches[E]; Events[E] += Ev[E]; Frames[E] += Fr[E]; if (First[E].IsEmpty()) First[E] = Fi[E]; }
		}
		void Log(int32 Ran) const
		{
			static const TCHAR* Names[4] = { TEXT("board"), TEXT("net"), TEXT("wall_goal"), TEXT("body") };
			for (int32 E = 0; E < 4; ++E)
				UE_LOG(LogTemp, Display, TEXT("JAMHARD escape class=%s matches=%d/%d events=%d frames=%d%s first=%s"), Names[E], Matches[E], Ran, Events[E], Frames[E], E == 3 ? *FString::Printf(TEXT(" while_deking=%d"), BodyDeke) : TEXT(""), First[E].IsEmpty() ? TEXT("none") : *First[E]);
		}
	};
	FEscapeTotals GEscape;
	bool GHands = true;

	bool RunCase(FAutomationTestBase& T, const FCaseSpec& Spec, uint64 Seed, int64 MaxFrames, int32& OutPauses, int32& OutCross, bool& bOver)
	{
		TArray<FString> Errors;
		UJamCartridge* Cart = UJamCartridge::LoadFromFile(FPaths::Combine(UJamCartridge::GetCartridgeDir(), Spec.File), Errors);
		const FString Case = FString::Printf(TEXT("%s@%s#%llu"), *Spec.File, *Spec.Level, Seed);
		if (!Cart) { Fault(T, TEXT("harness"), Case, FString::Printf(TEXT("cartridge did not load: %s"), Errors.Num() ? *Errors[0] : TEXT("?"))); return false; }
		const JamAI::FJamMatchConfig Cfg = JamAI::ToMatchConfigResolved(*Cart, *Spec.Level);
		uint64 Hash[2] = { 0, 0 };
		int64 Frames[2] = { 0, 0 };
		for (int32 Run = 0; Run < (GHands ? 2 : 1); ++Run)
		{
			// the game mode's loop (JamAI::RunMatch is not exported from JamCore): the brain thinks for everyone, the two hands replace their skaters' inputs
			FFuzzSim Sim(*Cart, T, Run == 0 ? Case : Case + TEXT(" (replay)"), Seed);
			Sim.bHands = GHands;
			Sim.Reset(Cfg, Seed);
			UJamBrain* Brain = NewObject<UJamBrain>();
			Brain->ApplyCartridge(*Cart, *Spec.Level);
			Brain->ResetBrain();
			JamAI::FJamAIInput Out[JamAI::kMaxSkaters];
			int64 F = 0;
			for (; F < MaxFrames && !Sim.IsOver(); ++F)
			{
				JamAI::FJamAIWorld W;
				Sim.View(W);
				W.PlayerIndex = -1;   // every slot a bot (RunMatch's Brain mode); the hands override two of them
				for (JamAI::FJamAIInput& O : Out) O = JamAI::FJamAIInput();
				Brain->Think(W, Out);
				Sim.Step(Out);
			}
			Hash[Run] = Sim.Inner.GetSim()->LockstepStateHash();
			Frames[Run] = F;
			if (Run == 0)
			{
				OutPauses = Sim.Pauses;
				OutCross = Sim.Hands[0].CrossHoldFrames + Sim.Hands[1].CrossHoldFrames;
				bOver = Sim.IsOver();
				GEscape.Add(Sim.Escape, Sim.EscapeFrames, Sim.EscapeFirst);
				GEscape.BodyDeke += Sim.BodyDeke;
				GEscape.PassFull += Sim.Hands[0].PassFullHolds + Sim.Hands[1].PassFullHolds;
				GEscape.ShotFull += Sim.Hands[0].ShotFullHolds + Sim.Hands[1].ShotFullHolds;
				// the sim stops a match at max(33000, timer + 6000) frames: a full-length run must be over
				if (MaxFrames >= 36000 && !bOver) Fault(T, TEXT("never-ends"), Case, FString::Printf(TEXT("not over after %lld frames, %d:%d"), static_cast<long long>(F), Sim.Score(0), Sim.Score(1)));
			}
		}
		if (GHands && (Hash[0] != Hash[1] || Frames[0] != Frames[1]))
			Fault(T, TEXT("hash"), Case, FString::Printf(TEXT("lockstep hash %016llx / %016llx, frames %lld / %lld"), static_cast<unsigned long long>(Hash[0]), static_cast<unsigned long long>(Hash[1]), static_cast<long long>(Frames[0]), static_cast<long long>(Frames[1])));
		return true;
	}

	void RunFuzz(FAutomationTestBase& T, int32 Matches, int64 MaxFrames, int32 First)
	{
		const TArray<FCaseSpec> Cases = CaseList();
		int32 Ran = 0, Pauses = 0, Cross = 0, Over = 0;
		GEscape = FEscapeTotals();
		for (int32 I = 0; I < Matches; ++I)
		{
			const FCaseSpec& Spec = Cases[(First + I) % Cases.Num()];
			int32 P = 0, C = 0; bool bOver = false;
			if (RunCase(T, Spec, static_cast<uint64>(1000 + First + I), MaxFrames, P, C, bOver)) { ++Ran; Pauses += P; Cross += C; Over += bOver ? 1 : 0; }
		}
		UE_LOG(LogTemp, Display, TEXT("JAMHARD inputfuzz matches=%d over=%d goal_pauses=%d cross_hold_frames=%d cases=%d pass_full_holds=%d shot_full_holds=%d"), Ran, Over, Pauses, Cross, Cases.Num(), GEscape.PassFull, GEscape.ShotFull);
		GEscape.Log(Ran);
	}

	// The saved bindings as a damaged file gives them to FromSaved (FJamSettings::Load passes the [Jam.Controls] section through).
	struct FFileCase { const TCHAR* Name; TMap<FString, FString> Saved; };

	TArray<FFileCase> FileCases()
	{
		TArray<FFileCase> C;
		C.Add({ TEXT("missing"), {} });
		{ FFileCase K{ TEXT("empty values"), {} }; for (int32 I = 0; I < FJamBindings::NumVerbs; ++I) K.Saved.Add(FJamBindings::VerbId(static_cast<EJamVerb>(I)), TEXT("")); C.Add(K); }
		{ FFileCase K{ TEXT("bars only"), {} }; for (int32 I = 0; I < FJamBindings::NumVerbs; ++I) K.Saved.Add(FJamBindings::VerbId(static_cast<EJamVerb>(I)), TEXT("|")); C.Add(K); }
		{
			// cut in half: the first half of the verbs as saved, the line where the cut falls ends mid-name, the rest absent
			FFileCase K{ TEXT("cut in half"), {} };
			const TMap<FString, FString> Full = FJamBindings::Defaults().ToSaved();
			int32 I = 0;
			for (const TPair<FString, FString>& P : Full) { if (I < FJamBindings::NumVerbs / 2) K.Saved.Add(P.Key, P.Value); else if (I == FJamBindings::NumVerbs / 2) K.Saved.Add(P.Key, P.Value.Left(P.Value.Len() / 2)); ++I; }
			C.Add(K);
		}
		C.Add({ TEXT("unknown verbs"), { { TEXT("Jump"), TEXT("SpaceBar|") }, { TEXT("Shoot "), TEXT("Z|") }, { TEXT(""), TEXT("|") } } });
		C.Add({ TEXT("unknown keys"), { { TEXT("Shoot"), TEXT("NoSuchKey,LeftMouseButton|Gamepad_Nope") }, { TEXT("Poke"), TEXT("Keyboard_Space|") } } });
		C.Add({ TEXT("two verbs on one key"), { { TEXT("Poke"), TEXT("W|") } } });
		C.Add({ TEXT("two verbs on one key (pads)"), { { TEXT("Spin"), TEXT("E|Gamepad_FaceButton_Bottom") } } });
		C.Add({ TEXT("key that no longer exists (shoot)"), { { TEXT("Shoot"), TEXT("Mouse_Left|RightTrigger") } } });
		C.Add({ TEXT("key that no longer exists (skate forward)"), { { TEXT("SkateForward"), TEXT("Keyboard_W|") } } });
		C.Add({ TEXT("reserved keys only"), { { TEXT("Ability"), TEXT("Escape,One|Gamepad_DPad_Up") } } });
		C.Add({ TEXT("pad key in the keys half"), { { TEXT("Sprint"), TEXT("Gamepad_LeftTriggerAxis|LeftShift") } } });
		C.Add({ TEXT("garbage"), { { TEXT("Pass"), TEXT("\x01\x02|||,,,") }, { TEXT("Check"), FString::ChrN(5000, TEXT('A')) } } });
		C.Add({ TEXT("duplicates"), { { TEXT("Brake"), TEXT("LeftControl,LeftControl,LeftControl|Gamepad_LeftThumbstick,Gamepad_LeftThumbstick") } } });
		return C;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHardBindingsReach, "Bar.Hard.BindingsReach", JamHard::LongFlags)
bool FJamHardBindingsReach::RunTest(const FString& Parameters)
{
	// the probe: the file cases and 200 random rebinding sequences, failing on a verb left without any key or pad button
	JamHard::GStrictReach = true;
	for (const JamHard::FFileCase& C : JamHard::FileCases())
	{
		const FString Lost = JamHard::Unreachable(FJamBindings::FromSaved(C.Saved));
		if (!Lost.IsEmpty()) JamHard::Fault(*this, TEXT("file-unreachable"), C.Name, FString::Printf(TEXT("FromSaved left verbs without a key or pad: %s"), *Lost));
	}
	for (int32 I = 0; I < 200; ++I) { FRandomStream R(7000 + I); JamHard::FHand H; H.Init(R, 0, *this, FString::Printf(TEXT("rebind-seq#%d"), I)); }
	JamHard::GStrictReach = false;
	return true;
}

// FAULT 2's smallest repeat (status/A198.md): one rebinding from the defaults strips the mouse's shoot and puts LMB on skate back. Bar: it fails
// until InputMapping.cpp's Rebind keeps the clash off the verb that never touched the key.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHardRebindRepro, "Bar.Hard.RebindRepro", JamHard::LongFlags)
bool FJamHardRebindRepro::RunTest(const FString& Parameters)
{
	FJamBindings B = FJamBindings::Defaults();
	EJamVerb Swapped = EJamVerb::Count;
	const bool bOk = B.Rebind(EJamVerb::CrossCheck, EKeys::Down, &Swapped);
	auto Keys = [&](EJamVerb V) { TArray<FString> K; for (const FKey& X : B[V].Keys) K.Add(X.ToString()); return FString::Join(K, TEXT(",")); };
	UE_LOG(LogTemp, Display, TEXT("JAMHARD rebind-repro Rebind(CrossCheck, Down) ok=%d swapped=%s valid=%d shoot=[%s] crosscheck=[%s] skateback=[%s]"), bOk ? 1 : 0,
		FJamBindings::VerbId(Swapped), B.IsValid() ? 1 : 0, *Keys(EJamVerb::Shoot), *Keys(EJamVerb::CrossCheck), *Keys(EJamVerb::SkateBack));
	TestTrue(TEXT("Shoot keeps LeftMouseButton (it never touched Down)"), B.Has(EJamVerb::Shoot, EKeys::LeftMouseButton));
	TestFalse(TEXT("LeftMouseButton does not skate back"), B.Has(EJamVerb::SkateBack, EKeys::LeftMouseButton));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHardInputFuzzSmoke, "Jam.Hard.InputFuzzSmoke", JamHard::GateFlags)
bool FJamHardInputFuzzSmoke::RunTest(const FString& Parameters)
{
	JamHard::RunFuzz(*this, 4, 3000, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHardInputFuzz, "Bar.Hard.InputFuzz", JamHard::LongFlags)
bool FJamHardInputFuzz::RunTest(const FString& Parameters)
{
	JamHard::RunFuzz(*this, JamHard::EnvInt(TEXT("JAM_HARD_MATCHES"), 200), JamHard::EnvInt(TEXT("JAM_HARD_FRAMES"), 36000), JamHard::EnvInt(TEXT("JAM_HARD_FIRST"), 0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHardPuckEscape, "Bar.Hard.PuckEscape", JamHard::LongFlags)
bool FJamHardPuckEscape::RunTest(const FString& Parameters)
{
	// Lane A212: the brain alone over the case list; every case JAM_HARD_SEEDS seeds (from JAM_HARD_FIRST, default seed 1), full length.
	using namespace JamHard;
	const TArray<FCaseSpec> Cases = CaseList();
	const int32 Seeds = EnvInt(TEXT("JAM_HARD_SEEDS"), 10), First = EnvInt(TEXT("JAM_HARD_FIRST"), 1);
	GHands = false;
	GEscape = FEscapeTotals();
	int32 Ran = 0;
	for (const FCaseSpec& Spec : Cases)
		for (int32 I = 0; I < Seeds; ++I)
		{
			int32 P = 0, Cr = 0; bool bOver = false;
			if (RunCase(*this, Spec, static_cast<uint64>(First + I), EnvInt(TEXT("JAM_HARD_FRAMES"), 36000), P, Cr, bOver)) ++Ran;
		}
	GHands = true;
	UE_LOG(LogTemp, Display, TEXT("JAMHARD puckescape matches=%d cases=%d seeds=%d"), Ran, Cases.Num(), Seeds);
	GEscape.Log(Ran);
	static const TCHAR* Names[4] = { TEXT("board"), TEXT("net"), TEXT("wall_goal"), TEXT("body") };
	for (int32 E = 0; E < 4; ++E)
		if (GEscape.Matches[E] > 0) Fault(*this, FString::Printf(TEXT("escape-%s"), Names[E]), GEscape.First[E], FString::Printf(TEXT("%d of %d matches, %d events, %d frames"), GEscape.Matches[E], Ran, GEscape.Events[E], GEscape.Frames[E]));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHardBindingsFile, "Jam.Hard.BindingsFile", JamHard::GateFlags)
bool FJamHardBindingsFile::RunTest(const FString& Parameters)
{
	for (const JamHard::FFileCase& C : JamHard::FileCases())
	{
		const FJamBindings B = FJamBindings::FromSaved(C.Saved);
		FString Why;
		TestTrue(FString::Printf(TEXT("%s: the table read back is valid (%s)"), C.Name, *Why), B.IsValid(&Why));
		const FString Lost = JamHard::Unreachable(B);
		UE_LOG(LogTemp, Display, TEXT("JAMHARD bindings case=\"%s\" valid=%d defaults=%d unreachable=%s"), C.Name, B.IsValid() ? 1 : 0, B == FJamBindings::Defaults() ? 1 : 0,
			Lost.IsEmpty() ? TEXT("none") : *Lost);
		if (JamHard::GStrictReach) TestTrue(FString::Printf(TEXT("%s: every verb keeps a key or a pad button (lost: %s)"), C.Name, *Lost), Lost.IsEmpty());
	}
	// a rebinding made in a match survives the saved form
	FJamBindings B = FJamBindings::Defaults();
	B.Rebind(EJamVerb::Pass, EKeys::Z);
	B.Rebind(EJamVerb::Spin, EKeys::Gamepad_FaceButton_Left);
	TestTrue(TEXT("a rebinding round-trips through the saved form"), FJamBindings::FromSaved(B.ToSaved()) == B);
	return true;
}

// The restart: Write in one process, Read in the next. Both use the real settings file (FJamSettings::Load / Save), never the test seam.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHardBindingsWrite, "Bar.Hard.BindingsWrite", JamHard::LongFlags)
bool FJamHardBindingsWrite::RunTest(const FString& Parameters)
{
	FJamSettings St = FJamSettings::Load();
	FJamBindings B = St.Controls.Num() ? FJamBindings::FromSaved(St.Controls) : FJamBindings::Defaults();
	B = FJamBindings::Defaults();
	TestTrue(TEXT("rebind pass to Z"), B.Rebind(EJamVerb::Pass, EKeys::Z));
	TestTrue(TEXT("rebind spin to G"), B.Rebind(EJamVerb::Spin, EKeys::G));
	St.Controls = B.ToSaved();
	St.Save();
	UE_LOG(LogTemp, Display, TEXT("JAMHARD bindings written to_disk=%d file=%s pass=%s spin=%s"), FJamSettings::WritesReachDisk() ? 1 : 0, *GGameUserSettingsIni, *St.Controls.FindRef(TEXT("Pass")), *St.Controls.FindRef(TEXT("Spin")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHardBindingsRead, "Bar.Hard.BindingsRead", JamHard::LongFlags)
bool FJamHardBindingsRead::RunTest(const FString& Parameters)
{
	const FJamSettings St = FJamSettings::Load();
	const FJamBindings B = St.Controls.Num() ? FJamBindings::FromSaved(St.Controls) : FJamBindings::Defaults();
	const bool bPass = B.Has(EJamVerb::Pass, EKeys::Z), bSpin = B.Has(EJamVerb::Spin, EKeys::G);
	UE_LOG(LogTemp, Display, TEXT("JAMHARD bindings read controls=%d pass_on_z=%d spin_on_g=%d valid=%d unreachable=%s file=%s"), St.Controls.Num(), bPass ? 1 : 0, bSpin ? 1 : 0, B.IsValid() ? 1 : 0,
		*JamHard::Unreachable(B), *GGameUserSettingsIni);
	const FString Expect = FPlatformMisc::GetEnvironmentVariable(TEXT("JAM_HARD_EXPECT"));   // "rebound" (after Write) or "any" (a damaged file: only no crash and a valid table)
	if (Expect == TEXT("rebound")) { TestTrue(TEXT("the rebinding survived the restart (pass on Z, spin on G)"), bPass && bSpin); }
	TestTrue(TEXT("the table read back is valid"), B.IsValid());
	return true;
}

// Lane A212 (track S3, the night of 14 to 15 Nov): the release's acceptance rows of steps 1 to 3 and 7 to 10 (HokejAcceptDrill.cpp) replayed in the sim, as lane
// A207's Jam.Core.PassShotRows replays steps 4 to 6. Each row is staged as the drill stages it (the human, the mate or the opponent, the rest parked in the
// human's own corner, the puck pinned until handed), the driver's hand goes through the mapping layer (the default bindings, FJamButtonResolver with the
// game's own ButtonContext, AHokejGameMode::CursorAimFromHit for the cursor as a still screen point that the camera carries along with the skater), the
// drill's scripts move the mate and the opponent, and the drill's own verdict is applied. What it cannot replay: the camera rows (step 8's view target and
// distance), the pause's wall-clock gap (step 10 k=1 aims after it), the hand's latency (presses land on the frame the drill cues them).
// One JAMHARD acceptrow line a row; the test fails on a red row. Env JAM_HARD_ACCEPT_CUT: another cut (default tools/qa/accept/accept-cut.json).
namespace JamHardRows
{
	struct FStage
	{
		TUniquePtr<JamAI::FJamSimAdapter> Adapter;
		JamAI::FJamMatchConfig Cfg;
		JamAI::FJamAIInput In[JamAI::kMaxSkaters];
		UJamSim* Sim = nullptr;
		int32 Me = -1, Mate = -1, Opp = -1;
		TArray<int32> Parked;
		double Dir = 1.0, Gx = 0.0, FaceYaw = 0.0;
		FVector2D PuckPark;
		bool bPin = true, bMateLive = false, bOppLive = false;
		// the hand
		FJamBindings B = FJamBindings::Defaults();
		FJamButtonResolver R;
		TSet<FKey> Down;
		FVector2D CursorRel = FVector2D::ZeroVector;   // the cursor's ice point relative to the skater (zero: not aiming)
		bool bAiming = false;
		bool bWheel = false;
		bool bSpinTap = false;   // the spin key's edge (UJamPlayerInput::ConsumeSpinPressed in the game)
		FJamButtonVerbs LastV;
		TArray<TPair<FKey, int32>> Timers;   // the driver's held keys: released after their frames, across rows as the driver does (a hold outlives its row)
		void HoldFor(const FKey& K, int32 Frames) { Press(K); Timers.RemoveAll([&](const TPair<FKey, int32>& T) { return T.Key == K; }); Timers.Add({ K, FMath::Max(1, Frames) }); }

		FStage(const UJamCartridge& Cart)
		{
			Adapter = MakeUnique<JamAI::FJamSimAdapter>(Cart.Game);
			Cfg = JamAI::ToMatchConfigResolved(Cart, TEXT("rookie"));
			Adapter->Reset(Cfg, 1);
			Sim = Adapter->GetSim();
			Sim->SkipFaceoff();
			Me = Sim->GetHumanIndex();
			const TArray<FJamSkaterState>& Sk = Sim->GetSkaters();
			const int32 Team = Sk[Me].Team;
			for (const FJamSkaterState& S : Sk)
			{
				if (S.Index == Me) continue;
				if (S.Team == Team && Mate < 0 && S.Role != FName(TEXT("back"))) Mate = S.Index;
				else if (S.Team != Team && Opp < 0 && S.Role != FName(TEXT("back"))) Opp = S.Index;
			}
			for (const FJamSkaterState& S : Sk) if (S.Index != Me && S.Index != Mate && S.Index != Opp) Parked.Add(S.Index);
			Gx = Sim->AttackGoalX(Team);
			Dir = Gx > 0 ? 1.0 : -1.0;
			FaceYaw = Dir > 0 ? 0.0 : UE_DOUBLE_PI;
			PuckPark = FVector2D(-Dir * 20.0, -18.0);
		}
		const FJamSkaterState& H() const { return Sim->GetSkaters()[Me]; }
		const FJamSkaterState& O() const { return Sim->GetSkaters()[Opp]; }
		FVector2D Fwd() const { return FVector2D(Dir, 0.0); }
		void Press(const FKey& K) { if (!Down.Contains(K)) { Down.Add(K); R.Press(K); } }
		void Release(const FKey& K) { if (Down.Contains(K)) { Down.Remove(K); R.Release(K); } }
		bool Held(EJamVerb V) const { for (const FKey& K : Down) if (B.Has(V, K)) return true; return false; }
		void HandTo(int32 To) { bPin = false; const FJamSkaterState& T = Sim->GetSkaters()[To]; Sim->SetPuck(T.Pos + T.Forward() * 0.45, T.Vel); }

		// One frame: the drill's AcceptDrillInputs (park, pin, the scripts) and the human's mapped input, then the step.
		void Step(TFunctionRef<void(JamAI::FJamAIInput& Opp)> OppScript)
		{
			int32 Slot = 0;
			auto Park = [&](int32 i) { if (Sim->GetSkaters().IsValidIndex(i)) Sim->SetSkater(i, FVector2D(-Dir * 34.0, -16.0 + 6.5 * Slot++), FVector2D::ZeroVector, 0.0); };
			for (int32 i : Parked) Park(i);
			if (!bMateLive) Park(Mate);
			if (!bOppLive) Park(Opp);
			if (bPin && Sim->GetPhase() == EJamSimPhase::Playing) Sim->SetPuck(PuckPark, FVector2D::ZeroVector);
			for (int32 t = Timers.Num() - 1; t >= 0; --t) if (--Timers[t].Value < 0) { Release(Timers[t].Key); Timers.RemoveAt(t); }
			for (JamAI::FJamAIInput& I : In) I = JamAI::FJamAIInput();
			if (bOppLive && Opp >= 0) OppScript(In[Opp]);
			// the human, as AHokejGameMode::ReadHumanInput builds it (the camera's yaw is the facing; the drill's stage faces the attack)
			JamAI::FJamAIInput& M = In[Me];
			const double Y = (Held(EJamVerb::SkateForward) ? 1.0 : 0.0) - (Held(EJamVerb::SkateBack) ? 1.0 : 0.0);
			const double X = (Held(EJamVerb::SkateRight) ? 1.0 : 0.0) - (Held(EJamVerb::SkateLeft) ? 1.0 : 0.0);
			const FVector2D Fw = Fwd(), Rt(-Fw.Y, Fw.X);
			FVector2D Mv = Fw * Y + Rt * X;
			if (Mv.Size() > 1.0) Mv.Normalize();
			M.Move = JamAI::FVec2(Mv.X, Mv.Y);
			const FJamButtonVerbs V = R.Step(JamControls::ButtonContext(*Sim, Me), B);
			LastV = V;
			M.bShootHeld = V.bShootHeld; M.bPassHeld = V.bPassHeld; M.bCheck = V.bCheck; M.bLift = V.bLift;
			M.bCrossHold = V.bCrossHeld && Sim->CarrierIndex() != Me;
			M.bSprint = Held(EJamVerb::Sprint);
			M.bBrake = Held(EJamVerb::Brake);
			M.bSpin = bSpinTap; bSpinTap = false; M.bDeke = bWheel; bWheel = false;
			FVector2D St = FVector2D::ZeroVector;
			if (!CursorRel.IsZero()) AHokejGameMode::CursorAimFromHit(H().Pos + CursorRel, H().Pos, bAiming, St);
			M.Stick = JamAI::FVec2(St.X, St.Y);
			Adapter->Step(In);
		}
		void Step() { Step([](JamAI::FJamAIInput&) {}); }
	};

	struct FRow { FString Verb; bool bOk = false; FString Facts; };

	void Stage(FStage& S, const FVector2D& HPos, const FVector2D& HVel)
	{
		S.Sim->SetSkater(S.Me, HPos, HVel, S.FaceYaw);
		S.bPin = true;
	}
	double Ang(const FVector2D& A, const FVector2D& Bv) { const FVector2D a = A.GetSafeNormal(), b = Bv.GetSafeNormal(); if (a.IsNearlyZero() || b.IsNearlyZero()) return 180.0; return FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector2D::DotProduct(a, b), -1.0, 1.0))); }

	// Step 1: skate, sprint, sprint_carry, brake. Speed1 / Speed2 carry across the sub-steps as in the drill.
	FRow Skate(const UJamCartridge& C, int32 K, double& Speed1, double& Speed2)
	{
		FStage S(C);
		const FJamGameConfig& Cfg = S.Sim->GetConfig();
		Stage(S, FVector2D(-S.Dir * 22.0, 0.0), FVector2D::ZeroVector);
		for (int32 f = 0; f < 40; ++f) S.Step();
		if (K == 3) S.HandTo(S.Me);
		for (int32 f = 0; f < 45; ++f) S.Step();
		S.Press(EKeys::W);
		if (K == 2 || K == 3) S.Press(EKeys::LeftShift);
		double MaxSpeed = 0.0; int32 Sprint = 0, SprintCarry = 0, Carry = 0;
		if (K < 4)
		{
			for (int32 f = 0; f <= 120; ++f)
			{
				if (f == 120) { S.Release(EKeys::W); S.Release(EKeys::LeftShift); }
				S.Step();
				const FJamSkaterState& H = S.H();
				MaxSpeed = FMath::Max(MaxSpeed, H.Vel.Size());
				const bool bCar = S.Sim->CarrierIndex() == S.Me;
				if (H.bSprinting) { ++Sprint; if (bCar) ++SprintCarry; }
				if (bCar) ++Carry;
			}
			const FString Facts = FString::Printf(TEXT("max_speed=%.2f sprint_frames=%d sprint_carry_frames=%d carry_frames=%d"), MaxSpeed, Sprint, SprintCarry, Carry);
			if (K == 1) { Speed1 = MaxSpeed; return { TEXT("skate"), MaxSpeed > 6.0, Facts }; }
			if (K == 2) { Speed2 = MaxSpeed; return { TEXT("sprint"), Sprint > 0 && MaxSpeed > Speed1 + 0.2, Facts + FString::Printf(TEXT(" walk_speed=%.2f"), Speed1) }; }
			const double CarryWalk = Speed1 * Cfg.Skater.CarrySpeedMult / FMath::Max(1.0, Cfg.Skater.FreeSpeedMult);
			return { TEXT("sprint_carry"), Carry >= 60 && SprintCarry > 0 && MaxSpeed > CarryWalk + 0.2 && MaxSpeed < Speed2 - 0.2, Facts + FString::Printf(TEXT(" carry_walk=%.2f sprint_speed=%.2f"), CarryWalk, Speed2) };
		}
		// k=4 brake: W for 2 s, then S for 1.5 s
		for (int32 f = 0; f < 120; ++f) { S.Step(); MaxSpeed = FMath::Max(MaxSpeed, S.H().Vel.Size()); }
		S.Release(EKeys::W);
		const FVector2D Fwd0 = S.H().Vel.GetSafeNormal();
		S.Press(EKeys::S);
		int64 Rev = -1, Stop = -1; double RevSpeed = 0.0, Back = 0.0; int32 BrakeFrames = 0;
		for (int32 f = 0; f < 420; ++f)
		{
			if (f == 90) S.Release(EKeys::S);
			S.Step();
			const FJamSkaterState& H = S.H();
			const FVector2D Mv(S.In[S.Me].Move.X, S.In[S.Me].Move.Y);
			if (Rev < 0 && Mv.Size() > 0.3 && FVector2D::DotProduct(Mv, H.Vel) < 0.0) { Rev = f; RevSpeed = H.Vel.Size(); }
			if (Rev >= 0 && Stop < 0 && H.Vel.Size() < 0.3) Stop = f;
			if (Stop >= 0) Back = FMath::Max(Back, -FVector2D::DotProduct(H.Vel, Fwd0));
			if (H.bReverseBraking) ++BrakeFrames;
			if (Rev >= 0 && f - Rev > 90) break;
		}
		const int64 StopFrames = (Rev >= 0 && Stop >= 0) ? Stop - Rev : -1;
		return { TEXT("brake"), Rev >= 0 && Stop >= 0 && StopFrames <= 40 && Back > 1.0, FString::Printf(TEXT("max_speed=%.2f rev_speed=%.2f stop_frames=%lld back_speed=%.2f brake_frames=%d"), MaxSpeed, RevSpeed, StopFrames, Back, BrakeFrames) };
	}

	// Steps 2 and 10: the blade follows the cursor within 12 frames of its arrival, 15 degrees. Rel: the target's angle from the facing.
	bool AimOne(const UJamCartridge& C, double RelDeg, FString& Facts)
	{
		FStage S(C);
		Stage(S, FVector2D(-S.Dir * 5.0, 0.0), FVector2D::ZeroVector);
		for (int32 f = 0; f < 40; ++f) S.Step();
		const double A = S.FaceYaw + FMath::DegreesToRadians(RelDeg);
		const FVector2D T = FVector2D(FMath::Cos(A), FMath::Sin(A)) * 2.5;
		S.CursorRel = T;
		int32 Follow = -1; double Err = 180.0;
		for (int32 f = 0; f < 240 && Follow < 0; ++f)
		{
			S.Step();
			const FJamSkaterState& H = S.H();
			Err = Ang(H.BladePos - H.Pos, T);
			if (Err <= 15.0) Follow = f;
		}
		Facts += FString::Printf(TEXT(" rel%.0f=(lat=%d err=%.1f)"), RelDeg, Follow, Err);
		return Follow >= 0 && Follow <= 12;
	}

	// The sweep the driver plays: the cursor from L to R and back, n passes of Ms each, L / R 75 degrees each side of the facing, 3 m out.
	void Sweep(FStage& S, int32 N, int32 Ms, TFunctionRef<void(int32)> Each)
	{
		const FVector2D L = FVector2D(FMath::Cos(S.FaceYaw + FMath::DegreesToRadians(75.0)), FMath::Sin(S.FaceYaw + FMath::DegreesToRadians(75.0))) * 3.0;
		const FVector2D Rr = FVector2D(FMath::Cos(S.FaceYaw - FMath::DegreesToRadians(75.0)), FMath::Sin(S.FaceYaw - FMath::DegreesToRadians(75.0))) * 3.0;
		const int32 PassF = FMath::Max(1, FMath::RoundToInt32(Ms * 0.06));
		int32 F = 0;
		for (int32 p = 0; p < N; ++p)
			for (int32 i = 0; i < PassF; ++i, ++F)
			{
				const double t = static_cast<double>(i + 1) / PassF;
				S.CursorRel = (p % 2 == 0) ? FMath::Lerp(L, Rr, t) : FMath::Lerp(Rr, L, t);
				S.Step();
				Each(F);
			}
	}

	FRow Carry(const UJamCartridge& C)
	{
		FStage S(C);
		Stage(S, FVector2D(-S.Dir * 5.0, 0.0), FVector2D::ZeroVector);
		for (int32 f = 0; f < 40; ++f) S.Step();
		S.HandTo(S.Me);
		S.CursorRel = S.Fwd() * 3.0;
		for (int32 f = 0; f < 45; ++f) S.Step();
		const int32 Sweeps0 = S.H().Sweeps;
		int32 Lost = -1, F = 0;
		auto Look = [&](int32) { if (Lost < 0 && (S.Sim->GetPuck().LastTouch != S.Me || S.Sim->CarrierIndex() != S.Me)) Lost = F; ++F; };
		S.CursorRel = FVector2D::ZeroVector;
		Sweep(S, 3, 400, Look);
		S.CursorRel = S.Fwd() * 3.0;   // the hand rests at the end of its sweep; the drill measures 300 frames
		while (F < 300) { S.Step(); Look(F); }
		return { TEXT("carry"), Lost < 0, FString::Printf(TEXT("lost=%d lost_after=%d sweeps=%d puck_dist=%.2f"), Lost >= 0 ? 1 : 0, Lost, S.H().Sweeps - Sweeps0, FVector2D::Distance(S.H().Pos, S.Sim->GetPuck().Pos)) };
	}

	// Step 7: check (k=1), crosscheck (k=2), crosscheck_held (k=3).
	// bSeq: the drill's own sequence in one match (HokejAcceptDrill: Gap 30 frames, then the Stage frame; step 7 k>1 waits up to 300 frames for its man on his feet).
	void Gap(FStage& S, bool bWaitUp)
	{
		for (int32 f = 0; f < 31; ++f) S.Step();
		if (bWaitUp) for (int32 w = 0; w <= 300 && S.O().IsDown(); ++w) S.Step();
	}

	FRow Hit(FStage& S, int32 K, bool bSeq)
	{
		if (bSeq) Gap(S, K > 1);
		const FJamGameConfig& Cfg = S.Sim->GetConfig();
		S.bOppLive = true;
		const bool bStand = K >= 2 && Cfg.Contact.bMomentum;
		S.Sim->SetSkater(S.Opp, FVector2D(S.Dir * 2.0, 0.0), FVector2D::ZeroVector, S.FaceYaw + UE_DOUBLE_PI);
		S.Sim->ClearReleaseGrace(S.Opp);
		Stage(S, FVector2D(-S.Dir * (K == 3 ? 9.0 : 3.5), 0.0), S.Fwd() * (K == 3 ? 5.0 : 4.0));
		S.bPin = false;
		S.Sim->SetPuck(S.O().Pos + S.O().Forward() * (K == 2 && Cfg.Contact.bMomentum ? -0.45 : 0.45), FVector2D::ZeroVector);
		auto OppScript = [&](JamAI::FJamAIInput& I)
		{
			const FVector2D ToH = (S.H().Pos - S.O().Pos).GetSafeNormal() * (bStand ? 0.04 : 0.3);
			const FVector2D St = bStand ? -S.O().Forward() : S.O().Forward() * 0.5;
			I.Move = JamAI::FVec2(ToH.X, ToH.Y); I.Stick = JamAI::FVec2(St.X, St.Y);
		};
		const int32 WFrames = (K == 2 && Cfg.Contact.bMomentum) ? 2 : 90;
		S.HoldFor(EKeys::W, WFrames);
		if (K == 3) S.HoldFor(EKeys::LeftMouseButton, 132);
		int32 Cue = -1, Down = -1, Up = -1, KnockAt = 0, Outcome = 0, StickHit = 0, UpSeen = -1; double Ratio = 0.0, Closing = 0.0;
		bool bCheck = false, bStrip = false;
		for (int32 f = 0; f < 400; ++f)
		{
			S.Step(OppScript);
			const FJamSkaterState& H = S.H(); const FJamSkaterState& O = S.O();
			const FVector2D ToO = O.Pos - H.Pos; const double Dist = ToO.Size();
			const double Gap = Dist - UJamSim::SkaterRadius * 2.0;
			const double Cl = Dist > 1e-6 ? FVector2D::DotProduct(H.Vel - O.Vel, ToO / Dist) : 0.0;
			if (O.ContactOutcome > Outcome) { Outcome = O.ContactOutcome; Ratio = O.ContactRatio; }
			if (K == 3)
			{
				if (Cue < 0 && Gap <= 0.25) { Cue = f; Closing = Cl; }
				if (UpSeen < 0 && !O.IsDown()) UpSeen = f;
				if (Down < 0 && UpSeen >= 0 && O.IsDown()) { Down = f; KnockAt = O.KnockFrames; }
				if ((Down >= 0 && f - Down >= 10) || (Cue >= 0 && f - Cue > 60) || f > 300) break;
				continue;
			}
			const bool bCue = !Cfg.Contact.bMomentum ? (Gap <= 0.25 || (Cl > 0.5 && Gap <= Cl * (UJamSim::HumanCheckWindowFrames - 6) * UJamSim::DT))
				: K == 1 ? (Gap <= 0.25 || (Cl > 0.5 && Gap <= Cl * (UJamSim::HumanCheckWindowFrames - 2 + Cfg.Contact.CheckWindupFrames) * UJamSim::DT))
				: Gap <= Cfg.Contact.CrossReachM * 0.5 + FMath::Max(0.0, Cl) * Cfg.Contact.CrossWindupFrames * UJamSim::DT;
			if (Cue < 0 && bCue) { Cue = f; Closing = Cl; S.HoldFor(K == 1 ? EKeys::RightMouseButton : EKeys::MiddleMouseButton, 3); }
			for (const FJamSimEvent& E : S.Sim->GetFrameEvents()) if (E.Name == JamEvents::Check && E.Who == S.Me) bCheck = true;
			if (O.StickHitResult != 0) StickHit = O.StickHitResult;
			if (Down < 0 && O.IsDown()) { Down = f; KnockAt = O.KnockFrames; }
			if (Down >= 0 && Up < 0 && !O.IsDown()) Up = f;
			if (Down >= 0 && f - Down == 5) bStrip = S.Sim->CarrierIndex() != S.Opp;
			if (K == 2 && Cue >= 0 && f - Cue == 20) bStrip = S.Sim->CarrierIndex() != S.Opp;
			const bool bEnd = K == 1 ? (Up >= 0 || (Cue >= 0 && f - Cue > 60 && Down < 0) || f > 400) : ((Cue >= 0 && f - Cue > 40) || f > 300);
			if (bEnd) break;
		}
		if (K == 3)
			return { TEXT("crosscheck_held"), Down >= 0 && Cue >= 0, FString::Printf(TEXT("touched=%d down=%d knock_frames=%d outcome=%d ratio=%.2f closing=%.2f"), Cue >= 0 ? 1 : 0, Down >= 0 ? 1 : 0, KnockAt, Outcome, Ratio, Closing) };
		if (K == 1)
		{
			const int32 DownF = Up >= 0 ? Up - Down : -1;
			const double Scale = Cfg.Skater.KnockdownFrames / 90.0;
			const bool bRange = !Cfg.Contact.bMomentum ? KnockAt == Cfg.Skater.KnockdownFrames
				: (KnockAt >= FMath::FloorToInt32(Cfg.Contact.DownFramesMin * Scale) - 1 && KnockAt <= FMath::CeilToInt32(FMath::Max(Cfg.Contact.DownFramesMin, Cfg.Contact.DownFramesMax) * Scale) + 1);
			return { TEXT("check"), bCheck && Down >= 0 && bStrip && DownF >= 0 && FMath::Abs(DownF - KnockAt) <= 10 && bRange,
				FString::Printf(TEXT("cued=%d check=%d knockdown=%d strip=%d down_frames=%d knock_frames=%d outcome=%d ratio=%.2f closing=%.2f"), Cue >= 0 ? 1 : 0, bCheck ? 1 : 0, Down >= 0 ? 1 : 0, bStrip ? 1 : 0, DownF, KnockAt, Outcome, Ratio, Closing) };
		}
		return { TEXT("crosscheck"), (Cfg.Contact.bMomentum ? Outcome >= UJamSim::ContactStumble : StickHit != 0) && bStrip,
			FString::Printf(TEXT("cued=%d stickhit=%d puck_free=%d outcome=%d ratio=%.2f knockdown=%d closing=%.2f"), Cue >= 0 ? 1 : 0, StickHit, bStrip ? 1 : 0, Outcome, Ratio, Down >= 0 ? 1 : 0, Closing) };
	}

	// Step 8: the opponent sprints 7 m at the standing carrier and checks at contact (with its own wind-up); the human goes down for the frames the sim gave.
	// WSettle: W held from the stage for this many frames (the window of H35 shows the human 4 m forward of his stage spot 40 frames in).
	FRow Checked(FStage& S, bool bSeq, int32 WSettle = 0)
	{
		if (bSeq) Gap(S, false);   // the drill stages step 8 without waiting for the man step 7 k=3 felled
		const FJamGameConfig& Cfg = S.Sim->GetConfig();
		S.bOppLive = true;
		S.Sim->SetSkater(S.Opp, FVector2D(S.Dir * 7.0, 0.0), FVector2D::ZeroVector, S.FaceYaw + UE_DOUBLE_PI);
		Stage(S, FVector2D(0.0, 0.0), FVector2D::ZeroVector);
		const int32 OppDownAtStage = S.O().KnockFrames;
		if (WSettle > 0) S.HoldFor(EKeys::W, WSettle);
		auto Idle = [](JamAI::FJamAIInput&) {};
		FString Path = FString::Printf(TEXT(" stage: me=(%.2f,%.2f) opp=(%.2f,%.2f) carrier=%d keys=%d"), S.H().Pos.X, S.H().Pos.Y, S.O().Pos.X, S.O().Pos.Y, S.Sim->CarrierIndex(), S.Down.Num());
		for (int32 f = 0; f < 40; ++f) { S.Step(Idle); if (f == 0 || f == 20) Path += FString::Printf(TEXT(" s%d: me=(%.2f,%.2f) v=%.2f carrier=%d puck=(%.2f,%.2f)"), f, S.H().Pos.X, S.H().Pos.Y, S.H().Vel.Size(), S.Sim->CarrierIndex(), S.Sim->GetPuck().Pos.X, S.Sim->GetPuck().Pos.Y); }
		S.CursorRel = S.Fwd() * 3.0;
		S.HandTo(S.Me);
		Path += FString::Printf(TEXT(" hand: me=(%.2f,%.2f) v=%.2f"), S.H().Pos.X, S.H().Pos.Y, S.H().Vel.Size());
		for (int32 f = 0; f < 45; ++f) S.Step(Idle);
		Path += FString::Printf(TEXT(" act: me=(%.2f,%.2f) v=%.2f opp=(%.2f,%.2f) opp_down=%d carrier=%d puck_dist=%.2f"), S.H().Pos.X, S.H().Pos.Y, S.H().Vel.Size(), S.O().Pos.X, S.O().Pos.Y, S.O().KnockFrames, S.Sim->CarrierIndex(), FVector2D::Distance(S.H().Pos, S.Sim->GetPuck().Pos));
		int32 Down = -1, Up = -1, KnockAt = 0, Outcome = 0, Cue = -1; double Ratio = 0.0, Closing = 0.0;
		auto Charge = [&](JamAI::FJamAIInput& I)
		{
			const FJamSkaterState& H = S.H(); const FJamSkaterState& O = S.O();
			const FVector2D ToH = H.Pos - O.Pos; const double Dist = ToH.Size();
			const FVector2D Mv = ToH.GetSafeNormal();
			I.Move = JamAI::FVec2(Mv.X, Mv.Y); I.bSprint = true;
			const double Gap = Dist - UJamSim::SkaterRadius * 2.0;
			const double Cl = Dist > 1e-6 ? FVector2D::DotProduct(O.Vel - H.Vel, ToH / Dist) : 0.0;
			const int32 Wu = Cfg.Contact.CheckWindupFor(O.bHuman);
			if ((Gap <= 0.25 + Cl * Wu * UJamSim::DT || (Cl > 0.5 && Gap <= Cl * (UJamSim::CheckWindowFrames - 2 + Wu) * UJamSim::DT)) && !H.IsDown() && O.CanAct() && Down < 0)
			{
				I.bCheck = true;
				if (Cue < 0) { Cue = 1; Closing = Cl; }
			}
		};
		for (int32 f = 0; f < 600; ++f)
		{
			S.Step(Charge);
			const FJamSkaterState& H = S.H();
			if (H.ContactOutcome > Outcome) { Outcome = H.ContactOutcome; Ratio = H.ContactRatio; }
			if (Down < 0 && H.IsDown()) { Down = f; KnockAt = H.KnockFrames; }
			if (Down >= 0 && Up < 0 && !H.IsDown() && H.CanAct()) Up = f;
			if (Up >= 0 || (Down >= 0 && f - Down > FMath::Max(Cfg.Skater.KnockdownFrames, KnockAt) * 2)) break;
		}
		const int32 DownF = Up >= 0 ? Up - Down : -1;
		return { TEXT("checked"), Down >= 0 && DownF >= 0 && FMath::Abs(DownF - KnockAt) <= 15,
			FString::Printf(TEXT("down=%d down_frames=%d knock_frames=%d outcome=%d ratio=%.2f opp_checked=%d closing=%.2f opp_down_at_stage=%d (camera rows not replayed)%s"), Down >= 0 ? 1 : 0, DownF, KnockAt, Outcome, Ratio, Cue > 0 ? 1 : 0, Closing, OppDownAtStage, *Path) };
	}

	// Step 9: deke (wheel), spin (E), sweep (one 120 ms pass): fired and the puck kept 30 frames.
	FRow Move9(const UJamCartridge& C, int32 K)
	{
		FStage S(C);
		Stage(S, FVector2D(-S.Dir * 5.0, 0.0), FVector2D::ZeroVector);
		for (int32 f = 0; f < 40; ++f) S.Step();
		S.CursorRel = S.Fwd() * 3.0;
		S.HandTo(S.Me);
		for (int32 f = 0; f < 45; ++f) S.Step();
		int32 Fired = -1, Lost = -1, F = 0;
		auto Look = [&]()
		{
			const FJamSkaterState& H = S.H();
			const bool bFire = K == 1 ? H.bDekeThisFrame : (K == 2 ? H.bSpinThisFrame : H.bSweepThisFrame);
			if (bFire && Fired < 0) Fired = F;
			if (Fired >= 0 && Lost < 0 && F - Fired <= 30 && (S.Sim->GetPuck().LastTouch != S.Me || S.Sim->CarrierIndex() != S.Me)) Lost = F;
			++F;
		};
		if (K == 1) S.bWheel = true;
		if (K == 2) S.bSpinTap = true;
		if (K == 3) { S.CursorRel = FVector2D::ZeroVector; Sweep(S, 1, 120, [&](int32) { Look(); }); S.CursorRel = S.Fwd() * 3.0; }
		while (!((Fired >= 0 && F - Fired > 30) || (Fired < 0 && F > 120)))
		{
			S.Step();
			Look();
		}
		static const TCHAR* Verb[3] = { TEXT("deke"), TEXT("spin"), TEXT("sweep") };
		return { Verb[K - 1], Fired >= 0 && Lost < 0, FString::Printf(TEXT("fired=%d fired_after=%d kept_30=%d lost_after=%d"), Fired >= 0 ? 1 : 0, Fired, Fired >= 0 && Lost < 0 ? 1 : 0, Lost >= 0 ? Lost - Fired : -1) };
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHardAcceptRows, "Bar.Hard.AcceptRows", JamHard::LongFlags)
bool FJamHardAcceptRows::RunTest(const FString& Parameters)
{
	using namespace JamHardRows;
	FString CutPath = FPlatformMisc::GetEnvironmentVariable(TEXT("JAM_HARD_ACCEPT_CUT"));
	if (CutPath.IsEmpty()) CutPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), TEXT(".."), TEXT("tools"), TEXT("qa"), TEXT("accept"), TEXT("accept-cut.json")));
	TArray<FString> Errors;
	UJamCartridge* C = UJamCartridge::LoadFromFile(CutPath, Errors);
	if (!C) { AddError(FString::Printf(TEXT("the acceptance cut did not load: %s (%s)"), *CutPath, Errors.Num() ? *Errors[0] : TEXT("?"))); return false; }
	TArray<FRow> Rows;
	double Speed1 = 0.0, Speed2 = 0.0;
	for (int32 K = 1; K <= 4; ++K) Rows.Add(Skate(*C, K, Speed1, Speed2));
	{
		FString Facts; int32 Ok = 0;
		for (const double Rel : { 0.0, 90.0, 180.0, -90.0 }) Ok += AimOne(*C, Rel, Facts) ? 1 : 0;
		Rows.Add({ TEXT("aim"), Ok == 4, FString::Printf(TEXT("followed=%d/4"), Ok) + Facts });
	}
	Rows.Add(Carry(*C));
	// steps 7 and 8 as the drill plays them: one match, k=1, k=2, k=3, then 8 (the rows the release reads)
	{
		FStage S(*C);
		for (int32 K = 1; K <= 3; ++K) Rows.Add(Hit(S, K, true));
		Rows.Add(Checked(S, true));
	}
	// the same rows each on a fresh scene (the staging alone, without what the row before leaves behind)
	for (int32 K = 1; K <= 3; ++K) { FStage S(*C); FRow Row = Hit(S, K, false); Row.Verb += TEXT("_fresh"); Rows.Add(Row); }
	{ FStage S(*C); FRow Row = Checked(S, false); Row.Verb += TEXT("_fresh"); Rows.Add(Row); }
	// the window's step 8 (H35, 00:38 and 00:41): the drill's sequence with W still held 40 frames into the Settle
	{ FStage S(*C); for (int32 K = 1; K <= 3; ++K) Hit(S, K, true); FRow Row = Checked(S, true, 40); Row.Verb += TEXT("_w40"); Rows.Add(Row); }
	for (int32 K = 1; K <= 3; ++K) Rows.Add(Move9(*C, K));
	{
		FString Facts; int32 Ok = 0;
		for (const double Rel : { 90.0, -90.0 }) Ok += AimOne(*C, Rel, Facts) ? 1 : 0;
		Rows.Add({ TEXT("resume_aim"), Ok == 2, FString::Printf(TEXT("followed=%d/2 (the pause itself not replayed)"), Ok) + Facts });
	}
	int32 Red = 0;
	for (const FRow& Row : Rows)
	{
		UE_LOG(LogTemp, Display, TEXT("JAMHARD acceptrow verb=%s ok=%d %s"), *Row.Verb, Row.bOk ? 1 : 0, *Row.Facts);
		if (!Row.bOk) { ++Red; AddError(FString::Printf(TEXT("acceptance row %s would be red: %s"), *Row.Verb, *Row.Facts)); }
	}
	UE_LOG(LogTemp, Display, TEXT("JAMHARD acceptrows rows=%d red=%d cut=%s"), Rows.Num(), Red, *CutPath);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
