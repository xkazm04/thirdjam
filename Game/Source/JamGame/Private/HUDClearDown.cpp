// Lane A158 (track U1, the screen, evolution 5; B-to-A-0041 point 1; docs/experiments/SCREEN-2.md): nothing is drawn on top of the key feature.
// The fall, the lying and the get-up are what the operator named the key feature; device B's critic found the HUD drawn over them (the ring across the
// lying body, the caption on it, a bubble over the middle ice). This file holds the boxes the HUD keeps clear each tick (the human's figure, every downed
// figure, the nets), the rules that read them, and the probe that measures the bar: per knockdown, the frames on which any HUD element's painted box met
// the downed figure's projected box. Presentation only: it reads the sim's down flag and the figures' bones, and writes nothing back.
#include "HUDWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/MeshComponent.h"
#include "Components/SkinnedMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "UObject/UObjectIterator.h"
#include "UnrealClient.h"
#include "HokejGameMode.h"
#include "JamCartridge.h"
#include "JamSim.h"
#include "JamPawns.h"
#include "RadarWidget.h"
#include "DirectorsCallWidget.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"

static TAutoConsoleVariable<int32> CVarJamHudClearDown(TEXT("jam.hud.ClearDown"), 1,
	TEXT("Nothing drawn on a downed figure: 1 (default) no ring under the human while he is down, no marker on a downed figure, the caption and the contact line off the human, the bark lane off downed figures and nets; 0 the screen of before."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHudDownProbe(TEXT("jam.hud.DownProbe"), 0,
	TEXT("N > 0: a JAMSCREEN down line per knockdown (frames, frames with a HUD element over the downed figure's box, by element) and a JAMSCREEN downs line of totals every N sim frames. 0 off."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHudDownStills(TEXT("jam.hud.DownStills"), 0,
	TEXT("Proof: N > 0 takes two stills of every knockdown of the human, N and 3 N sim frames into it (u1-down-<tier>-f<frame>-<n>.png, with the HUD). 0 off."), ECVF_Default);

static TAutoConsoleVariable<FString> CVarJamHudDownStillTag(TEXT("jam.hud.DownStillTag"), TEXT(""),
	TEXT("Proof: a tag in the names of the knockdown stills (u1-down-<tier>-<tag>-f<frame>-<n>.png), so several processes can write into one screenshot folder."), ECVF_Default);

uint64 UJamHudWidget::KeepClearTick = 0;
TArray<FBox2D> UJamHudWidget::KeepClearFigures;
TArray<FBox2D> UJamHudWidget::KeepClearNets;

bool UJamHudWidget::ClearDownOn() { return CVarJamHudClearDown.GetValueOnGameThread() != 0; }

// Lane A171 (track U1, evolution 6): the last overlaps (see HUDWidget.h).
static TAutoConsoleVariable<int32> CVarJamHudClearEdges(TEXT("jam.hud.ClearEdges"), 0,
	TEXT("0 (default until the Director turns it on: it follows the package): as on main. 1: the caption's row under the scoreboard and the one-time prompt keep clear of the MATCH card and the net / puck markers, the own-net label stands above the radar and the ability plate, the Director's Call plates are not drawn on a downed figure; 0 as before."), ECVF_Default);

bool UJamHudWidget::ClearEdgesOn() { return CVarJamHudClearEdges.GetValueOnGameThread() != 0; }

// What stands in the band under the scoreboard this tick, as boxes in the viewport's Slate units.
void UJamHudWidget::TopBandObstacles(TArray<FBox2D>& Out, float VW, float VH) const
{
	Out.Reset();
	const UJamDirectorsCallHudWidget* Overlay = OverlayForPrompt.Get();
	// The overlay's line (a call's result, a RULE line) holds the first row; the prompt's own box wherever it stands.
	if (Overlay && Overlay->IsLineVisible()) Out.Add(FBox2D(FVector2D(VW * 0.5f - 480.f, PromptSlotY - 4.f), FVector2D(VW * 0.5f + 480.f, PromptSlotLowY - 10.f)));
	if (PromptPlate && PromptPlate->GetVisibility() != ESlateVisibility::Collapsed)
	{
		FBox2D B(ForceInit);
		if (WidgetBoxOnScreen(PromptPlate, B)) Out.Add(B);
	}
	if (!ClearEdgesOn()) return;
	float BandTop = 0.f, BandBottom = 0.f;
	if (Overlay && Overlay->GetBannerBandPx(VH, BandTop, BandBottom))
		Out.Add(FBox2D(FVector2D(VW * 0.5f - UJamDirectorsCallHudWidget::MatchCardW * 0.5f - 8.f, BandTop), FVector2D(VW * 0.5f + UJamDirectorsCallHudWidget::MatchCardW * 0.5f + 8.f, BandBottom)));
	const UWidget* const Marks[] = { NetDiamond.Get(), NetText.Get(), NetArrow.Get(), OwnDiamond.Get(), OwnText.Get(), OwnArrow.Get(), PuckArrow.Get(), PuckText.Get() };
	for (const UWidget* W : Marks)
	{
		if (!W || W->GetVisibility() == ESlateVisibility::Collapsed || W->GetRenderOpacity() <= 0.f) continue;
		FBox2D B(ForceInit);
		if (WidgetBoxOnScreen(W, B) && B.Min.Y < VH / 3.f) Out.Add(B.ExpandBy(10.f));   // the label's plate and a turned chevron paint past the slot's box
	}
}

// The Director's Call widget (another widget: its line, its MATCH card, its CALL READY pill, each a child of its root canvas, shown and hidden by
// visibility alone): a child whose painted box meets a downed figure's box is drawn with no ink this tick, and with all of it again when clear.
void UJamHudWidget::YieldOverlayToDowned()
{
	// The rings again, at the size they are drawn: lane A163's take grace (UpdateCarrierCues, last in the refresh) draws the carrier's ring and the human's at
	// up to 1.5 x their size after the HUD's own yield ran (the carrier's ring lay on the downed human for 11 frames of one knockdown in eight). Down wins.
	if (ClearDownOn() && DownBoxesNow.Num() > 0)
	{
		UWidget* const Rings[] = { YouRing.Get(), CarrierRing.Get() };
		for (UWidget* W : Rings)
		{
			if (!W || W->GetVisibility() == ESlateVisibility::Collapsed) continue;
			FBox2D B(ForceInit);
			if (WidgetBoxOnScreen(W, B) && BoxMeetsAny(B.Min, B.Max, DownBoxesNow, 16.f)) W->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	if (!OverlayForPrompt.IsValid() && (OverlayYieldTick++ % 60) == 0)
	{
		for (TObjectIterator<UJamDirectorsCallHudWidget> It; It; ++It) if (It->GetWorld() == GetWorld() && !It->HasAnyFlags(RF_ClassDefaultObject)) { OverlayForPrompt = *It; break; }
	}
	UJamDirectorsCallHudWidget* Overlay = OverlayForPrompt.Get();
	UPanelWidget* Canvas = Overlay && Overlay->WidgetTree ? Cast<UPanelWidget>(Overlay->WidgetTree->RootWidget) : nullptr;
	if (!Canvas) return;
	const bool bOn = ClearDownOn() && ClearEdgesOn() && DownBoxesNow.Num() > 0;
	const FVector2D VS = UWidgetLayoutLibrary::GetViewportSize(this);
	const float Scale = FMath::Max(UWidgetLayoutLibrary::GetViewportScale(this), 0.01f);
	const double ScreenArea = (VS.X / Scale) * (VS.Y / Scale);
	for (int32 i = 0; i < Canvas->GetChildrenCount(); ++i)
	{
		UWidget* W = Canvas->GetChildAt(i);
		if (!W) continue;
		bool bHide = false;
		if (bOn && W->GetVisibility() != ESlateVisibility::Collapsed)
		{
			FBox2D B(ForceInit);
			bHide = PaintedBoxOnScreen(W, B) && B.GetSize().X * B.GetSize().Y < 0.4 * ScreenArea && BoxMeetsAny(B.Min, B.Max, DownBoxesNow);
		}
		const float Want = bHide ? 0.f : 1.f;
		if (W->GetRenderOpacity() != Want) W->SetRenderOpacity(Want);
	}
}

bool UJamHudWidget::GetKeepClear(TArray<FBox2D>& OutFigures, TArray<FBox2D>& OutNets)
{
	if (KeepClearTick == 0 || GFrameCounter > KeepClearTick + 1 || !ClearDownOn()) return false;
	OutFigures = KeepClearFigures;
	OutNets = KeepClearNets;
	return true;
}

bool UJamHudWidget::SideStep(float WantX, float HalfW, float Y0, float Y1, const TArray<FBox2D>& Boxes, float MinX, float MaxX, float Pad, float& OutX)
{
	if (MaxX < MinX) return false;
	auto Free = [&](float X) { return X >= MinX - 0.01f && X <= MaxX + 0.01f && !BoxMeetsAny(FVector2D(X - HalfW, Y0), FVector2D(X + HalfW, Y1), Boxes, Pad); };
	const float W0 = FMath::Clamp(WantX, MinX, MaxX);
	if (Free(W0)) { OutX = W0; return true; }
	// The candidates: just left and just right of every box that stands on the row; the nearest free one.
	float Best = 0.f;
	bool bAny = false;
	for (const FBox2D& B : Boxes)
	{
		if (!(Y0 - Pad < B.Max.Y && Y1 + Pad > B.Min.Y)) continue;
		const float Cand[2] = { static_cast<float>(B.Min.X) - Pad - HalfW - 1.f, static_cast<float>(B.Max.X) + Pad + HalfW + 1.f };
		for (const float X : Cand)
		{
			if (Free(X) && (!bAny || FMath::Abs(X - W0) < FMath::Abs(Best - W0))) { Best = X; bAny = true; }
		}
	}
	if (bAny) OutX = Best;
	return bAny;
}

int32 UJamHudWidget::FigureBoxOnScreen(APlayerController* PC, const AJamSkater* Skater, float PadCm, FBox2D& OutBox)
{
	OutBox = FBox2D(ForceInit);
	if (!PC || !PC->PlayerCameraManager || !Skater) return 0;
	TArray<UMeshComponent*> Meshes;
	UObject* Visual = Skater->GetVisualObject();
	if (const AActor* A = Cast<AActor>(Visual)) A->GetComponents<UMeshComponent>(Meshes);
	else if (USceneComponent* SC = Cast<USceneComponent>(Visual))
	{
		TArray<USceneComponent*> All;
		All.Add(SC);
		SC->GetChildrenComponents(true, All);
		for (USceneComponent* C : All) if (UMeshComponent* M = Cast<UMeshComponent>(C)) Meshes.Add(M);
	}
	if (!Meshes.Num()) Skater->GetComponents<UMeshComponent>(Meshes);
	const FVector CamLoc = PC->PlayerCameraManager->GetCameraLocation();
	const FVector CamFwd = PC->PlayerCameraManager->GetCameraRotation().Vector();
	double DepthSum = 0.0;
	int32 N = 0;
	auto AddPoint = [&](const FVector& W)
	{
		const double Depth = FVector::DotProduct(W - CamLoc, CamFwd);
		if (Depth < 20.0) return;   // at or behind the camera: no place on the screen
		FVector2D Sp;
		if (!UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, W, Sp, false)) return;
		OutBox += Sp;
		DepthSum += Depth;
		++N;
	};
	int32 Source = 0;
	for (const UMeshComponent* M : Meshes)
	{
		const USkinnedMeshComponent* Sk = Cast<USkinnedMeshComponent>(M);
		if (!Sk || !Sk->IsRegistered() || !Sk->IsVisible() || Sk->GetNumBones() < 4) continue;
		for (int32 b = 0; b < Sk->GetNumBones(); ++b) AddPoint(Sk->GetBoneTransform(b).GetLocation());
		Source = 2;
	}
	if (Source == 0)
	{
		for (const UMeshComponent* M : Meshes)
		{
			if (!M || !M->IsRegistered() || !M->IsVisible()) continue;
			const FBox B = M->Bounds.GetBox();
			if (B.GetExtent().GetMax() > 400.0) continue;   // not a figure's part (a helper plane, a trail's bounds)
			for (int32 c = 0; c < 8; ++c) AddPoint(FVector(c & 1 ? B.Max.X : B.Min.X, c & 2 ? B.Max.Y : B.Min.Y, c & 4 ? B.Max.Z : B.Min.Z));
			Source = 1;
		}
	}
	if (N == 0) { OutBox = FBox2D(ForceInit); return 0; }
	// The pad: PadCm at the figure's mean depth, in the widget's units.
	const float Scale = FMath::Max(UWidgetLayoutLibrary::GetViewportScale(PC), 0.01f);
	const float VW = static_cast<float>(UWidgetLayoutLibrary::GetViewportSize(PC).X) / Scale;
	const double TanH = FMath::Tan(FMath::DegreesToRadians(FMath::Clamp(static_cast<double>(PC->PlayerCameraManager->GetFOVAngle()), 5.0, 170.0) * 0.5));
	const double PadPx = PadCm * (VW * 0.5 / TanH) / FMath::Max(DepthSum / N, 20.0);
	OutBox = OutBox.ExpandBy(PadPx);
	return Source;
}

void UJamHudWidget::UpdateKeepClear(const FJamPresentationFrame& F)
{
	bHasHumanBox = false;
	DownBoxesNow.Reset();
	KeepBoxesNow.Reset();
	DownWhoNow.Reset();
	DownSourceNow.Reset();
	APlayerController* PC = GetOwningPlayer();
	const AHokejGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AHokejGameMode>() : nullptr;
	const UJamSim* Sim = GM ? GM->GetSim() : nullptr;
	const bool bPlay = F.Phase == EJamPhase::Play || F.Phase == EJamPhase::GoalPause;
	// The skaters' actors by sim index: looked for again every 120 ticks and whenever one is gone (a new match, a figure swapped by a call).
	bool bStale = SkaterActors.Num() != F.Skaters.Num() || (SkaterSearchTick++ % 120) == 0;
	for (const TWeakObjectPtr<AJamSkater>& W : SkaterActors) if (!W.IsValid()) bStale = true;
	if (bStale && GetWorld())
	{
		SkaterActors.Reset();
		SkaterActors.SetNum(F.Skaters.Num());
		for (TActorIterator<AJamSkater> It(GetWorld()); It; ++It) if (SkaterActors.IsValidIndex(It->GetSimIndex())) SkaterActors[It->GetSimIndex()] = *It;
	}
	const bool bWasDown = bHumanDownNow;
	bHumanDownNow = bPlay && Sim && Sim->GetSkaters().IsValidIndex(F.HumanIndex) && Sim->GetSkaters()[F.HumanIndex].IsDown();
	if (bHumanDownNow) HumanUpSince = -1;
	else if (bWasDown) HumanUpSince = F.SimFrame;
	const FVector2D VS = UWidgetLayoutLibrary::GetViewportSize(this);
	const float Scale = FMath::Max(UWidgetLayoutLibrary::GetViewportScale(this), 0.01f);
	const float VW = VS.X / Scale, VH = VS.Y / Scale;
	if (PC && Sim && bPlay)
	{
		for (int32 i = 0; i < F.Skaters.Num() && i < SkaterActors.Num(); ++i)
		{
			const bool bHuman = i == F.HumanIndex;
			const bool bDown = Sim->GetSkaters().IsValidIndex(i) && Sim->GetSkaters()[i].IsDown();
			if (!bHuman && !bDown) continue;
			FBox2D Box(ForceInit);
			const int32 Source = FigureBoxOnScreen(PC, SkaterActors[i].Get(), FigurePadCm, Box);
			if (Source == 0 || Box.Max.X < 0.f || Box.Min.X > VW || Box.Max.Y < 0.f || Box.Min.Y > VH) continue;   // not in the picture
			if (bHuman) { HumanBoxNow = Box; bHasHumanBox = true; }
			if (bDown) { DownBoxesNow.Add(Box); DownWhoNow.Add(i); DownSourceNow.Add(Source); }
			KeepBoxesNow.Add(Box);
		}
	}
	// For the bark lane (its own widget): the figures above and both nets' mouths (2 m to either side of the centre, ice to crossbar).
	KeepClearTick = GFrameCounter;
	KeepClearFigures = KeepBoxesNow;
	KeepClearNets.Reset();
	if (PC && bPlay)
	{
		const FVector Far = F.GetFarNetWorldCm();
		const double NetX[2] = { Far.X, -Far.X };
		for (const double X : NetX)
		{
			FBox2D Box(ForceInit);
			bool bAll = true;
			for (int32 c = 0; c < 4 && bAll; ++c)
			{
				FVector2D Sp;
				bAll = UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, FVector(X, Far.Y + (c & 1 ? 200.0 : -200.0), c & 2 ? NetCrossbarCm : 0.0), Sp, false);
				if (bAll) Box += Sp;
			}
			if (bAll && Box.Max.X > 0.f && Box.Min.X < VW && Box.Max.Y > 0.f && Box.Min.Y < VH) KeepClearNets.Add(Box);
		}
	}
}

float UJamHudWidget::StepRingYield(bool bOn, const FVector2D& Centre, const FVector2D& Size)
{
	if (!ClearDownOn() || !bOn) { RingYield = 1.f; return 1.f; }
	const bool bOnDowned = BoxMeetsAny(Centre - Size * 0.5, Centre + Size * 0.5, DownBoxesNow, 0.f);
	if (bSimAdvancedThisFrame) RingYield = RingYieldStep(RingYield, bOnDowned);
	return RingYield;
}

bool UJamHudWidget::WidgetBoxOnScreen(const UWidget* W, FBox2D& OutBox) const
{
	const UCanvasPanelSlot* CS = W ? Cast<UCanvasPanelSlot>(W->Slot) : nullptr;
	if (!CS) return false;
	UJamHudWidget* Self = const_cast<UJamHudWidget*>(this);
	const FVector2D VS = UWidgetLayoutLibrary::GetViewportSize(Self);
	const float Scale = FMath::Max(UWidgetLayoutLibrary::GetViewportScale(Self), 0.01f);
	const FAnchors An = CS->GetAnchors();
	const FVector2D Anchor(An.Minimum.X * VS.X / Scale, An.Minimum.Y * VS.Y / Scale);
	FVector2D Size = CS->GetAutoSize() ? W->GetDesiredSize() : CS->GetSize();
	Size = FVector2D(FMath::Max(Size.X, 8.0), FMath::Max(Size.Y, 8.0));
	const FVector2D Min = Anchor + CS->GetPosition() - CS->GetAlignment() * Size;
	OutBox = FBox2D(Min, Min + Size);
	return true;
}

// The markers that stand at a skater's feet or over the world: not drawn on a frame their box meets a downed figure's (their own updates show them again).
void UJamHudWidget::YieldToDowned()
{
	if (!ClearDownOn() || !DownBoxesNow.Num()) return;
	UWidget* const List[] = { CarrierRing.Get(), NetDiamond.Get(), NetText.Get(), NetArrow.Get(), OwnDiamond.Get(), OwnText.Get(), OwnArrow.Get(), PuckArrow.Get(), PuckText.Get() };
	for (UWidget* W : List)
	{
		if (!W || W->GetVisibility() == ESlateVisibility::Collapsed) continue;
		FBox2D B(ForceInit);
		// The pad: a label stands on a plate wider than its type (LabelPlatePad) and a turned chevron paints past its slot's box; 16 covers both
		// (with the 6 of the captions one knockdown in nine had the puck arrow's plate on the figure's box for a frame).
		if (WidgetBoxOnScreen(W, B) && BoxMeetsAny(B.Min, B.Max, DownBoxesNow, 16.f)) W->SetVisibility(ESlateVisibility::Collapsed);
	}
}

// The contact line's row: its bottom ContactRowPx over the bottom edge (BuildTree's place), between the frame's left margin and the ability plate.
void UJamHudWidget::PlaceContactLine()
{
	static constexpr float ContactRowPx = 192.f;
	UCanvasPanelSlot* CS = ContactPlate ? Cast<UCanvasPanelSlot>(ContactPlate->Slot) : nullptr;
	if (!CS) return;
	const bool bShown = ContactPlate->GetVisibility() != ESlateVisibility::Collapsed;
	if (!bShown || !ClearDownOn())
	{
		if (bContactStepped) { bContactStepped = false; ContactStepX = 0.f; CS->SetPosition(FVector2D(0, -ContactRowPx)); }
		return;
	}
	const FVector2D VS = UWidgetLayoutLibrary::GetViewportSize(this);
	const float Scale = FMath::Max(UWidgetLayoutLibrary::GetViewportScale(this), 0.01f);
	const float VW = VS.X / Scale, VH = VS.Y / Scale;
	const FVector2D Sz(FMath::Max(ContactPlate->GetDesiredSize().X, 200.0), FMath::Max(ContactPlate->GetDesiredSize().Y, 36.0));
	const float HalfW = static_cast<float>(Sz.X) * 0.5f;
	float X = VW * 0.5f;
	// It stays where it stepped to while that place is free (no hop back under a figure that is getting up).
	if (SideStep(VW * 0.5f + ContactStepX, HalfW, VH - ContactRowPx - static_cast<float>(Sz.Y), VH - ContactRowPx, KeepBoxesNow, 48.f + HalfW, VW - 430.f - HalfW, KeepClearPadPx, X))
	{
		ContactStepX = X - VW * 0.5f;
		bContactStepped = true;
		CS->SetPosition(FVector2D(ContactStepX, -ContactRowPx));
	}
	else ContactPlate->SetRenderOpacity(0.f);   // no free place on its row this frame
}

bool UJamHudWidget::PaintedBoxOnScreen(const UWidget* W, FBox2D& OutBox) const
{
	if (!W || !W->GetCachedWidget().IsValid()) return false;
	const FGeometry& G = W->GetCachedGeometry();
	const FVector2D Sz = G.GetAbsoluteSize();
	if (Sz.X < 1.0 || Sz.Y < 1.0) return false;
	const FSlateRect R = G.GetRenderBoundingRect();
	UJamHudWidget* Self = const_cast<UJamHudWidget*>(this);
	FVector2D Px, V0, V1;
	USlateBlueprintLibrary::AbsoluteToViewport(Self, FVector2D(R.Left, R.Top), Px, V0);
	USlateBlueprintLibrary::AbsoluteToViewport(Self, FVector2D(R.Right, R.Bottom), Px, V1);
	OutBox = FBox2D(V0, V1);
	return true;
}

FString UJamHudWidget::ProbeNameOf(const UWidget* W) const
{
	// The element a widget belongs to: its ancestor that is a child of this HUD's canvas, by the name the track uses for it.
	const UWidget* Top = W;
	while (Top && Top->GetParent() && Top->GetParent() != Root.Get()) Top = Top->GetParent();
	struct FRow { const UWidget* W; const TCHAR* Name; };
	const FRow Rows[] = {
		{ StopMarkBar.Get(), TEXT("stop_mark") }, { ContestRing.Get(), TEXT("contest_ring") }, { YouRing.Get(), TEXT("ring") }, { CarrierRing.Get(), TEXT("carrier_ring") }, { YouChevron.Get(), TEXT("chevron") }, { YouTag.Get(), TEXT("you_tag") },
		{ CommentaryStrip.Get(), TEXT("caption") }, { ContactPlate.Get(), TEXT("contact_line") }, { PromptPlate.Get(), TEXT("prompt") }, { ToastPlate.Get(), TEXT("toast") },
		{ NetDiamond.Get(), TEXT("net_badge") }, { NetText.Get(), TEXT("net_badge") }, { NetArrow.Get(), TEXT("net_badge") }, { NetPlate.Get(), TEXT("net_badge") },
		{ OwnDiamond.Get(), TEXT("own_badge") }, { OwnText.Get(), TEXT("own_badge") }, { OwnArrow.Get(), TEXT("own_badge") }, { OwnPlate.Get(), TEXT("own_badge") },
		{ PuckArrow.Get(), TEXT("puck_arrow") }, { PuckText.Get(), TEXT("puck_arrow") }, { PuckPlate.Get(), TEXT("puck_arrow") },
		{ ControlsStrip.Get(), TEXT("controls") }, { ControlsText.Get(), TEXT("controls") }, { CountdownBox.Get(), TEXT("faceoff") }, { FaceoffWord.Get(), TEXT("faceoff") },
		{ FaceoffHintPlate.Get(), TEXT("faceoff") }, { Radar.Get(), TEXT("radar") }, { StaminaBack.Get(), TEXT("stamina") }, { StaminaFill.Get(), TEXT("stamina") },
		{ AbilityPlate.Get(), TEXT("ability") }, { AbilityFill.Get(), TEXT("ability") }, { CentrePlate.Get(), TEXT("scoreboard") }, { HomeBox.Get(), TEXT("scoreboard") },
		{ AwayBox.Get(), TEXT("scoreboard") }, { CardPlate.Get(), TEXT("card") }, { EffectsPlate.Get(), TEXT("effects") }, { CallText.Get(), TEXT("call_ready") } };
	for (const FRow& R : Rows) if (R.W && (R.W == Top || R.W == W)) return R.Name;
	return Top ? Top->GetName() : FString(TEXT("?"));
}

// Lane A168 (the black post at the faceoff): what stands at the puck. jam.hud.NearPuck 1: at every faceoff hold (20 frames before the drop) and 240
// frames after each drop, one JAMSCREEN nearpuck line per visible primitive component whose bounds' centre is within 1.5 m of the puck on the ice
// (an instanced mesh: per instance), with its class, owner, mesh, place, scale, bounds and first material, and a still (u1-post-<tier>-<tag>f<frame>-<hold|play>.png).
static TAutoConsoleVariable<int32> CVarJamHudNearPuck(TEXT("jam.hud.NearPuck"), 0,
	TEXT("Proof: 1 = JAMSCREEN nearpuck lines (every visible primitive within 1.5 m of the puck) and a still at each faceoff hold and 240 frames after each drop. 0 off."), ECVF_Default);

static void JamDumpNearPuck(UWorld* World, const FJamPresentationFrame& F, const TCHAR* Why, const FString& Tier)
{
	const FVector2D Puck(F.PuckM.X * 100.0, F.PuckM.Y * 100.0);
	int32 N = 0;
	auto Line = [&](const UPrimitiveComponent* C, const FVector& Loc, const FVector& Scale, const FVector& Origin, const FVector& Extent, int32 Instance)
	{
		const UStaticMeshComponent* SM = Cast<UStaticMeshComponent>(C);
		const AActor* Owner = C->GetOwner();
		const UMaterialInterface* Mat = C->GetNumMaterials() > 0 ? C->GetMaterial(0) : nullptr;
		UE_LOG(LogHokej, Log, TEXT("JAMSCREEN nearpuck %s sim_frame=%lld comp=%s class=%s instance=%d owner=%s owner_class=%s level=%s parent=%s mesh=%s loc=(%.1f,%.1f,%.1f) scale=(%.3f,%.3f,%.3f) bounds_centre=(%.1f,%.1f,%.1f) half_extent=(%.1f,%.1f,%.1f) mat0=%s"),
			Why, F.SimFrame, *C->GetName(), *C->GetClass()->GetName(), Instance, Owner ? *Owner->GetName() : TEXT("none"), Owner ? *Owner->GetClass()->GetName() : TEXT("none"),
			Owner && Owner->GetLevel() ? *Owner->GetLevel()->GetOutermost()->GetName() : TEXT("none"), C->GetAttachParent() ? *C->GetAttachParent()->GetName() : TEXT("none"),
			SM && SM->GetStaticMesh() ? *SM->GetStaticMesh()->GetPathName() : TEXT("none"), Loc.X, Loc.Y, Loc.Z, Scale.X, Scale.Y, Scale.Z, Origin.X, Origin.Y, Origin.Z, Extent.X, Extent.Y, Extent.Z,
			Mat ? *Mat->GetPathName() : TEXT("none"));
		++N;
	};
	for (TObjectIterator<UPrimitiveComponent> It; It; ++It)
	{
		const UPrimitiveComponent* C = *It;
		if (C->GetWorld() != World || !C->IsRegistered() || !C->IsVisible()) continue;
		if (const UInstancedStaticMeshComponent* Ism = Cast<UInstancedStaticMeshComponent>(C))
		{
			for (int32 i = 0; i < Ism->GetInstanceCount(); ++i)
			{
				FTransform T;
				if (Ism->GetInstanceTransform(i, T, true) && FVector2D::Distance(FVector2D(T.GetLocation()), Puck) <= 150.0) Line(C, T.GetLocation(), T.GetScale3D(), T.GetLocation(), FVector::ZeroVector, i);
			}
			continue;
		}
		const FBoxSphereBounds B = C->Bounds;
		if (FVector2D::Distance(FVector2D(B.Origin), Puck) > 150.0) continue;
		if (B.BoxExtent.GetMax() > 1500.0) continue;   // the ice, the boards, the barn: centred near the dot by being the whole rink
		Line(C, C->GetComponentLocation(), C->GetComponentScale(), B.Origin, B.BoxExtent, -1);
	}
	UE_LOG(LogHokej, Log, TEXT("JAMSCREEN nearpuck %s sim_frame=%lld components=%d puck=(%.1f,%.1f) countdown=%d"), Why, F.SimFrame, N, Puck.X, Puck.Y, F.FaceoffCountdownFrames);
	const FString Tag = CVarJamHudDownStillTag.GetValueOnGameThread();
	FScreenshotRequest::RequestScreenshot(FString::Printf(TEXT("u1-post-%s-%s%sf%05lld-%s.png"), *Tier, *Tag, Tag.IsEmpty() ? TEXT("") : TEXT("-"), F.SimFrame, Why), true, false);
}

static void JamNearPuckTick(UWorld* World, const FJamPresentationFrame& F, const FString& Tier)
{
	static int64 LastFrame = -1, DropFrame = -1;
	static bool bHoldDone = false, bPlayDone = true;
	if (CVarJamHudNearPuck.GetValueOnGameThread() == 0 || !World) return;
	if (F.SimFrame < LastFrame) { DropFrame = -1; bHoldDone = false; bPlayDone = true; }
	LastFrame = F.SimFrame;
	if (F.FaceoffCountdownFrames > 0)
	{
		if (!bHoldDone && F.FaceoffCountdownFrames <= 20) { bHoldDone = true; JamDumpNearPuck(World, F, TEXT("hold"), Tier); }
		DropFrame = -1;
	}
	else
	{
		if (bHoldDone) { bHoldDone = false; DropFrame = F.SimFrame; bPlayDone = false; }
		if (!bPlayDone && DropFrame >= 0 && F.SimFrame - DropFrame >= 240 && F.Phase == EJamPhase::Play) { bPlayDone = true; JamDumpNearPuck(World, F, TEXT("play"), Tier); }
	}
}

// The probe. Called at the top of the tick, after this tick's boxes are known and before any widget is moved: every widget still stands where the last
// tick put it and its cached geometry is the last paint, so the last tick's figure boxes (Prev) are compared with what was drawn over them.
void UJamHudWidget::ProbeDown(const FJamPresentationFrame& F)
{
	if (CVarJamHudNearPuck.GetValueOnGameThread() != 0)
	{
		const AHokejGameMode* Mode = GetWorld() ? GetWorld()->GetAuthGameMode<AHokejGameMode>() : nullptr;
		JamNearPuckTick(GetWorld(), F, Mode && Mode->GetCartridge() ? Mode->GetCartridge()->Game.Look.Tier : FString(TEXT("tier")));
	}
	const int32 Every = CVarJamHudDownProbe.GetValueOnGameThread();
	if (Every <= 0) return;
	FDownProbe& P = DownProbe;
	if (F.SimFrame < P.PrevFrame) P = FDownProbe();   // a restart
	if (F.SimFrame == P.PrevFrame) return;             // one reading a sim frame seen
	const FVector2D VS = UWidgetLayoutLibrary::GetViewportSize(this);
	const float Scale = FMath::Max(UWidgetLayoutLibrary::GetViewportScale(this), 0.01f);
	const float VW = VS.X / Scale, VH = VS.Y / Scale;

	if (P.Prev.Num() || P.bPrevHumanUp)
	{
		// What is on the screen: every text, image, plate and nested widget of every widget in the viewport, shown all the way up its tree.
		struct FEl { FString Name; FBox2D Box; };
		TArray<FEl> Els;
		auto Shown = [](const UWidget* W)
		{
			for (const UWidget* X = W; X; X = X->GetParent()) if (!X->IsVisible() || X->GetRenderOpacity() <= 0.02f) return false;
			return true;
		};
		for (TObjectIterator<UUserWidget> It; It; ++It)
		{
			UUserWidget* U = *It;
			if (U->GetWorld() != GetWorld() || U->HasAnyFlags(RF_ClassDefaultObject) || !U->WidgetTree || !U->IsInViewport() || !U->IsVisible() || U->GetRenderOpacity() <= 0.02f) continue;
			const FString Prefix = U == this ? FString() : U->GetClass()->GetName() + TEXT(".");
			U->WidgetTree->ForEachWidget([&](UWidget* W)
			{
				const UTextBlock* T = Cast<UTextBlock>(W);
				if (!(T || W->IsA<UImage>() || W->IsA<UBorder>() || W->IsA<UUserWidget>())) return;
				if (T && T->GetText().IsEmpty()) return;
				if (!Shown(W)) return;
				FBox2D B(ForceInit);
				if (!PaintedBoxOnScreen(W, B)) return;
				if (B.GetSize().X * B.GetSize().Y > 0.4 * VW * VH) return;   // a full-screen layer (a fade, a backdrop) is not an element
				Els.Add({ U == this ? ProbeNameOf(W) : Prefix + W->GetName(), B });
			});
		}
		P.Elements = Els.Num();
		for (const FDownProbe::FPrev& D : P.Prev)
		{
			FDownRun* Run = P.Open.Find(D.Who);
			if (!Run) continue;
			++Run->Frames;
			Run->Source = D.Source;
			TSet<FString> Hit;
			bool bChevron = false;
			for (const FEl& E : Els)
			{
				if (!BoxesMeet(E.Box.Min, E.Box.Max, D.Box.Min, D.Box.Max)) continue;
				if (D.bHuman && (E.Name == TEXT("chevron") || E.Name == TEXT("you_tag"))) { bChevron = true; continue; }   // the human's marker stays (the brief)
				Hit.Add(E.Name);
			}
			if (bChevron) ++Run->ChevronFrames;
			if (Hit.Num()) ++Run->HitFrames;
			for (const FString& Name : Hit) ++Run->By.FindOrAdd(Name);
		}
		if (P.bPrevHumanUp)
		{
			++P.UpFrames;
			for (const FEl& E : Els) if (E.Name == TEXT("caption") && BoxesMeet(E.Box.Min, E.Box.Max, P.PrevHumanBox.Min, P.PrevHumanBox.Max)) { ++P.UpCaptionHit; break; }
		}
	}

	// Runs: one per knockdown; closed (and logged) on the first frame its skater is up again.
	auto ByText = [](const TMap<FString, int32>& By)
	{
		FString S;
		for (const TPair<FString, int32>& K : By) S += FString::Printf(TEXT("%s%s:%d"), S.IsEmpty() ? TEXT("") : TEXT(","), *K.Key, K.Value);
		return S.IsEmpty() ? FString(TEXT("none")) : S;
	};
	const AHokejGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AHokejGameMode>() : nullptr;
	const UJamSim* Sim = GM ? GM->GetSim() : nullptr;
	for (auto It = P.Open.CreateIterator(); It; ++It)
	{
		const int32 Who = It.Key();
		if (Sim && Sim->GetSkaters().IsValidIndex(Who) && Sim->GetSkaters()[Who].IsDown()) continue;
		const FDownRun& R = It.Value();
		const bool bHuman = Who == F.HumanIndex;
		if (R.Frames > 0)
		{
			UE_LOG(LogHokej, Log, TEXT("JAMSCREEN down who=%d human=%d start=%lld end=%lld seen=%d hit_frames=%d by=%s chevron_frames=%d box=%s rule=%d"),
				Who, bHuman ? 1 : 0, R.Start, F.SimFrame, R.Frames, R.HitFrames, *ByText(R.By), R.ChevronFrames, R.Source == 2 ? TEXT("bones") : TEXT("bounds"), ClearDownOn() ? 1 : 0);
			if (bHuman) { ++P.HumanDowns; P.HumanClean += R.HitFrames == 0 ? 1 : 0; P.HumanFrames += R.Frames; P.HumanHit += R.HitFrames; for (const TPair<FString, int32>& K : R.By) P.HumanBy.FindOrAdd(K.Key) += K.Value; }
			else { ++P.OtherDowns; P.OtherClean += R.HitFrames == 0 ? 1 : 0; P.OtherFrames += R.Frames; P.OtherHit += R.HitFrames; for (const TPair<FString, int32>& K : R.By) P.OtherBy.FindOrAdd(K.Key) += K.Value; }
		}
		It.RemoveCurrent();
	}
	P.Prev.Reset();
	for (int32 i = 0; i < DownBoxesNow.Num(); ++i)
	{
		const int32 Who = DownWhoNow[i];
		FDownRun& Run = P.Open.FindOrAdd(Who);
		if (Run.Start < 0) Run.Start = F.SimFrame;
		P.Prev.Add({ Who, Who == F.HumanIndex, DownBoxesNow[i], DownSourceNow[i] });
		// The proof's stills: N and 3 N frames into a knockdown of the human.
		if (const int32 StillAt = CVarJamHudDownStills.GetValueOnGameThread(); StillAt > 0 && Who == F.HumanIndex && GM && GM->GetCartridge())
		{
			const int64 Age = F.SimFrame - Run.Start;
			const int32 Want = Age >= 3 * StillAt ? 2 : Age >= StillAt ? 1 : 0;
			if (Want > Run.Stills)
			{
				Run.Stills = Want;
				const FString Tag = CVarJamHudDownStillTag.GetValueOnGameThread();
				FScreenshotRequest::RequestScreenshot(FString::Printf(TEXT("u1-down-%s-%s%sf%05lld-%d.png"), *GM->GetCartridge()->Game.Look.Tier, *Tag, Tag.IsEmpty() ? TEXT("") : TEXT("-"), Run.Start, Want), true, false);
			}
		}
	}
	P.bPrevHumanUp = bHasHumanBox && !bHumanDownNow;
	P.PrevHumanBox = HumanBoxNow;
	P.PrevFrame = F.SimFrame;
	if (F.SimFrame - P.LastLogged >= Every)
	{
		P.LastLogged = F.SimFrame;
		UE_LOG(LogHokej, Log, TEXT("JAMSCREEN downs sim_frame=%lld rule=%d human_knockdowns=%d human_clean=%d human_frames=%lld human_hit_frames=%lld human_by=%s other_knockdowns=%d other_clean=%d other_frames=%lld other_hit_frames=%lld other_by=%s up_frames=%lld up_caption_hits=%lld open=%d elements=%lld"),
			F.SimFrame, ClearDownOn() ? 1 : 0, P.HumanDowns, P.HumanClean, P.HumanFrames, P.HumanHit, *ByText(P.HumanBy), P.OtherDowns, P.OtherClean, P.OtherFrames, P.OtherHit, *ByText(P.OtherBy),
			P.UpFrames, P.UpCaptionHit, P.Open.Num(), P.Elements);
	}
}
