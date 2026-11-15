// Lane A213 (track U3, the night of 14 to 15 Nov; docs/experiments/SCREEN-TELLS.md): possession at a glance (R-0067 point 2: "puck does not stick with
// players"; the HUD's part is that whoever has it is seen to have it, the instant he has it). Presentation only: UpdatePossession reads the SIM's carrier
// (UJamSim::CarrierIndex, never a figure's has-puck flag: device B found figures told they carry on frames the sim's carrier is nobody) and writes nothing
// back. It runs last in RefreshFromFrame, after UpdateYours placed the two rings and UpdateCarrierCues scaled them for the take grace:
//   you carry       your ring doubled (YouHaveRing, YouHaveRingScale x your ring, your team's colour): it stands at your feet where the carried puck may
//                   be hidden behind your hips at this camera;
//   a mate carries  his ring in your team's colour;
//   an opponent     his ring in his team's colour;
//   nobody          a chalk ring round the puck on the ice (LooseRing), not while lane A166's contested ring stands round it, not in a faceoff countdown.
// The pickup: the new carrier's ring pulses (the take grace's closing ring where the grace runs; PossessPulseScale where it does not). The take grace's
// ring keeps the carrier's colour (lane A163 drew it in the HUD's yellow, which said "grace" and hid who had it for the first half second).
// jam.hud.Possess 0 = the screen of before (white carrier ring, yellow grace, no double ring, no loose ring).
#include "HUDWidget.h"
#include "MenuStyle.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HokejGameMode.h"
#include "JamSim.h"
#include "InputMapping.h"
#include "JamCartridge.h"
#include "JamEventBus.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "UnrealClient.h"

static TAutoConsoleVariable<int32> CVarJamHudPossess(TEXT("jam.hud.Possess"), 1,
	TEXT("Lane A213: possession at a glance (1, default): your ring doubled while you carry, the carrier's ring in his team's colour, a chalk ring round a loose puck, a pulse at each pickup. 0 = the screen of before."), ECVF_Default);
// Wave 4: on the night look (look.tier l2_night: blue ice) the home team's blue rings read only by their ink edge (the night stills of READY 3):
// the blue rings there are drawn a step lighter (NightTintShare toward white). 0 = the team colour as it is.
static TAutoConsoleVariable<int32> CVarJamHudNightTint(TEXT("jam.hud.NightTint"), 1,
	TEXT("Lane A213 (wave 4): 1 (default) on the night look the rings in a blue team colour are drawn lighter (they read only by their ink edge on the blue ice); 0 = the team colour."), ECVF_Default);
static constexpr float NightTintShare = 0.25f;   // 0.45 read near-white on the night stills (the team's blue lost)
// Wave 4b: the pickup's pulse keyed on the sim's `possess` event (lane A208: a loose puck becomes a skater's) for the carrier who took it; a carrier
// change by a press (no possess) still pulses as before. And the release: the double ring leaves him on the frame his pass or shot leaves the stick
// (his Passes or Shots grew while the sim still names him the carrier for a few frames). jam.hud.PulseOnPossess 0 = the carrier change alone.
static TAutoConsoleVariable<int32> CVarJamHudPulseOnPossess(TEXT("jam.hud.PulseOnPossess"), 1,
	TEXT("Lane A213 (wave 4b): 1 (default) the pickup pulse restarts on the sim's possess event and the double ring leaves him on his release frame; 0 = the carrier change only."), ECVF_Default);
namespace { int32 GReleasePasses = -1, GReleaseShots = -1, GReleaseCarrier = -2; bool GReleased = false; }

static TAutoConsoleVariable<int32> CVarJamHudRingNearCam(TEXT("jam.hud.RingNearCam"), 1,
	TEXT("Lane A213 (wave 4c): 1 (default) his own rings fade to 30 % while the camera's eye is within 2.5 m of his feet (behind his own net, back to the glass); 0 = as before."), ECVF_Default);
static constexpr float NearCamRingAlpha = 0.3f;
static constexpr double NearCamFadeM = 2.5, NearCamFullM = 3.5;

static TAutoConsoleVariable<int32> CVarJamHudPossessStills(TEXT("jam.hud.PossessStills"), 0,
	TEXT("Proof: N > 0 takes a still (with the HUD) N sim frames into each possession (you / mate / opponent / loose), at most 24: a213-possess-<state>-f<frame>.png. 0 off."), ECVF_Default);

bool UJamHudWidget::PossessOn() { return CVarJamHudPossess.GetValueOnGameThread() != 0; }
bool UJamHudWidget::HudWordsTonight() { return UJamInputMap::WordsTonight(); }

UJamHudWidget::EPossess UJamHudWidget::PossessFromSim(const UJamSim& Sim)
{
	const TArray<FJamSkaterState>& Sk = Sim.GetSkaters();
	const int32 Ci = Sim.CarrierIndex(), Hi = Sim.GetHumanIndex();
	if (!Sk.IsValidIndex(Ci)) return EPossess::Loose;
	return PossessOf(Ci, Hi, Sk[Ci].Team, Sk.IsValidIndex(Hi) ? Sk[Hi].Team : -1);
}

bool UJamHudWidget::IsLooseRingShown() const { return LooseRing && LooseRing->GetVisibility() != ESlateVisibility::Collapsed; }
bool UJamHudWidget::IsYouHaveRingShown() const { return YouHaveRing && YouHaveRing->GetVisibility() != ESlateVisibility::Collapsed; }

namespace
{
	int32 GPossessStills = 0;
	bool GPossessStillTaken = false;

	UImage* JamMakePossessRing(UWidgetTree* Tree, UCanvasPanel* Root, const TCHAR* Name)
	{
		if (!Tree || !Root) return nullptr;
		UImage* I = Tree->ConstructWidget<UImage>(UImage::StaticClass(), Name);
		if (UTexture2D* Plain = FJamUiStyle::Art(TEXT("T_UI_RingPlain"))) I->SetBrush(FJamUiStyle::ImageBrush(Plain, FVector2D(64.f, 64.f)));
		else I->SetBrush(FJamUiStyle::Ring(64.f));
		I->SetVisibility(ESlateVisibility::Collapsed);
		if (UCanvasPanelSlot* CS = Root->AddChildToCanvas(I))
		{
			CS->SetAnchors(FAnchors(0.f, 0.f));
			CS->SetAlignment(FVector2D(0.5f, 0.5f));
			CS->SetAutoSize(false);
			CS->SetZOrder(-1);   // on the ice: under every other element
		}
		return I;
	}

	bool Shown(const UWidget* W) { return W && W->GetVisibility() != ESlateVisibility::Collapsed; }
}

void UJamHudWidget::UpdatePossession(const FJamPresentationFrame& F)
{
	if (!Root || !WidgetTree) return;
	if (!YouHaveRing) YouHaveRing = JamMakePossessRing(WidgetTree, Root, TEXT("YouHaveRing"));
	if (!LooseRing) LooseRing = JamMakePossessRing(WidgetTree, Root, TEXT("LooseRing"));
	if (!YouHaveRing || !LooseRing) return;
	auto Hide = [](UWidget* W) { if (W) W->SetVisibility(ESlateVisibility::Collapsed); };
	const AHokejGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AHokejGameMode>() : nullptr;
	const UJamSim* Sim = GM ? GM->GetSim() : nullptr;
	const bool bPlay = F.Phase == EJamPhase::Play || F.Phase == EJamPhase::GoalPause;
	if (!PossessOn() || !Sim || !bPlay || !Sim->GetSkaters().IsValidIndex(Sim->GetHumanIndex()))
	{
		Hide(YouHaveRing); Hide(LooseRing);
		PossessCarrier = -2; PossessSince = -1; PossessShownState = static_cast<uint8>(EPossess::Loose);
		return;
	}
	const TArray<FJamSkaterState>& Sk = Sim->GetSkaters();
	const int32 Hi = Sim->GetHumanIndex();
	const int32 Ci = Sk.IsValidIndex(Sim->CarrierIndex()) ? Sim->CarrierIndex() : -1;
	const EPossess P = PossessFromSim(*Sim);
	if (Ci != PossessCarrier || F.SimFrame < PossessSince)
	{
		UE_LOG(LogTemp, Log, TEXT("JAMHUD frame=%lld possess=%s carrier=%d human=%d was=%d"), F.SimFrame, PossessName(P), Ci, Hi, PossessCarrier);
		PossessCarrier = Ci;
		PossessSince = F.SimFrame;
		GPossessStillTaken = false;
	}
	if (CVarJamHudPulseOnPossess.GetValueOnGameThread() != 0)
	{
		for (const FJamSimEvent& E : Sim->GetFrameEvents()) if (E.Name == JamEvents::Possess && E.Who == Ci && Ci >= 0) PossessSince = F.SimFrame;
	}
	// The release (wave 4b): his pass or shot left the stick while the sim still names him the carrier.
	{
		const FJamSkaterState& HuR = Sk[Hi];
		if (Ci != GReleaseCarrier || HuR.Passes < GReleasePasses || HuR.Shots < GReleaseShots) { GReleaseCarrier = Ci; GReleasePasses = HuR.Passes; GReleaseShots = HuR.Shots; GReleased = false; }
		if (Ci == Hi && (HuR.Passes > GReleasePasses || HuR.Shots > GReleaseShots)) GReleased = true;
	}
	const bool bReleased = CVarJamHudPulseOnPossess.GetValueOnGameThread() != 0 && GReleased && Ci == Hi;
	PossessShownState = static_cast<uint8>(bReleased ? EPossess::Loose : P);
	const int64 Age = F.SimFrame - PossessSince;
	const bool bGrace = Ci >= 0 && Sim->InTakeGrace(Sk[Ci]);
	// UpdateCarrierCues already sized the ring for the grace (1.5 x closing): that is the pulse while the grace runs.
	const float Pulse = bGrace ? 1.f : PossessPulseScale(Age);
	auto Recolour = [](UImage* Ring, const FLinearColor& C, float Alpha) { Ring->SetColorAndOpacity(FLinearColor(C.R, C.G, C.B, Alpha)); };
	// Wave 4: the ring colour of a team on this look (the night look's blue lightened).
	const bool bNight = CVarJamHudNightTint.GetValueOnGameThread() != 0 && GM && GM->GetCartridge() && GM->GetCartridge()->Game.Look.Tier == TEXT("l2_night");
	auto RingColour = [&](int32 TeamId)
	{
		const FLinearColor C = F.GetTeamColour(TeamId);
		return bNight && C.B > C.R && C.B > C.G ? FMath::Lerp(C, FLinearColor::White, NightTintShare) : C;
	};
	if (bNight && YouRing && Shown(YouRing)) Recolour(YouRing, RingColour(Sk[Hi].Team), YouRing->GetColorAndOpacity().A);
	// Wave 4c (the camera lane, 06:15: behind his own net, back to the glass, the camera's eye 2 m up almost over him: "the screen is a blue ring and
	// the net's mesh"): his own rings fade to NearCamRingAlpha while the eye is within NearCamFadeM of his feet, back by NearCamFullM.
	float NearCam = 1.f;
	if (CVarJamHudRingNearCam.GetValueOnGameThread() != 0)
	{
		const APlayerController* PCc = GetOwningPlayer();
		if (PCc && PCc->PlayerCameraManager)
		{
			const double D = FVector::Dist(PCc->PlayerCameraManager->GetCameraLocation(), UJamSim::ToWorld(Sk[Hi].Pos, 0.0)) / 100.0;
			NearCam = FMath::Lerp(NearCamRingAlpha, 1.f, FMath::Clamp(static_cast<float>((D - NearCamFadeM) / (NearCamFullM - NearCamFadeM)), 0.f, 1.f));
		}
		if (NearCam < 1.f && YouRing && Shown(YouRing)) { const FLinearColor C = YouRing->GetColorAndOpacity(); YouRing->SetColorAndOpacity(FLinearColor(C.R, C.G, C.B, C.A * NearCam)); }
	}
	auto Grow = [](UImage* Ring, float K) { if (UCanvasPanelSlot* CS = Cast<UCanvasPanelSlot>(Ring->Slot); CS && K != 1.f) CS->SetSize(CS->GetSize() * K); };

	// YOU: the ring under him doubled, in his team's colour (the grace's yellow taken back).
	bool bYouHave = false;
	if (P == EPossess::You && !bReleased && YouRing && Shown(YouRing))
	{
		const FLinearColor Team = RingColour(Sk[Hi].Team);
		const float A = YouRing->GetColorAndOpacity().A;
		Recolour(YouRing, Team, A);
		Grow(YouRing, Pulse);
		if (const UCanvasPanelSlot* RS = Cast<UCanvasPanelSlot>(YouRing->Slot))
		{
			Recolour(YouHaveRing, Team, A * YouHaveRingAlpha);
			YouHaveRing->SetVisibility(ESlateVisibility::HitTestInvisible);
			if (UCanvasPanelSlot* CS = Cast<UCanvasPanelSlot>(YouHaveRing->Slot)) { CS->SetPosition(RS->GetPosition()); CS->SetSize(RS->GetSize() * YouHaveRingScale); }
			bYouHave = true;
		}
	}
	if (!bYouHave) Hide(YouHaveRing);

	// A MATE or an OPPONENT: the carrier's ring in his team's colour.
	if ((P == EPossess::Mate || P == EPossess::Opponent) && CarrierRing && Shown(CarrierRing))
	{
		Recolour(CarrierRing, RingColour(Sk[Ci].Team), 0.95f);
		Grow(CarrierRing, Pulse);
	}

	// LOOSE: the puck's own ring on the ice (it yields to the contested mark, a countdown and a downed figure).
	bool bLoose = false;
	if (P == EPossess::Loose && F.Phase == EJamPhase::Play && !bCountingThisFrame && !Shown(ContestRing.Get()))
	{
		FVector2D C, Size;
		float DistM = 0.f;
		if (ProjectGroundRing(F.PuckM, LooseRingRadiusM, C, Size, DistM))
		{
			Size *= PossessPulseScale(Age);
			// Far off (the attack's end, 30 m and more) the ring is a few pixels: never under LooseRingMinPx across (the first stills: unreadable at 30 m).
			if (Size.X > 0.0 && Size.X < LooseRingMinPx) Size *= LooseRingMinPx / Size.X;
			const FVector2D Min = C - Size * 0.5f, Max = C + Size * 0.5f;
			// Wave 4b (the eye-hand's stills, f220): a pass in flight is going to a mate (its ring is under him: HUDChargeLook.cpp), not loose; and the
			// ring is never drawn over his own figure (the puck just past his stick projected onto his torso, a ring floating at his chest).
			const bool bPassFlying = Sim->GetPuck().PassFrames > 0;
			const bool bOnHim = bHasHumanBox && BoxesMeet(Min, Max, HumanBoxNow.Min, HumanBoxNow.Max);
			if (!bPassFlying && !bOnHim && !(ClearDownOn() && BoxMeetsAny(Min, Max, DownBoxesNow)))
			{
				Recolour(LooseRing, DistM > LooseRingFarM ? FJamUiStyle::Yellow() : FJamUiStyle::Paper(), 0.9f);
				LooseRing->SetVisibility(ESlateVisibility::HitTestInvisible);
				if (UCanvasPanelSlot* CS = Cast<UCanvasPanelSlot>(LooseRing->Slot)) { CS->SetPosition(C); CS->SetSize(Size); }
				bLoose = true;
			}
		}
	}
	if (!bLoose) Hide(LooseRing);
	// The contested puck (lane A166 / A178) is ONE ring: its outer ring round the puck in the opponent's colour (it stands in the loose ring's place);
	// its inner ring and the ring under the coming opponent go (A211's EYE 9 and list 2: four ring systems round one pair of skaters).
	Hide(ContestRingInner.Get());
	Hide(ContestOppRing.Get());

	// The proof's stills: one per possession, N sim frames into it.
	const int32 After = CVarJamHudPossessStills.GetValueOnGameThread();
	if (After > 0 && !GPossessStillTaken && GPossessStills < 24 && Age >= After && F.Phase == EJamPhase::Play)
	{
		GPossessStillTaken = true;
		++GPossessStills;
		const FString Name = FString::Printf(TEXT("a213-possess-%s-f%06lld"), PossessName(P), F.SimFrame);
		FScreenshotRequest::RequestScreenshot(Name + TEXT(".png"), true, false);
		UE_LOG(LogTemp, Log, TEXT("JAMHUD frame=%lld possess_still=%s.png carrier=%d you_have=%d loose_ring=%d grace=%d"), F.SimFrame, *Name, Ci, bYouHave ? 1 : 0, bLoose ? 1 : 0, bGrace ? 1 : 0);
	}
}
