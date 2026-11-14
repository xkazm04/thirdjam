// Lane A110 (track Q1, evolution 2): the online guest plays the whole game. Headless, no world:
//   Jam.Net.GuestSeat       the guest's seat gets the mode's ability and a pickup's, F works for it, the seat is in the checkpoint, two seated sims agree.
//   Jam.Net.CursorAimParity the lobby's local stick from a cursor at a known screen point equals single player's (A49's CursorAimRink), on the wire too.
//   Jam.Net.HashCoverage    every reflected field of the skater and the puck is either seen by LockstepStateHash() or listed here as a read-out with its
//                           reason; every member of the two structs is reflected (a layout check); UJamSim's unreflected state is pinned by size.
// Run: UnrealEditor-Cmd.exe Game/ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Net; Quit" -unattended -nullrhi -nosplash -log
#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"
#include "SceneView.h"
#include "JamCartridge.h"
#include "JamSim.h"
#include "JamNetPackets.h"
#include "HokejGameMode.h"
#include "LobbyGameMode.h"
#include "BarkSystem.h"
#include "CommentarySystem.h"
#include "JamCommentaryData.h"
#include "JamEventBus.h"
#include "HUDFeed.h"
#include "SignalSystem.h"
#include "JamBrain.h"
#include "JamSimAdapter.h"
#include "UObject/StrongObjectPtr.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamNetGuestTests
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;

	UJamCartridge* Load(FAutomationTestBase& T, const TCHAR* Name)
	{
		TArray<FString> Errors;
		UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(Name), Errors);
		for (const FString& E : Errors) T.AddError(E);
		return C;
	}

	// UJamLobbySubsystem::ComputeCartridgeHash's rule: team 1's first skater that is not the back.
	int32 GuestSlot(const UJamSim& Sim)
	{
		for (const FJamSkaterState& S : Sim.GetSkaters()) if (S.Team == 1 && S.Role != FName(TEXT("back"))) return S.Index;
		for (const FJamSkaterState& S : Sim.GetSkaters()) if (S.Team == 1) return S.Index;
		return -1;
	}

	// Busy inputs for every seat: cuts, sprints, stick sweeps, the verbs; F on the two human seats every 97 frames.
	void Busy(TArray<FJamInput>& In, int32 F, int32 Host, int32 Guest)
	{
		for (int32 i = 0; i < In.Num(); ++i)
		{
			FJamInput& I = In[i];
			I = FJamInput();
			const double Ang = 0.37 * i + (F / 40) * 2.6;
			I.Move = FVector2D(FMath::Cos(Ang), FMath::Sin(Ang));
			I.bSprint = (F / 90 + i) % 2 == 0;
			I.Stick = FVector2D(FMath::Cos(Ang * 1.7), FMath::Sin(Ang * 1.7)) * ((F / 25) % 2 ? 1.0 : 0.0);
			I.bPoke = (F + i) % 37 == 0; I.bLift = (F + i) % 53 == 0; I.bDeke = (F + i) % 61 == 0; I.bCheck = (F + i) % 29 == 0;
			I.bPassHeld = (F + 3 * i) % 70 < 30;
			I.bShootHeld = (F + 5 * i) % 90 < 20;
			I.bAbility = (i == Host || i == Guest) && (F + i) % 97 == 0;
		}
	}
}

using namespace JamNetGuestTests;

// ---------------------------------------------------------------------------------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNetGuestSeat, "Jam.Net.GuestSeat", JamNetGuestTests::Flags)
bool FJamNetGuestSeat::RunTest(const FString& Parameters)
{
	UJamCartridge* Shotgun = Load(*this, TEXT("modes/hockey.mode.shotgun"));
	UJamCartridge* Arsenal = Load(*this, TEXT("modes/hockey.mode.arsenal"));
	if (!Shotgun || !Arsenal) return false;

	// The mode's ability: before, only the cartridge's human held it; seated, the guest holds it from the drop.
	TStrongObjectPtr<UJamSim> Bot(NewObject<UJamSim>()), Seated(NewObject<UJamSim>());
	if (!TestTrue(TEXT("shotgun inits"), Bot->Init(Shotgun->Game, 11) && Seated->Init(Shotgun->Game, 11))) return false;
	const int32 Host = Seated->GetHumanIndex(), Guest = GuestSlot(*Seated);
	if (!TestTrue(TEXT("a guest seat exists"), Guest >= 0 && Guest != Host)) return false;
	TestEqual(TEXT("unseated, the guest's seat holds nothing (A104's finding)"), Bot->GetSkaters()[Guest].AbilitySlot, -1);
	TestEqual(TEXT("StateHash before the seat: the two sims are the same match"), Seated->StateHash(), Bot->StateHash());
	TestTrue(TEXT("SeatGuest(guest) at frame 0"), Seated->SeatGuest(Guest));
	TestTrue(TEXT("the guest's seat is a human now"), Seated->GetSkaters()[Guest].bHuman);
	TestEqual(TEXT("the host stays the sim's human index"), Seated->GetHumanIndex(), Host);
	TestEqual(TEXT("the guest holds the mode's ability (shotgun, slot 0)"), Seated->GetSkaters()[Guest].AbilitySlot, 0);
	TestEqual(TEXT("the host still holds it"), Seated->GetSkaters()[Host].AbilitySlot, 0);
	TestNotEqual(TEXT("the checkpoint sees the seat (LockstepStateHash)"), Seated->LockstepStateHash(), Bot->LockstepStateHash());
	TestFalse(TEXT("SeatGuest refuses the cartridge's own human"), Seated->SeatGuest(Host));
	TestFalse(TEXT("SeatGuest refuses a bad index"), Seated->SeatGuest(999));

	// F: the guest's presses are used (seated) and do nothing (unseated); same inputs to both.
	TArray<FJamInput> In;
	In.SetNum(Seated->GetSkaters().Num());
	for (int32 F = 0; F < 1800; ++F)
	{
		Busy(In, F, Host, Guest);
		Bot->Step(In);
		Seated->Step(In);
	}
	TestFalse(TEXT("SeatGuest refuses after frame 0"), Seated->SeatGuest(Guest));
	const int32 UsedSeated = Seated->GetSkaters()[Guest].AbilitiesUsed, UsedBot = Bot->GetSkaters()[Guest].AbilitiesUsed;
	AddInfo(FString::Printf(TEXT("guest abilities used over 1800 frames: seated %d, unseated %d; host %d"), UsedSeated, UsedBot, Seated->GetSkaters()[Host].AbilitiesUsed));
	TestTrue(TEXT("seated, the guest's F is used"), UsedSeated > 0);
	TestEqual(TEXT("unseated, the guest's F does nothing"), UsedBot, 0);

	// Two seated sims, the same inputs: equal lockstep hashes on every frame (the seat is deterministic, and a Restart keeps it).
	TStrongObjectPtr<UJamSim> A(NewObject<UJamSim>()), B(NewObject<UJamSim>());
	A->Init(Shotgun->Game, 5); B->Init(Shotgun->Game, 5);
	A->SeatGuest(Guest); B->SeatGuest(Guest);
	int32 Differ = -1;
	for (int32 F = 0; F < 1200 && Differ < 0; ++F)
	{
		Busy(In, F, Host, Guest);
		A->Step(In); B->Step(In);
		if (A->LockstepStateHash() != B->LockstepStateHash()) Differ = F;
	}
	TestEqual(TEXT("two seated sims agree on every frame"), Differ, -1);
	A->Restart();
	TestTrue(TEXT("Restart keeps the seat (the rematch)"), A->GetSkaters()[Guest].bHuman);
	TestEqual(TEXT("Restart grants the guest the mode's ability again"), A->GetSkaters()[Guest].AbilitySlot, 0);

	// A pickup's ability (hockey.mode.arsenal: four kinds, all trigger pickup): granted to the seated guest's take, not to an unseated one.
	TStrongObjectPtr<UJamSim> PBot(NewObject<UJamSim>()), PSeat(NewObject<UJamSim>());
	PBot->Init(Arsenal->Game, 3); PSeat->Init(Arsenal->Game, 3);
	const int32 AGuest = GuestSlot(*PSeat);
	PSeat->SeatGuest(AGuest);
	TestEqual(TEXT("arsenal: nothing held at the drop (pickup trigger only)"), PSeat->GetSkaters()[AGuest].AbilitySlot, -1);
	PBot->NotifyPickupTaken(AGuest, FName(TEXT("boost")));
	PSeat->NotifyPickupTaken(AGuest, FName(TEXT("boost")));
	TestEqual(TEXT("arsenal: an unseated guest's take grants nothing"), PBot->GetSkaters()[AGuest].AbilitySlot, -1);
	TestTrue(TEXT("arsenal: the seated guest's take grants a pickup ability"), PSeat->GetSkaters()[AGuest].AbilitySlot >= 0);
	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// The engine's own deprojection (FSceneView, as ULocalPlayer builds the view for APlayerController::DeprojectScreenPositionToWorld) for a third-person
// camera over the rink; a cursor put on the screen point of a known spot on the ice. Single player reads it through AHokejGameMode::CursorAimRink ->
// CursorAimFromRay and places the right stick first; the lobby now runs the same CursorAimRink and ComposeLocalStick. Headless has no viewport, so the
// mouse position itself is the one thing not driven here.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNetCursorAimParity, "Jam.Net.CursorAimParity", JamNetGuestTests::Flags)
bool FJamNetCursorAimParity::RunTest(const FString& Parameters)
{
	const FIntRect ViewRect(0, 0, 1920, 1080);
	const FVector CamLoc(-900.0, 150.0, 650.0);          // cm: 9 m behind, 6.5 m up
	const FRotator CamRot(-32.0, 4.0, 0.0);
	const FMatrix View = FTranslationMatrix(-CamLoc) * FInverseRotationMatrix(CamRot) * FMatrix(FPlane(0, 0, 1, 0), FPlane(1, 0, 0, 0), FPlane(0, 1, 0, 0), FPlane(0, 0, 0, 1));
	const FMatrix Proj = FReversedZPerspectiveMatrix(FMath::DegreesToRadians(90.0 * 0.5), ViewRect.Width(), ViewRect.Height(), 10.0);
	const FMatrix ViewProj = View * Proj;
	const FMatrix InvViewProj = ViewProj.Inverse();
	const FVector2D SkaterM(1.0, 0.5);
	const double CamYaw = FMath::DegreesToRadians(CamRot.Yaw);
	struct FCase { FVector2D IceM; FVector2D Pad; bool bAimingBefore; const TCHAR* Name; };
	const FCase Cases[] = {
		{ FVector2D(3.5, 1.4), FVector2D::ZeroVector, false, TEXT("2.7 m ahead-left: full blade") },
		{ FVector2D(1.9, -0.4), FVector2D::ZeroVector, false, TEXT("1.3 m ahead-right: part blade") },
		{ FVector2D(1.6, 0.5), FVector2D::ZeroVector, false, TEXT("0.6 m, not aiming yet: the dead zone (0.7 m to enter)") },
		{ FVector2D(1.6, 0.5), FVector2D::ZeroVector, true, TEXT("0.6 m, aiming: the hysteresis keeps it (0.5 m to leave)") },
		{ FVector2D(-2.0, 3.0), FVector2D(0.3, 0.8), false, TEXT("the right stick wins over the cursor") },
	};
	for (const FCase& C : Cases)
	{
		FVector2D Screen;
		if (!TestTrue(*FString::Printf(TEXT("%s: the spot is on screen"), C.Name), FSceneView::ProjectWorldToScreen(FVector(C.IceM.X * 100.0, C.IceM.Y * 100.0, 0.0), ViewRect, ViewProj, Screen))) continue;
		FVector O, D;
		FSceneView::DeprojectScreenToWorld(Screen, ViewRect, InvViewProj, O, D);
		// Single player (AHokejGameMode::ReadHumanInput): the right stick first, else the cursor.
		bool bAimSp = C.bAimingBefore;
		FVector2D Sp;
		const bool bCurSp = AHokejGameMode::CursorAimFromRay(O, D, SkaterM, bAimSp, Sp);
		const FVector2D Fwd(FMath::Cos(CamYaw), FMath::Sin(CamYaw)), Right(-FMath::Sin(CamYaw), FMath::Cos(CamYaw));
		const FVector2D SpStick = C.Pad.SizeSquared() >= 0.01 ? Fwd * C.Pad.Y + Right * C.Pad.X : Sp;
		// The lobby (ALobbyGameMode::ReadLocalInput): the same CursorAimRink (inherited, its own hysteresis state), then ComposeLocalStick. A mouse delta is
		// pending, as it always is while the mouse moves: it must not win over the cursor.
		bool bAimLobby = C.bAimingBefore;
		FVector2D Cur;
		const bool bCurLobby = C.Pad.SizeSquared() < 0.01 && AHokejGameMode::CursorAimFromRay(O, D, SkaterM, bAimLobby, Cur);
		const FVector2D Lobby = ALobbyGameMode::ComposeLocalStick(C.Pad, bCurLobby, Cur, FVector2D(140.0, -60.0), 1.0, CamYaw);
		TestTrue(*FString::Printf(TEXT("%s: single player reads the cursor"), C.Name), bCurSp);
		TestTrue(*FString::Printf(TEXT("%s: lobby stick == single player's (%.4f,%.4f) vs (%.4f,%.4f)"), C.Name, Lobby.X, Lobby.Y, SpStick.X, SpStick.Y), Lobby.Equals(SpStick, 1e-9));
		const FJamNetQuantInput QL = FJamNetQuantInput::From([&] { FJamInput I; I.Stick = Lobby; return I; }());
		const FJamNetQuantInput QS = FJamNetQuantInput::From([&] { FJamInput I; I.Stick = SpStick; return I; }());
		TestTrue(*FString::Printf(TEXT("%s: equal on the wire (quantised aim)"), C.Name), QL.StickX == QS.StickX && QL.StickY == QS.StickY);
		if (C.Pad.IsZero() && C.IceM != FVector2D(1.6, 0.5))
		{
			// The blade points at the cursor's spot on the ice: the ray's hit on z = 0, 0.5 + 0.5 * (dist - 0.5) / 2.5 long (A49). The engine's project /
			// deproject round trip lands within a few cm of the spot we put the cursor on (single precision in its screen maths); the stick is exact to the hit.
			const FVector Hit = O + D * (-O.Z / D.Z);
			const FVector2D HitM(Hit.X / 100.0, Hit.Y / 100.0);
			const FVector2D To = HitM - SkaterM;
			const FVector2D Want = To.GetSafeNormal() * (0.5 + 0.5 * FMath::Clamp((To.Size() - 0.5) / 2.5, 0.0, 1.0));
			AddInfo(FString::Printf(TEXT("%s: cursor spot (%.3f,%.3f) m, ray hit (%.3f,%.3f) m; stick (%.5f,%.5f), expected (%.5f,%.5f)"), C.Name, C.IceM.X, C.IceM.Y, HitM.X, HitM.Y, Lobby.X, Lobby.Y, Want.X, Want.Y));
			TestTrue(*FString::Printf(TEXT("%s: the ray hits the ice within 5 cm of the cursor's spot"), C.Name), FVector2D::Distance(HitM, C.IceM) < 0.05);
			TestTrue(*FString::Printf(TEXT("%s: the stick points at the ray's hit on the ice"), C.Name), Lobby.Equals(Want, 1e-9));
		}
	}
	// No cursor (hidden, or off the ice): the mouse delta, 100 px = one reach, as before.
	const FVector2D Delta = ALobbyGameMode::ComposeLocalStick(FVector2D::ZeroVector, false, FVector2D::ZeroVector, FVector2D(60.0, 0.0), 1.0, 0.0);
	TestTrue(TEXT("no cursor: the mouse delta (60 px right = 0.6 to the right)"), Delta.Equals(FVector2D(0.0, 0.6), 1e-9));
	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
namespace JamNetGuestTests
{
	// The read-outs: reflected fields LockstepStateHash() leaves out on purpose, each with why no rule reads it across frames (or why it is derived).
	// A lane that adds a field to FJamSkaterState or FJamPuckState either hashes it (StateHash behind its default, or LockstepStateHash) or lists it here.
	struct FReadout { const TCHAR* Name; const TCHAR* Why; };
	const FReadout SkaterReadouts[] = {
		{ TEXT("ChecksMissed"), TEXT("read-out counter behind the HUD's MISSED line (lane A113); read by no rule") },
		{ TEXT("ChecksLate"), TEXT("read-out counter: check presses the late-press rule resolved (lane A151); read by no rule") },
		{ TEXT("DownedPushFrames"), TEXT("read-out counter: frames the push-out around a downed skater turned this skater's course (lane A157); read by no rule") },
		{ TEXT("ChecksCooling"), TEXT("read-out counter: check presses the cooldown dropped (lane A157); read by no rule") },
		{ TEXT("LungesMissed"), TEXT("read-out counter: lunges that ran out on nobody (lane A157); read by no rule") },
		{ TEXT("ChecksGraced"), TEXT("read-out counter behind the HUD's JUST GOT UP line (lane A113); read by no rule") },
		{ TEXT("ChecksSoft"), TEXT("read-out counter behind the HUD's TOO SOFT line (lane A113); read by no rule") },
		{ TEXT("ContactGraced"), TEXT("read-out counter behind the HUD's contact line (lane A113); read by no rule") },
		{ TEXT("bContactGrace"), TEXT("read-out for the HUD: the grace turned this frame's hit into a bump (lane A113); read by no rule") },
		{ TEXT("LastContactBoardFrame"), TEXT("read-out: gates the board event and the figure's take against the echo of the same hit (lane A101); read by no rule") },
		{ TEXT("ContactBy"), TEXT("read-out for the figure and the HUD: who made the last contact (lane A101); set from hashed state, read by no rule") },
		{ TEXT("bContactBoard"), TEXT("read-out for the figure: the last landed check drove this skater into the boards (lane A101); read by no rule") },
		{ TEXT("ContactBoardHits"), TEXT("read-out counter for tests and the playtest line (lane A101); read by no rule") },
		{ TEXT("Index"), TEXT("identity: fixed by Init from the cartridge (the cartridge hash travels in HELLO)") },
		{ TEXT("Team"), TEXT("identity: fixed by Init") },
		{ TEXT("Role"), TEXT("identity: fixed by Init") },
		{ TEXT("SpawnPos"), TEXT("derived: the cartridge's spawns scaled by the rink revision, which is hashed") },
		{ TEXT("SpawnYaw"), TEXT("identity: fixed by Init") },
		{ TEXT("YawRate"), TEXT("read-out for the figure's lean (A104 audit)") },
		{ TEXT("PrevPos"), TEXT("render interpolation; overwritten at the start of each step") },
		{ TEXT("PrevYaw"), TEXT("render interpolation; overwritten at the start of each step") },
		{ TEXT("bBraking"), TEXT("A93 read-out for the stop pose") },
		{ TEXT("bReverseBraking"), TEXT("A93 read-out for the spray") },
		{ TEXT("bReverseBrakeLast"), TEXT("only counts the ReverseBrakes statistic") },
		{ TEXT("CarrySprintFrames"), TEXT("statistic") },
		{ TEXT("ReverseBrakes"), TEXT("statistic") },
		{ TEXT("KnockDir"), TEXT("the fall's direction for the figure (A104 audit)") },
		{ TEXT("Passes"), TEXT("statistic") },
		{ TEXT("LastShotCharge"), TEXT("one-frame read-out for PlayShoot") },
		{ TEXT("Sweeps"), TEXT("statistic") },
		{ TEXT("Spins"), TEXT("statistic") },
		{ TEXT("Saucers"), TEXT("statistic") },
		{ TEXT("bSaucerThisFrame"), TEXT("one-frame flag") },
		{ TEXT("PassesReceived"), TEXT("statistic") },
		{ TEXT("OneTimers"), TEXT("statistic") },
		{ TEXT("Breakaways"), TEXT("statistic") },
		{ TEXT("Dekes"), TEXT("statistic") },
		{ TEXT("bShotThisFrame"), TEXT("one-frame flag") },
		{ TEXT("bPassThisFrame"), TEXT("one-frame flag") },
		{ TEXT("bCheckThisFrame"), TEXT("one-frame flag") },
		{ TEXT("bPokeThisFrame"), TEXT("one-frame flag") },
		{ TEXT("bLiftThisFrame"), TEXT("one-frame flag") },
		{ TEXT("bOneTimerThisFrame"), TEXT("one-frame flag") },
		{ TEXT("bBreakawayThisFrame"), TEXT("one-frame flag") },
		{ TEXT("bDekeThisFrame"), TEXT("one-frame flag") },
		{ TEXT("bSweepThisFrame"), TEXT("one-frame flag") },
		{ TEXT("bSpinThisFrame"), TEXT("one-frame flag") },
		{ TEXT("PokeResult"), TEXT("one-frame result") },
		{ TEXT("DrawResult"), TEXT("one-frame result") },
		{ TEXT("bFoulThisFrame"), TEXT("one-frame flag") },
		{ TEXT("LastPassStrength"), TEXT("one-frame read-out") },
		{ TEXT("bAtFaceoff"), TEXT("one-frame level for the visual") },
		{ TEXT("bBumpThisFrame"), TEXT("A45 read-out") },
		{ TEXT("BumpDir"), TEXT("A45 read-out") },
		{ TEXT("LastBumpFrame"), TEXT("A45 read-out (A104 audit)") },
		{ TEXT("bBoardHitThisFrame"), TEXT("A45 read-out") },
		{ TEXT("BoardFrames"), TEXT("A45 read-out (A104 audit)") },
		{ TEXT("bBoardPinned"), TEXT("A45 read-out (A104 audit)") },
		{ TEXT("StickHitResult"), TEXT("one-frame result") },
		{ TEXT("ContactOutcome"), TEXT("A92 one-frame result") },
		{ TEXT("ContactRatio"), TEXT("A92 one-frame result") },
		{ TEXT("bContactCross"), TEXT("A92 one-frame result") },
		{ TEXT("bCheckRefused"), TEXT("A92 one-frame cue") },
		{ TEXT("bRecoilThisFrame"), TEXT("A92 one-frame flag") },
		{ TEXT("ContactBumps"), TEXT("statistic") },
		{ TEXT("ContactStumbles"), TEXT("statistic") },
		{ TEXT("ContactDowns"), TEXT("statistic") },
		{ TEXT("ContactRecoils"), TEXT("statistic") },
		{ TEXT("ChecksRefused"), TEXT("statistic") },
		{ TEXT("bTouchingPuck"), TEXT("recomputed every step before it is read") },
		{ TEXT("bHasPuck"), TEXT("recomputed every step before it is read") },
		{ TEXT("Goals"), TEXT("statistic (the score is hashed)") },
		{ TEXT("DistanceM"), TEXT("statistic") },
		{ TEXT("bAbilityThisFrame"), TEXT("one-frame flag") },
	};
	const FReadout PuckReadouts[] = {
		{ TEXT("PrevPos"), TEXT("render interpolation") },
		{ TEXT("PrevZ"), TEXT("render interpolation") },
		{ TEXT("PathM"), TEXT("statistic") },
	};
	// A field hashed only while another left its default (StateHash's "behind its default"): set the guard first, then perturb.
	struct FGuard { const TCHAR* Field; const TCHAR* Guard; int64 Value; };
	const FGuard SkaterGuards[] = {
		{ TEXT("SweepSide"), TEXT("SweepFrames"), 5 }, { TEXT("SpinLength"), TEXT("SpinFrames"), 5 }, { TEXT("SpinDir"), TEXT("SpinFrames"), 5 },
		{ TEXT("DekeSide"), TEXT("DekeFrames"), 5 }, { TEXT("AbilityCharges"), TEXT("AbilitySlot"), 0 }, { TEXT("AbilityReadyFrame"), TEXT("AbilitySlot"), 0 },
		{ TEXT("AbilityArmed"), TEXT("AbilitySlot"), 0 },
		// Lane A151 (track F1): the late press's memory is hashed while LateFrames runs.
		{ TEXT("LateWho"), TEXT("LateFrames"), 5 }, { TEXT("LateDir"), TEXT("LateFrames"), 5 }, { TEXT("LateVelSelf"), TEXT("LateFrames"), 5 }, { TEXT("LateVelOther"), TEXT("LateFrames"), 5 },
	};
	const FGuard PuckGuards[] = {
		{ TEXT("ShotgunBy"), TEXT("ShotgunFrames"), 5 }, { TEXT("ShotgunMul"), TEXT("ShotgunFrames"), 5 },
		{ TEXT("SlowBy"), TEXT("SlowFrames"), 5 }, { TEXT("SlowMul"), TEXT("SlowFrames"), 5 },
	};

	void SetInt(FProperty* P, void* Container, int64 V)
	{
		if (FNumericProperty* N = CastField<FNumericProperty>(P); N && N->IsInteger()) N->SetIntPropertyValue(N->ContainerPtrToValuePtr<void>(Container), V);
	}

	// Every perturbation of one element of one property that a rule could plausibly see; true if any moved Hash().
	bool Moves(FProperty* P, int32 Elem, void* Container, int64 Frame, TFunctionRef<uint64()> Hash)
	{
		void* V = P->ContainerPtrToValuePtr<void>(Container, Elem);
		const uint64 H0 = Hash();
		if (FBoolProperty* B = CastField<FBoolProperty>(P))
		{
			const bool Old = B->GetPropertyValue(V);
			B->SetPropertyValue(V, !Old);
			const bool bMoved = Hash() != H0;
			B->SetPropertyValue(V, Old);
			return bMoved;
		}
		if (FNumericProperty* N = CastField<FNumericProperty>(P))
		{
			if (N->IsInteger())
			{
				const int64 Old = N->GetSignedIntPropertyValue(V);
				const int64 Tries[] = { Old + 1, Old - 1, Old + 7, 0, -1, 1, 5, Frame, Frame - 1, Frame + 30 };
				bool bMoved = false;
				for (int64 T : Tries) { if (T == Old) continue; N->SetIntPropertyValue(V, T); bMoved |= Hash() != H0; N->SetIntPropertyValue(V, Old); }
				return bMoved;
			}
			const double Old = N->GetFloatingPointPropertyValue(V);
			const double Tries[] = { Old + 1.0, Old - 1.0, Old + 0.25, 0.0, 2.0 };
			bool bMoved = false;
			for (double T : Tries) { if (T == Old) continue; N->SetFloatingPointPropertyValue(V, T); bMoved |= Hash() != H0; N->SetFloatingPointPropertyValue(V, Old); }
			return bMoved;
		}
		if (FNameProperty* Nm = CastField<FNameProperty>(P))
		{
			const FName Old = Nm->GetPropertyValue(V);
			const FName Tries[] = { FName(TEXT("shotgun")), FName(TEXT("hip_check")), FName(TEXT("slow_puck")), FName(TEXT("shield")), NAME_None };
			bool bMoved = false;
			for (const FName& T : Tries) { if (T == Old) continue; Nm->SetPropertyValue(V, T); bMoved |= Hash() != H0; Nm->SetPropertyValue(V, Old); }
			return bMoved;
		}
		if (FStructProperty* S = CastField<FStructProperty>(P); S && S->Struct == TBaseStructure<FVector2D>::Get())
		{
			FVector2D& X = *static_cast<FVector2D*>(V);
			const FVector2D Old = X;
			const FVector2D Tries[] = { Old + FVector2D(1.0, 0.5), Old - FVector2D(0.5, 1.0), FVector2D(1.0, 0.0), FVector2D::ZeroVector };
			bool bMoved = false;
			for (const FVector2D& T : Tries) { if (T == Old) continue; X = T; bMoved |= Hash() != H0; X = Old; }
			return bMoved;
		}
		return false;   // a type this lint cannot perturb: reported as not covered, so a lane extends Moves() or lists it
	}

	// Each reflected field of Struct, over every element of Items: covered if a perturbation of any element moves Hash(), else it must be listed.
	template <typename TItem, int32 NR, int32 NG>
	void Cover(FAutomationTestBase& T, const TCHAR* What, UScriptStruct* Struct, TArray<TItem*> Items, const FReadout (&Readouts)[NR], const FGuard (&Guards)[NG],
		int64 Frame, TFunctionRef<uint64()> Hash, TArray<FString>& Untagged, int32& Covered, int32& Listed)
	{
		TSet<FString> ListedNames;
		for (const FReadout& R : Readouts) ListedNames.Add(R.Name);
		TSet<FString> Seen;
		for (TFieldIterator<FProperty> It(Struct); It; ++It)
		{
			FProperty* P = *It;
			const FString Name = P->GetName();
			Seen.Add(Name);
			if (ListedNames.Contains(Name)) { ++Listed; continue; }
			const FGuard* G = nullptr;
			for (const FGuard& Gd : Guards) if (Name == Gd.Field) G = &Gd;
			FProperty* GuardP = G ? Struct->FindPropertyByName(FName(G->Guard)) : nullptr;
			bool bMoved = false;
			for (TItem* Item : Items)
			{
				const TItem Backup = *Item;
				if (GuardP) SetInt(GuardP, Item, G->Value);
				for (int32 e = 0; e < P->ArrayDim && !bMoved; ++e) bMoved = Moves(P, e, Item, Frame, Hash);
				*Item = Backup;
				if (bMoved) break;
			}
			if (bMoved) ++Covered;
			else Untagged.Add(FString::Printf(TEXT("%s.%s"), What, *Name));
		}
		for (const FReadout& R : Readouts)
		{
			if (!Seen.Contains(R.Name)) T.AddError(FString::Printf(TEXT("%s: the read-out list names \"%s\", which is no field any more: take it off the list"), What, R.Name));
		}
	}

	// Every byte of the struct belongs to a reflected property or to the alignment padding before the next one: an unreflected member (which reflection,
	// the soak's field compare and this lint cannot see) leaves a gap. Members that fit in padding the alignment would leave anyway (a bool after a bool)
	// are not seen; the convention in JamSim.h is that every member of these two structs is a UPROPERTY.
	void Layout(FAutomationTestBase& T, const TCHAR* What, UScriptStruct* Struct, int32 SizeOf)
	{
		struct FSpan { int32 Off, Size, Align; FString Name; };
		TArray<FSpan> Spans;
		for (TFieldIterator<FProperty> It(Struct); It; ++It) Spans.Add({ It->GetOffset_ForInternal(), It->GetSize(), FMath::Max(1, It->GetMinAlignment()), It->GetName() });
		Spans.Sort([](const FSpan& A, const FSpan& B) { return A.Off < B.Off; });
		int32 Cursor = 0;
		FString Prev = TEXT("(start)");
		for (const FSpan& S : Spans)
		{
			const int32 Expected = Align(Cursor, S.Align);
			if (S.Off > Expected)
				T.AddError(FString::Printf(TEXT("%s: %d bytes between %s and %s belong to no UPROPERTY: an unreflected member? Make it a UPROPERTY (then hash it or list it)"), What, S.Off - Cursor, *Prev, *S.Name));
			Cursor = FMath::Max(Cursor, S.Off + S.Size);
			Prev = S.Name;
		}
		const int32 Tail = Align(Cursor, FMath::Max(1, Struct->GetMinAlignment()));
		if (SizeOf > Tail) T.AddError(FString::Printf(TEXT("%s: %d bytes after %s belong to no UPROPERTY: an unreflected member at the end?"), What, SizeOf - Cursor, *Prev));
	}

	// UJamSim's own state is not reflected (its scalars: frame, phase, score, the faceoff, the deke commit, the draw, the rule flags). Its size is pinned:
	// a member added or removed changes it. Then: hash it (StateHash behind its default, or LockstepStateHash) or say why not here, and move the pin.
	constexpr int32 SimBytesPinned = 1768;   // lane A110, UE 5.9 Win64 editor Development (measured by this test)
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNetHashCoverage, "Jam.Net.HashCoverage", JamNetGuestTests::Flags)
bool FJamNetHashCoverage::RunTest(const FString& Parameters)
{
	// A live shotgun match with both human seats holding the ability, the burst ring live, holds and cuts: most "behind its default" state is on.
	UJamCartridge* C = Load(*this, TEXT("modes/hockey.mode.shotgun"));
	if (!C) return false;
	TStrongObjectPtr<UJamSim> Sim(NewObject<UJamSim>());
	if (!TestTrue(TEXT("sim inits"), Sim->Init(C->Game, 9))) return false;
	const int32 Host = Sim->GetHumanIndex(), Guest = GuestSlot(*Sim);
	Sim->SeatGuest(Guest);
	TArray<FJamInput> In;
	In.SetNum(Sim->GetSkaters().Num());
	for (int32 F = 0; F < 420; ++F) { Busy(In, F, Host, Guest); Sim->Step(In); }
	TestTrue(TEXT("the shotgun cut runs the burst (MoveHist is hashed)"), C->Game.Skater.BurstFrames > 0);
	auto Hash = [&] { return Sim->LockstepStateHash(); };
	const uint64 Before = Hash();
	TArray<FString> Untagged;
	int32 Covered = 0, Listed = 0;
	TArray<FJamSkaterState*> Skaters;
	for (const FJamSkaterState& S : Sim->GetSkaters()) Skaters.Add(const_cast<FJamSkaterState*>(&S));   // test only: one field at a time, put back
	Cover<FJamSkaterState>(*this, TEXT("skater"), FJamSkaterState::StaticStruct(), Skaters, SkaterReadouts, SkaterGuards, Sim->GetFrame(), Hash, Untagged, Covered, Listed);
	TArray<FJamPuckState*> Pucks = { const_cast<FJamPuckState*>(&Sim->GetPuck()) };
	Cover<FJamPuckState>(*this, TEXT("puck"), FJamPuckState::StaticStruct(), Pucks, PuckReadouts, PuckGuards, Sim->GetFrame(), Hash, Untagged, Covered, Listed);
	AddInfo(FString::Printf(TEXT("hash coverage: %d fields seen by LockstepStateHash, %d listed read-outs, %d untagged"), Covered, Listed, Untagged.Num()));
	for (const FString& U : Untagged)
		AddError(FString::Printf(TEXT("%s is neither seen by LockstepStateHash() nor listed as a read-out (JamNetGuestTests::SkaterReadouts / PuckReadouts): hash it or list it with its reason"), *U));
	TestEqual(TEXT("untagged fields"), Untagged.Num(), 0);
	TestEqual(TEXT("the sim is unchanged by the probes"), Hash(), Before);

	Layout(*this, TEXT("FJamSkaterState"), FJamSkaterState::StaticStruct(), sizeof(FJamSkaterState));
	Layout(*this, TEXT("FJamPuckState"), FJamPuckState::StaticStruct(), sizeof(FJamPuckState));
	if (static_cast<int32>(sizeof(UJamSim)) != SimBytesPinned)
		AddWarning(FString::Printf(TEXT("sizeof(UJamSim) is %d, pinned %d: a member was added or removed. Hash it (StateHash behind its default, or LockstepStateHash) or note why not, then set SimBytesPinned"), static_cast<int32>(sizeof(UJamSim)), SimBytesPinned));
	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// Lane A120 (track Q1): barks and commentary speak to this machine's seat. ALobbyGameMode::GetLocalSeat (LocalSeatOf) is the guest's seat on the guest's
// machine; UJamBarkSystem::BuildWorld and UJamCommentarySystem::BuildWorld read it (they read the sim's human, the host's, before). Presentation only: the
// worlds are built from a seated shotgun sim and the lockstep hash is the same before and after.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNetGuestBarks, "Jam.Net.GuestBarks", JamNetGuestTests::Flags)
bool FJamNetGuestBarks::RunTest(const FString& Parameters)
{
	UJamCartridge* Shotgun = Load(*this, TEXT("modes/hockey.mode.shotgun"));
	if (!Shotgun) return false;
	TStrongObjectPtr<UJamSim> Sim(NewObject<UJamSim>());
	if (!TestTrue(TEXT("shotgun inits"), Sim->Init(Shotgun->Game, 7))) return false;
	const int32 Host = Sim->GetHumanIndex(), Guest = GuestSlot(*Sim);
	if (!TestTrue(TEXT("a guest seat"), Guest >= 0 && Guest != Host && Sim->SeatGuest(Guest))) return false;
	TArray<FJamInput> In; In.SetNum(Sim->GetSkaters().Num());
	for (int32 f = 0; f < 400; ++f) { Busy(In, f, Host, Guest); Sim->Step(In); }
	const uint64 Before = JamNetLockstepHash(Sim.Get(), nullptr, nullptr, nullptr);

	const int32 GuestMachine = ALobbyGameMode::LocalSeatOf(false, Host, Guest), HostMachine = ALobbyGameMode::LocalSeatOf(true, Host, Guest);
	TestEqual(TEXT("the guest's machine speaks to the guest's seat"), GuestMachine, Guest);
	TestEqual(TEXT("the host's machine speaks to the host's seat"), HostMachine, Host);
	FJamBarkWorld Wg, Wh;
	UJamBarkSystem::FillWorld(*Sim, GuestMachine, Wg);
	UJamBarkSystem::FillWorld(*Sim, HostMachine, Wh);
	const int32 GuestTeam = Sim->GetSkaters()[Guest].Team;
	// The bark picker reads a goal at x > 0 as team 0's (its own convention): the guest's team scores at the other end.
	const FVector2D GoalAt(GuestTeam == 0 ? 40.0 : -40.0, 0.0);
	FJamEventPayload P; P.Who = Guest; P.Where = FVector(GoalAt.X * 100, GoalAt.Y * 100, 0);
	FJamBarkResolved R;
	// Goal: the guest's own teammate praises it on the guest's machine (the human's bark); on the host's machine the same goal is goal_against.
	TestTrue(TEXT("guest's goal resolves on the guest's machine"), FJamBarkPicker::Resolve(JamEvents::Goal, P, GoalAt, Wg, R));
	TestTrue(FString::Printf(TEXT("guest's goal -> goal_for, a teammate of the guest speaks (situation %s, speaker %d)"), *R.Situation.ToString(), R.Speaker),
		R.Situation == JamBarks::GoalFor && R.Side == EJamBarkSpeaker::Teammate && Wg.IsTeammate(R.Speaker) && Sim->GetSkaters()[R.Speaker].Team == GuestTeam);
	TestTrue(TEXT("the same goal on the host's machine"), FJamBarkPicker::Resolve(JamEvents::Goal, P, GoalAt, Wh, R) && R.Situation == JamBarks::GoalAgainst);
	// Hit: the guest's check lands a check_landed bark on the guest's machine; on the host's it is a bot's check (says nothing), as before for the guest.
	FJamEventPayload Ck = P; Ck.Where = FVector::ZeroVector;
	TestTrue(TEXT("guest's check -> check_landed on the guest's machine"), FJamBarkPicker::Resolve(JamEvents::Check, Ck, FVector2D::ZeroVector, Wg, R) && R.Situation == JamBarks::CheckLanded && Wg.IsOpponent(R.Speaker));
	TestFalse(TEXT("guest's check on the host's machine: not the local human's"), FJamBarkPicker::Resolve(JamEvents::Check, Ck, FVector2D::ZeroVector, Wh, R));
	FJamEventPayload Kd = P; Kd.Kind = TEXT("check");
	TestTrue(TEXT("guest knocked down -> check_received on the guest's machine"), FJamBarkPicker::Resolve(JamEvents::Knockdown, Kd, FVector2D::ZeroVector, Wg, R) && R.Situation == JamBarks::CheckReceived);

	// Ability: the commentary's "ability.human" line holds for the guest's F on the guest's machine only.
	FJamCommentaryWorld Cg, Ch;
	for (const FJamSkaterState& S : Sim->GetSkaters()) { Cg.Teams.Add(S.Team); Cg.Positions.Add(S.Pos); Cg.DrawEarly.Add(false); }
	Ch = Cg;
	Cg.HumanIndex = GuestMachine; Ch.HumanIndex = HostMachine;
	FJamCommentaryLine L; L.Id = TEXT("ability.human.01"); L.Situation = TEXT("ability.human"); L.Event = JamEvents::Ability; L.Team = JamCommentary::TeamAny;
	FJamCommentaryEvent E; E.Event = JamEvents::Ability; E.Who = Guest; E.Team = GuestTeam; E.Strength = 1.f;
	FJamCommentaryPicker Picker(FJamCommentaryBank::Embedded(), 3);
	TestTrue(TEXT("guest's F -> ability.human on the guest's machine"), Picker.WhenHolds(L, E, Cg));
	TestFalse(TEXT("guest's F is not the local human's on the host's machine"), Picker.WhenHolds(L, E, Ch));
	int32 HumanAbilityLines = 0;
	for (const FJamCommentaryLine& Bl : FJamCommentaryBank::Embedded().Lines) if (Bl.Event == JamEvents::Ability && FJamCommentaryPicker::Suffix(Bl) == FName(TEXT("human"))) ++HumanAbilityLines;
	AddInfo(FString::Printf(TEXT("embedded bank: %d ability.human lines; guest %d (team %d), host %d"), HumanAbilityLines, Guest, GuestTeam, Host));

	TestEqual(TEXT("presentation only: the lockstep hash is unchanged"), JamNetLockstepHash(Sim.Get(), nullptr, nullptr, nullptr), Before);
	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// Lane A129 (track Q1, evolution 4): the remaining sites that spoke to the host's seat on the guest's machine, the brain's second human, the lockstep
// rematch and the refused one-machine change. Headless, no world: each site's seat-reading function is called with the guest's machine's seat.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNetGuestSites, "Jam.Net.GuestSites", JamNetGuestTests::Flags)
bool FJamNetGuestSites::RunTest(const FString& Parameters)
{
	using namespace JamNetGuestTests;
	UJamCartridge* Shotgun = Load(*this, TEXT("modes/hockey.mode.shotgun"));
	if (!Shotgun) return false;
	TStrongObjectPtr<UJamSim> Sim(NewObject<UJamSim>());
	if (!TestTrue(TEXT("shotgun inits"), Sim->Init(Shotgun->Game, 7))) return false;
	const int32 Host = Sim->GetHumanIndex(), Guest = GuestSlot(*Sim);
	// Offline first: the brain's world has no second human.
	{
		JamAI::FJamAIWorld W0;
		JamAI::FJamSimAdapter::BuildWorld(*Sim, W0);
		TestEqual(TEXT("single player: no guest in the brain's world"), W0.GuestIndex, -1);
	}
	if (!TestTrue(TEXT("a guest seat"), Guest >= 0 && Guest != Host && Sim->SeatGuest(Guest))) return false;
	TArray<FJamInput> In; In.SetNum(Sim->GetSkaters().Num());
	for (int32 f = 0; f < 400; ++f) { Busy(In, f, Host, Guest); Sim->Step(In); }
	const int32 GuestMachine = ALobbyGameMode::LocalSeatOf(false, Host, Guest);
	const int32 GuestTeam = Sim->GetSkaters()[Guest].Team, HostTeam = Sim->GetSkaters()[Host].Team;

	// 1. The HUD feed (HUDFeed.cpp BuildFrameFromSim): the guest's machine's frame is the guest's seat, its attack side and its team's score first.
	FJamPresentationFrame Fg, Fh;
	UJamPresentation::BuildFrameFromSim(*Sim, Fg, GuestMachine);
	UJamPresentation::BuildFrameFromSim(*Sim, Fh);
	TestEqual(TEXT("HUD: the guest's machine's human is the guest"), Fg.HumanIndex, Guest);
	TestEqual(TEXT("HUD: single player's is the sim's human"), Fh.HumanIndex, Host);
	TestEqual(TEXT("HUD: the guest attacks its own team's net"), Fg.AttackSign, Sim->AttackGoalX(GuestTeam) >= 0.0 ? 1 : -1);
	if (GuestTeam != HostTeam) TestNotEqual(TEXT("HUD: the guest's attack side is not the host's"), Fg.AttackSign, Fh.AttackSign);

	// 2. The signals (SignalSystem.cpp Issue -> AnswererFor): a bot teammate of the guest answers the guest's signal, never a human seat.
	const int32 Ans = UJamSignalSystem::AnswererFor(*Sim, GuestMachine);
	TestTrue(FString::Printf(TEXT("signals: the guest's answerer %d is a bot of the guest's team"), Ans), Ans >= 0 && Sim->GetSkaters()[Ans].Team == GuestTeam && !Sim->GetSkaters()[Ans].bHuman);
	const int32 AnsH = UJamSignalSystem::AnswererFor(*Sim, Host);
	TestTrue(TEXT("signals: the host's answerer is never the guest"), AnsH >= 0 && AnsH != Guest && Sim->GetSkaters()[AnsH].Team == HostTeam);

	// 3 and 4. The skate loop (CommentarySystem.cpp UpdateLoops) and GetHumanSkater read AHokejGameMode::GetLocalSeat: ALobbyGameMode's is LocalSeatOf.
	TestEqual(TEXT("skate loop / GetHumanSkater: the guest's machine's seat"), GuestMachine, Guest);

	// 5. The brain (LobbyGameMode GatherBotInputs): the seated guest is its second human: no think, no job for it; offline nothing changes (above).
	JamAI::FJamAIWorld W;
	JamAI::FJamSimAdapter::BuildWorld(*Sim, W);
	TestEqual(TEXT("brain: the guest is the world's second human"), W.GuestIndex, Guest);
	TestTrue(TEXT("brain: both seats are human seats"), W.IsHumanSeat(Host) && W.IsHumanSeat(Guest));
	TStrongObjectPtr<UJamBrain> Brain(NewObject<UJamBrain>());
	Brain->ApplyCartridge(*Shotgun);
	Brain->ResetBrain();
	JamAI::FJamAIInput Out[JamAI::kMaxSkaters];
	const JamAI::FVec2 Sentinel(9.0, 9.0);
	for (int32 i = 0; i < JamAI::kMaxSkaters; ++i) Out[i].Move = Sentinel;
	for (int32 t = 0; t < 30; ++t) Brain->Think(W, Out);
	TestTrue(TEXT("brain: the guest's slot is left to the guest (untouched)"), Out[Guest].Move.X == 9.0 && Out[Guest].Move.Y == 9.0);
	int32 Bots = 0;
	for (int32 i = 0; i < W.NumSkaters; ++i) if (!W.IsHumanSeat(i) && !(Out[i].Move.X == 9.0 && Out[i].Move.Y == 9.0)) ++Bots;
	TestEqual(TEXT("brain: every bot thought"), Bots, W.NumSkaters - 2);

	// 6. The rematch: only the frame both seats hold the ask while the sim is Over; a restarted sim forgets its history (both machines restart equal).
	FJamInput Ask; JamNet::SetRematchReady(Ask);
	FJamInput Brake; Brake.bBrake = true; Brake.bShootHeld = true;
	TestTrue(TEXT("rematch: both ask on the card"), ALobbyGameMode::RematchAgreed(EJamSimPhase::Over, Ask, Ask));
	TestFalse(TEXT("rematch: one asks"), ALobbyGameMode::RematchAgreed(EJamSimPhase::Over, Ask, FJamInput()));
	TestFalse(TEXT("rematch: brake + charge is play, not an ask"), ALobbyGameMode::RematchAgreed(EJamSimPhase::Over, Ask, Brake));
	TestFalse(TEXT("rematch: never in play"), ALobbyGameMode::RematchAgreed(EJamSimPhase::Playing, Ask, Ask));
	TStrongObjectPtr<UJamSim> Other(NewObject<UJamSim>());
	Other->Init(Shotgun->Game, 7);
	Other->SeatGuest(Guest);
	for (int32 f = 0; f < 650; ++f) { Busy(In, f * 3 + 1, Host, Guest); Other->Step(In); }
	Sim->Restart();
	Other->Restart();
	// Both machines restart from the same history (lockstep), so the rematch is deterministic across them; whether Restart forgets the history is a sim
	// property (outside the family): the fields it keeps are listed as information for the sim's owner.
	TStrongObjectPtr<UJamSim> Twin(NewObject<UJamSim>());
	Twin->Init(Shotgun->Game, 7);
	Twin->SeatGuest(Guest);
	for (int32 f = 0; f < 650; ++f) { Busy(In, f * 3 + 1, Host, Guest); Twin->Step(In); }
	Twin->Restart();
	TestEqual(TEXT("rematch: two machines with the same history restart to the same lockstep hash"), Twin->LockstepStateHash(), Other->LockstepStateHash());
	// Lane A140: Restart forgets the history (the fields lane A129 listed here are reset), so even two different histories restart equal.
	TestEqual(TEXT("rematch: two different histories restart to the same lockstep hash"), Sim->LockstepStateHash(), Other->LockstepStateHash());
	if (Sim->LockstepStateHash() != Other->LockstepStateHash())
	{
		FString Kept;
		for (int32 i = 0; i < Sim->GetSkaters().Num(); ++i)
			for (TFieldIterator<FProperty> It(FJamSkaterState::StaticStruct()); It; ++It)
				if (!It->Identical_InContainer(&Sim->GetSkaters()[i], &Other->GetSkaters()[i])) Kept += FString::Printf(TEXT(" skater[%d].%s"), i, *It->GetName());
		for (TFieldIterator<FProperty> It(FJamPuckState::StaticStruct()); It; ++It)
			if (!It->Identical_InContainer(&Sim->GetPuck(), &Other->GetPuck())) Kept += FString::Printf(TEXT(" puck.%s"), *It->GetName());
		AddInfo(FString::Printf(TEXT("finding (sim): UJamSim::Restart keeps history; after different histories the restarted sims differ in:%s"), Kept.IsEmpty() ? TEXT(" unreflected state only") : *Kept));
	}
	TestTrue(TEXT("rematch: the guest stays seated"), Sim->GetSkaters()[Guest].bHuman && Sim->HeldAbility(Guest) != nullptr);

	// 7. A live cut / cut load / restart is refused in a lockstep match with a line for the screen; single player is untouched.
	FString Line;
	TestTrue(TEXT("refusal: online"), ALobbyGameMode::LiveChangeRefused(true, TEXT("a live cut"), Line) && Line.Contains(TEXT("online match")));
	TestFalse(TEXT("refusal: single player"), ALobbyGameMode::LiveChangeRefused(false, TEXT("a live cut"), Line));
	AddInfo(FString::Printf(TEXT("host %d (team %d), guest %d (team %d), guest's answerer %d, host's %d; refusal line: %s"), Host, HostTeam, Guest, GuestTeam, Ans, AnsH, *Line));
	return true;
}

#endif
