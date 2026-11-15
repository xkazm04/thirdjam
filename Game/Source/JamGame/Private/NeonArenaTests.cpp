// Lane A197 (track M1, evolution 8): Jam.Rink.NeonArena. The grid arena (NeonArena.h) on the one mode that plays on a neon tier (Glow Ice, the file
// hockey.mode.glass-house): the rink built from that cartridge draws the grid, the lines and the frames and hides the painted marks; a puck on the ice gets a
// trail and lights the grid tile under it; a tier switch clears and redraws; `jam.neon 0` gives the tier back as it was; no other mode of the twelve plays on
// a neon tier. Headless, a transient game world of the test's own (as Jam.Rink.*).
// Lane A230 (track L2): four of the twelve play on the neon tier now (Glow Ice, Arsenal, the Cage, Marathon), each with its own accent from its
// look.trim_colour_hex (Glow Ice keeps the white of before), and the Cage's pillars are part of the dark sheet with rims of light.
//   UnrealEditor-Cmd.exe Game/ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Rink.NeonArena; Quit" -unattended -nullrhi -Multiprocess -nosplash -log
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "Misc/Paths.h"
#include "ProceduralMeshComponent.h"
#include "RinkActor.h"
#include "NeonArena.h"
#include "LookTierSwitcher.h"
#include "JamCartridge.h"
#include "JamPawns.h"

#if WITH_DEV_AUTOMATION_TESTS

// Lane A230 (wave 2): Jam.Rink.RigPerAxis. The neon rig is drawn for a 90 x 45 m rink and its edge line is one loop just inside the boards. A rink that
// is not 2 : 1 (the Cage, 60 x 28) streams it scaled length / 90 along and width / 45 across, so the loop is as far from the long boards as from the ends;
// a 2 : 1 rink, every other tier and `jam.look.RigPerAxis 0` keep the uniform scale of before.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamRinkRigPerAxisTest, "Jam.Rink.RigPerAxis",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FJamRinkRigPerAxisTest::RunTest(const FString& Parameters)
{
	const FString Neon(TEXT("l3_neon"));
	// The Cage: 60 / 90 along, 28 / 45 across.
	TestTrue(TEXT("the Cage along the rink: 0.667"), FMath::IsNearlyEqual(ULookTierSwitcher::RigScaleFor(60.f), 60.f / 90.f, 1e-4f));
	TestTrue(TEXT("the Cage across the rink: 0.622"), FMath::IsNearlyEqual(ULookTierSwitcher::RigCrossScaleFor(Neon, 60.f, 28.f), 28.f / 45.f, 1e-4f));
	// The loop sits Inset m inside the boards of the 90 x 45 rink: its distance from the long boards and from the ends of a 60 x 28 rink.
	const float Inset = 2.2f;
	auto GapLong = [&](float Cross) { return 14.f - (22.5f - Inset) * Cross; };
	const float GapEnd = 30.f - (45.f - Inset) * ULookTierSwitcher::RigScaleFor(60.f);
	TestTrue(TEXT("uniform: the loop is a metre nearer the long boards than the ends"), GapEnd - GapLong(ULookTierSwitcher::RigScaleFor(60.f)) > 0.9f);
	TestTrue(TEXT("per axis: within 0.15 m of the same distance"), FMath::Abs(GapEnd - GapLong(ULookTierSwitcher::RigCrossScaleFor(Neon, 60.f, 28.f))) < 0.15f);
	// Every 2 : 1 rink a mode plays: the uniform scale, exactly.
	const float TwoToOne[][2] = { { 56.f, 28.f }, { 60.f, 30.f }, { 70.f, 35.f }, { 90.f, 45.f }, { 120.f, 60.f }, { 140.f, 70.f } };
	for (const float* R : TwoToOne)
	{
		TestEqual(*FString::Printf(TEXT("%.0f x %.0f is uniform"), R[0], R[1]), ULookTierSwitcher::RigCrossScaleFor(Neon, R[0], R[1]), ULookTierSwitcher::RigScaleFor(R[0]));
	}
	// Another tier, a width not given, and the variable off: the uniform scale of before.
	TestEqual(TEXT("clay keeps the uniform scale at 60 x 28"), ULookTierSwitcher::RigCrossScaleFor(TEXT("p3_clay"), 60.f, 28.f), ULookTierSwitcher::RigScaleFor(60.f));
	TestEqual(TEXT("no width given: uniform"), ULookTierSwitcher::RigCrossScaleFor(Neon, 60.f, 0.f), ULookTierSwitcher::RigScaleFor(60.f));
	if (IConsoleVariable* Cv = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.look.RigPerAxis")))
	{
		Cv->Set(0, ECVF_SetByCode);
		TestEqual(TEXT("jam.look.RigPerAxis 0: uniform at 60 x 28"), ULookTierSwitcher::RigCrossScaleFor(Neon, 60.f, 28.f), ULookTierSwitcher::RigScaleFor(60.f));
		Cv->Set(1, ECVF_SetByCode);
	}
	else AddError(TEXT("jam.look.RigPerAxis does not exist"));
	// The Cage's cartridge is the one shipped mode this changes.
	TArray<FString> Errors;
	UJamCartridge* Cage = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(TEXT("modes/hockey.mode.cage")), Errors);
	if (TestNotNull(TEXT("the Cage loads"), Cage))
	{
		const float Along = ULookTierSwitcher::RigScaleFor(static_cast<float>(Cage->Game.Rink.LengthM));
		const float Cross = ULookTierSwitcher::RigCrossScaleFor(Cage->Game.Look.Tier, static_cast<float>(Cage->Game.Rink.LengthM), static_cast<float>(Cage->Game.Rink.WidthM));
		TestTrue(TEXT("the Cage's rig is narrower across than along"), Cross < Along - 0.03f);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamRinkNeonArenaTest, "Jam.Rink.NeonArena",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FJamRinkNeonArenaTest::RunTest(const FString& Parameters)
{
	// 1. The pure parts.
	TestEqual(TEXT("grid at the puck is at its brightest"), UJamNeonArena::GridEmissiveAt(0.0), UJamNeonArena::GridNearEmissive);
	TestEqual(TEXT("grid beyond the glow radius rests"), UJamNeonArena::GridEmissiveAt(UJamNeonArena::GridGlowRadiusM + 1.0), UJamNeonArena::GridRestEmissive);
	TestTrue(TEXT("grid brightens monotonically towards the puck"), UJamNeonArena::GridEmissiveAt(2.0) > UJamNeonArena::GridEmissiveAt(5.0));
	TestEqual(TEXT("goal pulse starts at 1"), UJamNeonArena::GoalPulseAt(0), 1.f);
	TestEqual(TEXT("goal pulse is over after its frames"), UJamNeonArena::GoalPulseAt(UJamNeonArena::GoalPulseFrames), 0.f);
	TestTrue(TEXT("l3_neon and B's n1_ tiers are neon"), UJamNeonArena::IsNeonTier(TEXT("l3_neon")) && UJamNeonArena::IsNeonTier(TEXT("n1_neon")));
	TestFalse(TEXT("clay is not neon"), UJamNeonArena::IsNeonTier(TEXT("p3_clay")));

	// 2. Of the twelve modes, one plays on a neon tier: Glow Ice (hockey.mode.glass-house).
	TArray<FString> Files;
	const FString Dir = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Cartridges"), TEXT("modes"));
	IFileManager::Get().FindFiles(Files, *FPaths::Combine(Dir, TEXT("*.cartridge.json")), true, false);
	// Lane A230: four since the look lane of Sunday evening: Arsenal, the Cage and Marathon joined Glow Ice; each has an accent of its own.
	TArray<FString> Neon;
	TArray<FLinearColor> Accents;
	Files.Sort();
	for (const FString& F : Files)
	{
		TArray<FString> Errors;
		UJamCartridge* C = UJamCartridge::LoadFromFile(FPaths::Combine(Dir, F), Errors);
		if (C && UJamNeonArena::IsNeonTier(C->Game.Look.Tier)) { Neon.Add(F); Accents.Add(UJamNeonArena::AccentFor(C->Game.Look)); }
	}
	TestEqual(TEXT("twelve mode files"), Files.Num(), 12);
	const TArray<FString> WantNeon = { TEXT("hockey.mode.arsenal.cartridge.json"), TEXT("hockey.mode.cage.cartridge.json"),
		TEXT("hockey.mode.glass-house.cartridge.json"), TEXT("hockey.mode.marathon.cartridge.json") };
	TestEqual(TEXT("four modes on a neon tier"), Neon.Num(), WantNeon.Num());
	TestTrue(TEXT("they are Arsenal, the Cage, Glow Ice and Marathon"), Neon == WantNeon);
	auto Apart = [](const FLinearColor& A, const FLinearColor& B) { return FMath::Abs(A.R - B.R) + FMath::Abs(A.G - B.G) + FMath::Abs(A.B - B.B); };
	for (int32 i = 0; i < Accents.Num(); ++i)
	{
		for (int32 j = i + 1; j < Accents.Num(); ++j)
		{
			TestTrue(*FString::Printf(TEXT("%s and %s have accents an eye tells apart"), *Neon[i], *Neon[j]), Apart(Accents[i], Accents[j]) > 0.5f);
		}
	}
	{
		// A cut that names no trim colour of its own (the schema's default) keeps the white of before the lane; `jam.neon.accent 0` gives every mode that white.
		FJamLookConfig Plain;
		UJamCartridge::HexToLinear(TEXT("#38BDF8"), Plain.TrimColour);
		FJamLookConfig Lime;
		UJamCartridge::HexToLinear(TEXT("#A3FF3D"), Lime.TrimColour);
		const FLinearColor WhiteOfBefore = UJamNeonArena::AccentFor(Plain);
		TestTrue(TEXT("the default trim colour draws the white of before"), WhiteOfBefore.Equals(FLinearColor(0.85f, 0.95f, 1.f), 0.001f));
		TestTrue(TEXT("a trim colour of its own is the accent"), UJamNeonArena::AccentFor(Lime).Equals(FLinearColor(Lime.TrimColour.R, Lime.TrimColour.G, Lime.TrimColour.B, 1.f), 0.001f));
		if (IConsoleVariable* AccentCv = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.neon.accent")))
		{
			AccentCv->Set(0, ECVF_SetByCode);
			TestTrue(TEXT("jam.neon.accent 0: white in every mode"), UJamNeonArena::AccentFor(Lime).Equals(WhiteOfBefore, 0.001f));
			AccentCv->Set(1, ECVF_SetByCode);
		}
		else AddError(TEXT("jam.neon.accent does not exist"));
	}

	TArray<FString> Errors;
	UJamCartridge* Glow = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(TEXT("modes/hockey.mode.glass-house")), Errors);
	for (const FString& E : Errors) AddError(E);
	if (!TestNotNull(TEXT("Glow Ice loads"), Glow)) return false;
	TestEqual(TEXT("the card's name"), Glow->Title, FString(TEXT("Glow Ice")));
	{
		FJamLookConfig Plain;
		UJamCartridge::HexToLinear(TEXT("#38BDF8"), Plain.TrimColour);
		TestTrue(TEXT("Glow Ice keeps its white centre"), UJamNeonArena::AccentFor(Glow->Game.Look).Equals(UJamNeonArena::AccentFor(Plain), 0.001f));
	}

	// A's square grid is off by default since B's rig brings its own (jam.neon.grid 0); the test draws it to check it.
	IConsoleVariable* GridCv = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.neon.grid"));
	const int32 GridWas = GridCv ? GridCv->GetInt() : 0;
	if (GridCv) GridCv->Set(1, ECVF_SetByCode);
	// 3. The rink of that cartridge draws the arena.
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, TEXT("A197NeonArena"));
	FWorldContext& Ctx = GEngine->CreateNewWorldContext(EWorldType::Game);
	Ctx.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	ARinkActor* Rink = World->SpawnActor<ARinkActor>();
	if (TestNotNull(TEXT("rink spawns"), Rink))
	{
		Rink->DispatchBeginPlay();
		Rink->Build(Glow->Game);
		UJamNeonArena* A = Rink->FindComponentByClass<UJamNeonArena>();
		if (TestNotNull(TEXT("the rink carries the arena"), A))
		{
			TestTrue(TEXT("active on Glow Ice"), A->IsArenaDrawn());
			TestTrue(TEXT("grid tiles drawn (90 x 45: 12 x 6)"), A->GetGridTileCount() == 72);
			TestFalse(TEXT("the painted marks are hidden"), Rink->Markings->IsVisible());
			TestFalse(TEXT("the ochre kick plate is hidden"), Rink->KickPlate->IsVisible());
			// Lane A223 (B-to-A-0046 point 2): Glow Ice whistles, so its two penalty boxes stand on this tier; their daylight slots (board, bench, kick plate:
			// three a box) take the dark sheet's material, and `jam.look.NeonBoxes 0` gives back the cream boxes of before.
			TestEqual(TEXT("the two penalty boxes are on the dark sheet (6 slots)"), A->GetBoxSlotsDarkened(), 6);
			if (IConsoleVariable* BoxCv = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.look.NeonBoxes")))
			{
				BoxCv->Set(0, ECVF_SetByCode);
				Rink->Build(Glow->Game);
				TestEqual(TEXT("jam.look.NeonBoxes 0: the boxes keep their daylight materials"), A->GetBoxSlotsDarkened(), 0);
				BoxCv->Set(1, ECVF_SetByCode);
				Rink->Build(Glow->Game);
				TestEqual(TEXT("jam.look.NeonBoxes 1 again: dark"), A->GetBoxSlotsDarkened(), 6);
			}
			else AddError(TEXT("jam.look.NeonBoxes does not exist"));

			// A puck at (10, 5) m: two frames of samples give it a trail and light the tile under it.
			AJamPuck* Puck = World->SpawnActor<AJamPuck>(FVector(1000, 500, 0), FRotator::ZeroRotator);
			if (TestNotNull(TEXT("puck spawns"), Puck))
			{
				A->TickComponent(1.f / 60.f, LEVELTICK_All, &A->PrimaryComponentTick);
				Puck->SetActorLocation(FVector(1060, 500, 0));
				A->TickComponent(1.f / 60.f, LEVELTICK_All, &A->PrimaryComponentTick);
				TestTrue(TEXT("the puck has a trail"), A->GetTrailCount() >= 1 && A->GetTrailVertexCount() > 0);
				float Brightest = 0.f, Dimmest = 1e9f;
				for (int32 i = 0; i < A->GetGridTileCount(); ++i) { Brightest = FMath::Max(Brightest, A->GetGridTileEmissive(i)); Dimmest = FMath::Min(Dimmest, A->GetGridTileEmissive(i)); }
				TestTrue(TEXT("the tile near the puck is brighter than the far ones"), Brightest > UJamNeonArena::GridRestEmissive * 3.f && Dimmest <= UJamNeonArena::GridRestEmissive + 0.01f);
				AddInfo(FString::Printf(TEXT("A197 neon: %d tiles, trail verts %d, grid emissive %.2f .. %.2f"), A->GetGridTileCount(), A->GetTrailVertexCount(), Dimmest, Brightest));
			}

			// A tier switch away and back without a rebuild.
			UJamNeonArena::OnTierChanged(Rink, TEXT("p3_clay"));
			TestFalse(TEXT("cleared on clay"), A->IsArenaDrawn());
			TestEqual(TEXT("off the grid arena the boxes have their own materials back"), A->GetBoxSlotsDarkened(), 0);
			TestTrue(TEXT("the marks are back on clay"), Rink->Markings->IsVisible());
			UJamNeonArena::OnTierChanged(Rink, TEXT("l3_neon"));
			TestTrue(TEXT("drawn again on l3_neon"), A->IsArenaDrawn() && A->GetGridTileCount() == 72);

			// Lane A230: the Cage's four pillars are built with the boards' daylight material and stood on the dark sheet as four pale drums. On the grid
			// arena they take the sheet's material and rims of light; `jam.neon.obstacles 0` gives back the drums of before; off the tier they are as built.
			TestEqual(TEXT("Glow Ice has no obstacle to darken"), A->GetObstaclesDarkened(), 0);
			TArray<FString> CageErrors;
			UJamCartridge* Cage = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(TEXT("modes/hockey.mode.cage")), CageErrors);
			for (const FString& E : CageErrors) AddError(E);
			if (TestNotNull(TEXT("the Cage loads"), Cage))
			{
				Rink->Build(Cage->Game);
				TestTrue(TEXT("the Cage draws the arena"), A->IsArenaDrawn());
				TestEqual(TEXT("the Cage's four pillars are on the dark sheet"), A->GetObstaclesDarkened(), 4);
				if (IConsoleVariable* ObCv = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.neon.obstacles")))
				{
					ObCv->Set(0, ECVF_SetByCode);
					Rink->Build(Cage->Game);
					TestEqual(TEXT("jam.neon.obstacles 0: the pillars keep the boards' material"), A->GetObstaclesDarkened(), 0);
					ObCv->Set(1, ECVF_SetByCode);
					Rink->Build(Cage->Game);
					TestEqual(TEXT("jam.neon.obstacles 1 again: dark"), A->GetObstaclesDarkened(), 4);
				}
				else AddError(TEXT("jam.neon.obstacles does not exist"));
				UJamNeonArena::OnTierChanged(Rink, TEXT("p3_clay"));
				TestEqual(TEXT("off the grid arena the pillars have their material back"), A->GetObstaclesDarkened(), 0);
				Rink->Build(Glow->Game);
				TestTrue(TEXT("Glow Ice drawn again after the Cage"), A->IsArenaDrawn() && A->GetObstaclesDarkened() == 0);
			}

			// jam.neon 0: the tier as before.
			IConsoleVariable* Cv = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.neon"));
			if (TestNotNull(TEXT("jam.neon exists"), Cv))
			{
				Cv->Set(0, ECVF_SetByCode);
				Rink->Build(Glow->Game);
				TestFalse(TEXT("jam.neon 0: no arena"), A->IsArenaDrawn());
				TestTrue(TEXT("jam.neon 0: the marks show"), Rink->Markings->IsVisible());
				Cv->Set(1, ECVF_SetByCode);
			}
		}
	}
	if (GridCv) GridCv->Set(GridWas, ECVF_SetByCode);
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
