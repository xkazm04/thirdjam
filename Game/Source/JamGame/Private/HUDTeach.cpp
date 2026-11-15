// Lane A213, wave 2 (brief item 5): the first minute teaches three things in order, without a tutorial screen. (1) You have the puck: the double ring
// and its pulse (HUDPossession.cpp) and the pill turning to SHOOT / PASS. (2) Pass it: the pill pulses "RMB  PASS: A MATE IS OPEN" when he carries and
// a mate is open ahead. (3) Hit a man: the pill pulses "RMB  CHECK HIM" when an opposing carrier is in reach in front of him. Each pulse at most
// TeachMaxShows times a session, TeachGapFrames apart, never again after he has done the thing (TeachStep, HUDWidget.h). Presentation only: reads the
// sim, writes nothing back. jam.hud.Teach 0 = no pulse.
#include "HUDWidget.h"
#include "MenuStyle.h"
#include "InputMapping.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HokejGameMode.h"
#include "JamSim.h"
#include "JamEventBus.h"
#include "UnrealClient.h"

static TAutoConsoleVariable<int32> CVarJamHudTeachStills(TEXT("jam.hud.TeachStills"), 0,
	TEXT("Proof: 1 takes a still 12 sim frames into each teaching pulse: a213-teach-<pass|hit>-f<frame>.png. 0 off."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHudTeach(TEXT("jam.hud.Teach"), 1,
	TEXT("Lane A213: 1 (default) the buttons' pill teaches the pass (a mate open ahead) and the check (an opposing carrier in reach in front), at most 3 times a session each, never after he did it; 0 = none."), ECVF_Default);

namespace
{
	UJamHudWidget::FTeachState GTeach[static_cast<int32>(UJamHudWidget::ETeach::Count)];

	FString FirstKeyOf(EJamVerb V)
	{
		const FJamVerbBinding& B = UJamInputMap::Get()->GetBindings()[V];
		const TArray<FKey>& L = UJamInputMap::IsPadLast() ? B.Pads : B.Keys;
		return L.Num() ? UJamInputMap::KeyName(L[0]) : FString(TEXT("-"));
	}
}

FAutoConsoleCommand GJamTeachReset(TEXT("jam.ui.teachreset"), TEXT("Lane A213: the pill's teaching pulses count as not yet shown this session."),
	FConsoleCommandDelegate::CreateLambda([]() { UJamHudWidget::ResetTeachForTest(); }));

bool UJamHudWidget::TeachOn() { return CVarJamHudTeach.GetValueOnGameThread() != 0; }
void UJamHudWidget::ResetTeachForTest() { for (FTeachState& S : GTeach) S = FTeachState(); }
UJamHudWidget::FTeachState& UJamHudWidget::TeachStateFor(ETeach K) { return GTeach[FMath::Clamp(static_cast<int32>(K), 0, 1)]; }

void UJamHudWidget::UpdateTeach(const FJamPresentationFrame& F)
{
	using S = FJamUiStyle;
	if (!ButtonsBadge || !ButtonsBadgeText) return;
	const AHokejGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AHokejGameMode>() : nullptr;
	const UJamSim* Sim = GM ? GM->GetSim() : nullptr;
	int32 Show = -1;
	if (TeachOn() && Sim && F.Phase == EJamPhase::Play && !bCountingThisFrame && Sim->GetSkaters().IsValidIndex(Sim->GetHumanIndex()))
	{
		const TArray<FJamSkaterState>& Sk = Sim->GetSkaters();
		const int32 Hi = Sim->GetHumanIndex();
		const FJamSkaterState& Hu = Sk[Hi];
		const int32 Ci = Sk.IsValidIndex(Sim->CarrierIndex()) ? Sim->CarrierIndex() : -1;
		const FVector2D Face(FMath::Cos(Hu.Yaw), FMath::Sin(Hu.Yaw));
		// What he did this frame: a pass (his pass count grew), a check or a cross-check (his press, or the sim's event of his).
		if (TeachPassesSeen < 0 || Hu.Passes < TeachPassesSeen) TeachPassesSeen = Hu.Passes;
		const bool bPassed = Hu.Passes > TeachPassesSeen;
		TeachPassesSeen = Hu.Passes;
		bool bHit = JamControls::LastHumanButtons().bCheck || JamControls::LastHumanButtons().bLift;
		for (const FJamSimEvent& E : Sim->GetFrameEvents()) if (E.Who == Hi && (E.Name == JamEvents::Check || E.Name == JamEvents::Lift)) bHit = true;
		// The pass's moment: he carries; the mate a full pass along his facing would go to stands 6 to 30 m off, ahead of him.
		bool bPassDue = false;
		if (Ci == Hi && Sim->PassPowerOn() && !bHumanDownNow)
		{
			FVector2D Aim;
			const int32 T = Sim->HumanPassTarget(Hi, Face, Sim->GetConfig().Pass.PowerFullFrames, Aim);
			if (Sk.IsValidIndex(T))
			{
				const FVector2D To = Sk[T].Pos - Hu.Pos;
				const double D = To.Size();
				bPassDue = D >= 6.0 && D <= 30.0 && FVector2D::DotProduct(To / FMath::Max(D, 0.01), Face) > 0.7;
			}
		}
		// The check's moment: he has no puck; the carrier is an opponent within TeachHitRangeM, inside 60 degrees of his facing.
		bool bHitDue = false;
		if (Ci >= 0 && Ci != Hi && Sk[Ci].Team != Hu.Team && !bHumanDownNow)
		{
			const FVector2D To = Sk[Ci].Pos - Hu.Pos;
			const double D = To.Size();
			bHitDue = D <= TeachHitRangeM && FVector2D::DotProduct(To / FMath::Max(D, 0.01), Face) > 0.5;
		}
		const bool bPassOn = TeachStep(TeachStateFor(ETeach::Pass), bPassDue, bPassed, F.SimFrame);
		const bool bHitOn = TeachStep(TeachStateFor(ETeach::Hit), bHitDue, bHit, F.SimFrame);
		Show = bPassOn && Ci == Hi ? 0 : bHitOn && Ci != Hi ? 1 : -1;
		if (bHumanDownNow) Show = -1;
		// 4d (the critic, pro seed 5 f100-120: RMB CHECK HIM while the dial said WON): no teaching in the draw's first second.
		if (LastDropFrame >= 0 && F.SimFrame >= LastDropFrame && F.SimFrame - LastDropFrame < 60) Show = -1;   // knocked down inside a showing (the w2c still at f155): he cannot act, the pill does not ask him to
	}
	if (Show != TeachShown)
	{
		if (Show >= 0)
		{
			const ETeach K = Show == 0 ? ETeach::Pass : ETeach::Hit;
			UE_LOG(LogTemp, Log, TEXT("JAMHUD frame=%lld teach=%s shows=%d"), F.SimFrame, Show == 0 ? TEXT("pass") : TEXT("hit"), TeachStateFor(K).Shows);
		}
		TeachShown = Show;
	}
	// Lane A222: with the panel (or no legend) there is no pill: the panel lights the row it teaches (TeachShown, HUDControlsPanel.cpp).
	if (LegendNow() != ELegend::StripAndPill)
	{
		if (bTeachPainted) { ButtonsBadge->SetBrush(S::Pill(0.72f, 0.f)); bTeachPainted = false; }
		ButtonsBadge->SetRenderScale(FVector2D(1.f, 1.f));
		ButtonsBadge->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	if (Show < 0)
	{
		if (bTeachPainted) { ButtonsBadge->SetBrush(S::Pill(0.72f, 0.f)); bTeachPainted = false; }   // the pill as BuildButtonsBadge made it
		ButtonsBadgeText->SetColorAndOpacity(FSlateColor(S::Paper()));
		ButtonsBadge->SetRenderScale(FVector2D(1.f, 1.f));
		return;
	}
	const ETeach K = Show == 0 ? ETeach::Pass : ETeach::Hit;
	const FString Line = TeachLine(K, FirstKeyOf(K == ETeach::Pass ? EJamVerb::Pass : EJamVerb::Check));
	if (ButtonsBadgeText->GetText().ToString() != Line) ButtonsBadgeText->SetText(FText::FromString(Line));
	if (!bTeachPainted) { ButtonsBadge->SetBrush(S::Rounded(S::Yellow(), 8.f, S::Ink(), 2.f)); bTeachPainted = true; }
	ButtonsBadgeText->SetColorAndOpacity(FSlateColor(S::Ink()));
	ButtonsBadge->SetVisibility(ESlateVisibility::HitTestInvisible);
	ButtonsBadge->SetRenderOpacity(1.f);
	// The pulse: the pill breathes 1.0 to 1.08 twice a second while it teaches.
	const int64 Age = F.SimFrame - TeachStateFor(K).ShownAt;
	if (CVarJamHudTeachStills.GetValueOnGameThread() != 0 && Age == 12)
		FScreenshotRequest::RequestScreenshot(FString::Printf(TEXT("a213-teach-%s-f%06lld.png"), K == ETeach::Pass ? TEXT("pass") : TEXT("hit"), F.SimFrame), true, false);
	const float Breath = 1.f + 0.04f * (1.f - FMath::Cos(static_cast<float>(Age) * 2.f * UE_PI / 30.f));
	ButtonsBadge->SetRenderTransformPivot(FVector2D(0.f, 1.f));
	ButtonsBadge->SetRenderScale(FVector2D(Breath, Breath));
}
