// Lane A229 (track U5, the HUD round): the HUD's one look. Jam.Hud.OneLook: a HUD built in a test world sets its standing text in two label sizes and one
// value size and nothing else; with jam.hud.OneLook 0 the HUD of before (a third size) comes back. Jam.Hud.EndStatsTable: the end card's rows.
#include "HUDWidget.h"
#include "Misc/AutomationTest.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/IConsoleManager.h"
#include "MenuWidget.h"
#include "Fonts/FontMeasure.h"
#include "Rendering/SlateRenderer.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamOneLookTests
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;
	// The type sizes of a HUD built in a test world under jam.hud.OneLook = On; empty without Slate.
	TArray<TPair<FString, int32>> SizesOf(int32 On)
	{
		TArray<TPair<FString, int32>> Out;
		if (!GEngine || !FSlateApplication::IsInitialized()) return Out;
		IConsoleVariable* Cv = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.hud.OneLook"));
		const int32 Was = Cv ? Cv->GetInt() : 1;
		if (Cv) Cv->Set(On, ECVF_SetByCode);
		UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, TEXT("JamOneLookTestWorld"));
		GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
		if (UJamHudWidget* Hud = CreateWidget<UJamHudWidget>(World, UJamHudWidget::StaticClass()))
		{
			Hud->AddToRoot();
			Hud->TakeWidget();
			Out = Hud->GetHudTypeSizesForTest();
			Hud->ReleaseSlateResources(true);
			Hud->RemoveFromRoot();
		}
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
		if (Cv) Cv->Set(Was, ECVF_SetByCode);
		return Out;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudOneLook, "Jam.Hud.OneLook", JamOneLookTests::Flags)
bool FJamHudOneLook::RunTest(const FString& Parameters)
{
	using W = UJamHudWidget;
	const IConsoleVariable* Cv = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.hud.OneLook"));
	TestTrue(TEXT("jam.hud.OneLook exists and is on by default"), Cv && Cv->GetInt() == 1);
	TestTrue(TEXT("two label sizes under one value size"), W::HudLabelPt < W::HudLinePt && W::HudLinePt < W::HudValuePt);
	const TArray<TPair<FString, int32>> After = JamOneLookTests::SizesOf(1);
	if (After.Num() == 0) { AddInfo(TEXT("no Slate: the built HUD was not measured")); return true; }
	TSet<int32> Seen;
	FString Line;
	for (const TPair<FString, int32>& P : After)
	{
		Seen.Add(P.Value);
		Line += FString::Printf(TEXT("%s=%d "), *P.Key, P.Value);
		TestTrue(FString::Printf(TEXT("%s is a label or a value size (%d)"), *P.Key, P.Value), P.Value == W::HudLabelPt || P.Value == W::HudLinePt || P.Value == W::HudValuePt);
	}
	AddInfo(FString::Printf(TEXT("A229 HUD type sizes: %s"), *Line));
	TestTrue(TEXT("fourteen standing texts measured"), After.Num() >= 14);
	TestTrue(TEXT("no third label size: three sizes at most"), Seen.Num() <= 3);
	const TArray<TPair<FString, int32>> Before = JamOneLookTests::SizesOf(0);
	TSet<int32> SeenBefore;
	for (const TPair<FString, int32>& P : Before) SeenBefore.Add(P.Value);
	TestTrue(TEXT("jam.hud.OneLook 0 gives back the HUD of before (more sizes)"), SeenBefore.Num() > 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudEndStatsTable, "Jam.Hud.EndStatsTable", JamOneLookTests::Flags)
bool FJamHudEndStatsTable::RunTest(const FString& Parameters)
{
	using W = UJamHudWidget;
	W::FEndStats S;
	S.Shots = 4; S.Goals = 1; S.Passes = 9; S.Arrived = 6; S.Hits = 3; S.CarryFrames = 60 * 42;
	const TArray<TPair<FString, FString>> Rows = W::EndStatsRows(S);
	TestEqual(TEXT("five rows"), Rows.Num(), 5);
	const TCHAR* Want[][2] = { { TEXT("SHOTS"), TEXT("4") }, { TEXT("GOALS"), TEXT("1") }, { TEXT("PASSES THAT ARRIVED"), TEXT("6 OF 9") }, { TEXT("HITS LANDED"), TEXT("3") }, { TEXT("WITH THE PUCK"), TEXT("0:42") } };
	for (int32 I = 0; I < 5 && I < Rows.Num(); ++I)
	{
		TestEqual(TEXT("label"), Rows[I].Key, FString(Want[I][0]));
		TestEqual(TEXT("value"), Rows[I].Value, FString(Want[I][1]));
	}
	// The same facts as the four lines of before (EndStatsText): the table says nothing the lines did not.
	const FString Lines = W::EndStatsText(S);
	for (const TPair<FString, FString>& R : Rows) TestTrue(FString::Printf(TEXT("%s is in the lines too"), *R.Key), Lines.Contains(R.Key) && Lines.Contains(R.Value));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudOneLookPlates, "Jam.Hud.OneLookPlates", JamOneLookTests::Flags)
bool FJamHudOneLookPlates::RunTest(const FString& Parameters)
{
	using W = UJamHudWidget;
	// Wave 2, point 2: the markers stand down under every card, and only under jam.hud.OneLook 1.
	TestFalse(TEXT("no card: the markers stay"), W::LabelsStandDown(true, false, false, false, false));
	TestTrue(TEXT("the MATCH card"), W::LabelsStandDown(true, true, false, false, false));
	TestTrue(TEXT("the GOAL banner"), W::LabelsStandDown(true, false, true, false, false));
	TestTrue(TEXT("the pause card"), W::LabelsStandDown(true, false, false, true, false));
	TestTrue(TEXT("the end card"), W::LabelsStandDown(true, false, false, false, true));
	TestFalse(TEXT("jam.hud.OneLook 0: the markers of before"), W::LabelsStandDown(false, true, true, true, true));
	// Wave 2, point 1: A227's two plates at the line size; IN THE BOX n inside A227's fixed box (300 x 44, padding 14 a side), the effect words measured.
	const TArray<TPair<FString, int32>> Sizes = JamOneLookTests::SizesOf(1);
	if (Sizes.Num() == 0 || !FSlateApplication::IsInitialized()) { AddInfo(TEXT("no Slate: the plates were not measured")); return true; }
	const TPair<FString, int32>* Box = Sizes.FindByPredicate([](const TPair<FString, int32>& P) { return P.Key == TEXT("box"); });
	TestTrue(TEXT("the box plate's words at the line size"), Box && Box->Value == W::HudLinePt);
	const TSharedRef<FSlateFontMeasure> M = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	const FSlateFontInfo Line = FJamMenuLook::Font(W::HudLinePt, FName("Bold"), FJamMenuLook::TrackLabel);
	const FVector2D BoxText = M->Measure(W::BoxPlateText(12 * 60), Line);
	AddInfo(FString::Printf(TEXT("A229 wave 2: '%s' %.0f x %.0f in %.0f x %.0f"), *W::BoxPlateText(12 * 60), BoxText.X, BoxText.Y, W::BoxPlateW - 28.f, W::BoxPlateH - 12.f));
	TestTrue(TEXT("IN THE BOX 12 fits the box's inside"), BoxText.X <= W::BoxPlateW - 28.f && BoxText.Y <= W::BoxPlateH - 12.f);
	const FString Slot = W::EffectSlotText(FName(TEXT("heavy_puck")), 600, true);
	const FVector2D SlotText = M->Measure(Slot, Line);
	AddInfo(FString::Printf(TEXT("A229 wave 2: effect slot '%s' %.0f x %.0f"), *Slot, SlotText.X, SlotText.Y));
	TestTrue(TEXT("the effect slot's words stand in one line of the line size"), SlotText.Y <= W::HudLinePt * 2.f && SlotText.X > 10.f);
	return true;
}

#endif
