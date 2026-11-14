#include "HUDWidget.h"
#include "MenuStyle.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/PanelWidget.h"
#include "Components/OverlaySlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/SizeBox.h"
#include "GameFramework/PlayerController.h"
#include "Styling/CoreStyle.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "UObject/UObjectIterator.h"
#include "HokejGameMode.h"
#include "JamEffectSystem.h"
#include "JamCartridge.h"
#include "JamSim.h"
#include "CommentarySystem.h"
#include "Progression.h"
#include "Engine/GameInstance.h"
#include "Camera/PlayerCameraManager.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "Components/PrimitiveComponent.h"
#include "JamPawns.h"
#include "JamSkaterVisual.h"
#include "DirectorsCallWidget.h"
#include "InputMapping.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"

// Lane A84: the ability slot plate width (slate units at 1080p).
static constexpr float AbilitySlotW = 380.f;

int32 UJamHudWidget::FaceoffsThisSession = 0;
bool UJamHudWidget::bControlsHintSpent = false;
bool UJamHudWidget::bKeptPromptSpent = false;
bool UJamHudWidget::bBrakePromptSpent = false;
bool UJamHudWidget::bPadInUse = false;
uint64 UJamHudWidget::PromptBandTick = 0;
float UJamHudWidget::PromptBandTop = 0.f;

// Lane A123 (track U1, evolution 2; docs/experiments/SCREEN.md part 2). Each rule has its neutral value: the screen before this lane.
static TAutoConsoleVariable<int32> CVarJamHudMarkerAnchor(TEXT("jam.hud.MarkerAnchor"), 1,
	TEXT("The human's marker: 1 its tip on the figure's head (the visual's head_socket, else the top of its bounds) and a ghost while the puck is under it (default); 0 the point 1.8 m over the skater's feet."), ECVF_Default);
static TAutoConsoleVariable<float> CVarJamHudMarkerLiftCm(TEXT("jam.hud.MarkerLiftCm"), UJamHudWidget::YouMarkerHeadLiftCm,
	TEXT("Centimetres from a mesh's head_socket along the head's axis (bone to socket) to the marker's anchor: -18 (default) is the helmet's top of device B's figures; 0 the socket itself."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHudMarkerProbe(TEXT("jam.hud.MarkerProbe"), 0,
	TEXT("N > 0: a JAMSCREEN marker line every N sim frames (where the tip stood against the head, the old rule's distance, frames down). 0 off."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHudClearMiddle(TEXT("jam.hud.ClearMiddle"), 1,
	TEXT("No HUD box in the middle third of the frame during play (bark bubbles pushed out, the YOU tag at the faceoff count only, no net label there): 1 on (default), 0 as before."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHudCleanPlate(TEXT("jam.hud.CleanPlate"), 1,
	TEXT("The scoreboard's centre plate: 1 centred, PERIOD n over FIRST TO n when there is no timer (default); 0 one line at the plate's top."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHudPrompts(TEXT("jam.hud.Prompts"), 1,
	TEXT("The two one-time prompts of a session (the kept puck at the first carry, the brake when a defender first closes inside 10 m): 1 on (default), 0 none."), ECVF_Default);

bool UJamHudWidget::ClearMiddleOn() { return CVarJamHudClearMiddle.GetValueOnGameThread() != 0; }

// Lane A135 (track U1, evolution 3): the brake prompt in the player's own input.
static TAutoConsoleVariable<int32> CVarJamHudPromptKeys(TEXT("jam.hud.PromptKeys"), 1,
	TEXT("1 = the brake prompt names the controls bound to pulling back and to the brake, for the device last touched; 0 = the words of before (S AGAINST YOUR MOTION). Presentation only."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHudPromptPad(TEXT("jam.hud.PromptPad"), -1,
	TEXT("-1 = the prompts read the device the player last touched; 0 = the keyboard's words; 1 = the pad's (the proof's stills)."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHudPromptProof(TEXT("jam.hud.PromptProof"), 0,
	TEXT("N > 0: at sim frame N the brake prompt is shown in its lower slot (the proof's still of the slot; the remote console is not needed). Lane A135."), ECVF_Default);

// Lane A135, evolution 4: the prompt keeps off the attacked net (UJamHudWidget::PromptPlace).
static TAutoConsoleVariable<int32> CVarJamHudPromptOffNet(TEXT("jam.hud.PromptOffNet"), 1,
	TEXT("1 = the one-time prompt takes a place clear of the attacked net and its badge (the top row beside the net before the lower row); 0 = evolution 3's two slots (the lower one whenever the badge holds the top). Presentation only."), ECVF_Default);

bool UJamHudWidget::PromptBoxMeets(const FVector2D& Place, float VW, float HalfW, const FVector2D& NetMin, const FVector2D& NetMax, float Pad)
{
	const float Cx = VW * 0.5f + static_cast<float>(Place.X), Top = static_cast<float>(Place.Y);
	return !(Cx + HalfW < NetMin.X - Pad || Cx - HalfW > NetMax.X + Pad || Top + PromptSlotH < NetMin.Y - Pad || Top > NetMax.Y + Pad);
}

FVector2D UJamHudWidget::PromptPlace(float VW, float HalfW, bool bTopTaken, bool bHasNet, const FVector2D& NetMin, const FVector2D& NetMax, float Pad)
{
	// The centres first (they do not move with the net), then beside the net with room to spare (the net sweeps across the picture as he turns: the first
	// build put the prompt 16 units from it and the net pushed it along, frame after frame), and only then right beside it.
	const float Edge = 8.f;
	auto Clear = [&](const FVector2D& P, bool bTop)
	{
		if (bTop && bTopTaken) return false;
		const float Cx = VW * 0.5f + static_cast<float>(P.X);
		if (Cx - HalfW < Edge || Cx + HalfW > VW - Edge) return false;
		return !(bHasNet && PromptBoxMeets(P, VW, HalfW, NetMin, NetMax, Pad));
	};
	if (Clear(FVector2D(0, PromptSlotY), true)) return FVector2D(0, PromptSlotY);
	if (Clear(FVector2D(0, PromptSlotLowY), false)) return FVector2D(0, PromptSlotLowY);
	if (bHasNet)
	{
		for (float Gap : { 120.f, 16.f })
		{
			const float Left = static_cast<float>(NetMin.X) - Pad - Gap - HalfW - VW * 0.5f, Right = static_cast<float>(NetMax.X) + Pad + Gap + HalfW - VW * 0.5f;
			const float Near = FMath::Abs(Left) <= FMath::Abs(Right) ? Left : Right, Far = FMath::Abs(Left) <= FMath::Abs(Right) ? Right : Left;
			for (float Y : { PromptSlotY, PromptSlotLowY }) for (float X : { Near, Far })
				if (Clear(FVector2D(X, Y), Y == PromptSlotY)) return FVector2D(X, Y);
		}
	}
	return FVector2D(0, PromptSlotLowY);
}

// The attacked net on the screen: its mouth (2 m either side of its centre, from the ice to the crossbar) joined with its badge and distance label where
// they are drawn. False when neither is in the picture.
bool UJamHudWidget::NetBoxOnScreen(const FJamPresentationFrame& F, FVector2D& OutMin, FVector2D& OutMax) const
{
	bool bAny = false;
	OutMin = FVector2D(1e9, 1e9);
	OutMax = FVector2D(-1e9, -1e9);
	auto Add = [&](const FVector2D& P) { OutMin.X = FMath::Min(OutMin.X, P.X); OutMin.Y = FMath::Min(OutMin.Y, P.Y); OutMax.X = FMath::Max(OutMax.X, P.X); OutMax.Y = FMath::Max(OutMax.Y, P.Y); bAny = true; };
	const float VW = static_cast<float>(UWidgetLayoutLibrary::GetViewportSize(const_cast<UJamHudWidget*>(this)).X) / FMath::Max(UWidgetLayoutLibrary::GetViewportScale(const_cast<UJamHudWidget*>(this)), 0.01f);
	if (NetDiamond && NetDiamond->GetVisibility() != ESlateVisibility::Collapsed)
	{
		const FVector2D P = Marker.ScreenPos;
		const bool bLeft = P.X > VW - 200.f;
		const float LabelW = 12.f * Marker.Text.Len() + 2.f * LabelPlatePadX;
		Add(P + FVector2D(bLeft ? -24.f - LabelW : -14.f, -18.f));
		Add(P + FVector2D(bLeft ? 14.f : 24.f + LabelW, 18.f));
	}
	if (APlayerController* PC = GetOwningPlayer())
	{
		const FVector Net = F.GetFarNetWorldCm();
		FVector2D C[4];
		const FVector W[4] = { FVector(Net.X, Net.Y - 200.0, 0.0), FVector(Net.X, Net.Y + 200.0, 0.0), FVector(Net.X, Net.Y - 200.0, NetCrossbarCm), FVector(Net.X, Net.Y + 200.0, NetCrossbarCm) };
		bool bAll = true;
		for (int32 i = 0; i < 4; ++i) bAll &= UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, W[i], C[i], false);
		if (bAll) for (const FVector2D& P : C) Add(P);
	}
	return bAny;
}

FString UJamHudWidget::KeyLabel(const FKey& Key)
{
	if (Key == EKeys::LeftControl || Key == EKeys::RightControl) return TEXT("CTRL");
	if (Key == EKeys::LeftShift || Key == EKeys::RightShift) return TEXT("SHIFT");
	if (Key == EKeys::LeftAlt || Key == EKeys::RightAlt) return TEXT("ALT");
	if (Key == EKeys::SpaceBar) return TEXT("SPACE");
	if (Key == EKeys::Down) return TEXT("DOWN ARROW");
	if (Key == EKeys::Up) return TEXT("UP ARROW");
	if (Key == EKeys::Gamepad_Left2D || Key == EKeys::Gamepad_LeftX || Key == EKeys::Gamepad_LeftY) return TEXT("LEFT STICK");
	if (Key == EKeys::Gamepad_Right2D || Key == EKeys::Gamepad_RightX || Key == EKeys::Gamepad_RightY) return TEXT("RIGHT STICK");
	if (Key == EKeys::Gamepad_LeftThumbstick) return TEXT("L3");
	if (Key == EKeys::Gamepad_RightThumbstick) return TEXT("R3");
	if (Key == EKeys::Gamepad_FaceButton_Bottom) return TEXT("A");
	if (Key == EKeys::Gamepad_FaceButton_Right) return TEXT("B");
	if (Key == EKeys::Gamepad_FaceButton_Left) return TEXT("X");
	if (Key == EKeys::Gamepad_FaceButton_Top) return TEXT("Y");
	if (Key == EKeys::Gamepad_LeftShoulder) return TEXT("LB");
	if (Key == EKeys::Gamepad_RightShoulder) return TEXT("RB");
	if (Key == EKeys::Gamepad_LeftTrigger || Key == EKeys::Gamepad_LeftTriggerAxis) return TEXT("LT");
	if (Key == EKeys::Gamepad_RightTrigger || Key == EKeys::Gamepad_RightTriggerAxis) return TEXT("RT");
	return Key.IsValid() ? Key.GetDisplayName(false).ToString().ToUpper() : FString();
}

bool UJamHudWidget::BrakeKeysFor(bool bPad, FKey& OutBack, FKey& OutBrake)
{
	OutBack = FKey();
	OutBrake = FKey();
	const UJamInputMap* M = UJamInputMap::Get();
	if (!M || !M->Context) return false;
	for (const FEnhancedActionKeyMapping& K : M->Context->GetMappings())
	{
		if (K.Key.IsGamepadKey() != bPad) continue;
		if (K.Action.Get() == M->Brake.Get() && !OutBrake.IsValid()) OutBrake = K.Key;
		if (K.Action.Get() == M->Move.Get() && !OutBack.IsValid())
		{
			// A pad pulls back with the stick itself. A key pulls back when it is turned onto the forward axis and negated (W: swizzle; S: swizzle and negate).
			bool bSwizzle = false, bNegate = false;
			for (const UInputModifier* Mod : K.Modifiers)
			{
				if (Cast<UInputModifierSwizzleAxis>(Mod)) bSwizzle = true;
				if (Cast<UInputModifierNegate>(Mod)) bNegate = true;
			}
			if (bPad || (bSwizzle && bNegate)) OutBack = K.Key;
		}
	}
	return OutBack.IsValid();
}

FString UJamHudWidget::BrakePromptFor(const FKey& Back, const FKey& Brake)
{
	const FString B = KeyLabel(Back), K = KeyLabel(Brake);
	if (B.IsEmpty()) return PromptText(EPrompt::Brake);
	return K.IsEmpty() ? FString::Printf(TEXT("%s AGAINST YOUR MOTION: STOP, THEN GO"), *B) : FString::Printf(TEXT("%s AGAINST YOUR MOTION, OR HOLD %s: STOP, THEN GO"), *B, *K);
}

FString UJamHudWidget::BrakePromptText(bool bKeysOn, bool bPad)
{
	FKey Back, Brake;
	if (!bKeysOn || !BrakeKeysFor(bPad, Back, Brake)) return PromptText(EPrompt::Brake);
	return BrakePromptFor(Back, Brake);
}

FString UJamHudWidget::LivePromptText(EPrompt P)
{
	if (P != EPrompt::Brake) return PromptText(P);
	const int32 Forced = CVarJamHudPromptPad.GetValueOnGameThread();
	return BrakePromptText(CVarJamHudPromptKeys.GetValueOnGameThread() != 0, Forced < 0 ? bPadInUse : Forced != 0);
}

// The device last touched: any of the pad's buttons or a stick past a third of its throw makes it the pad, a game key or a mouse button the keyboard.
void UJamHudWidget::WatchInputDevice()
{
	const APlayerController* PC = GetOwningPlayer();
	if (!PC) return;
	static const FKey PadKeys[] = { EKeys::Gamepad_FaceButton_Bottom, EKeys::Gamepad_FaceButton_Right, EKeys::Gamepad_FaceButton_Left, EKeys::Gamepad_FaceButton_Top,
		EKeys::Gamepad_LeftShoulder, EKeys::Gamepad_RightShoulder, EKeys::Gamepad_LeftTrigger, EKeys::Gamepad_RightTrigger, EKeys::Gamepad_LeftThumbstick,
		EKeys::Gamepad_RightThumbstick, EKeys::Gamepad_DPad_Up, EKeys::Gamepad_DPad_Down, EKeys::Gamepad_DPad_Left, EKeys::Gamepad_DPad_Right, EKeys::Gamepad_Special_Left, EKeys::Gamepad_Special_Right };
	static const FKey PadAxes[] = { EKeys::Gamepad_LeftX, EKeys::Gamepad_LeftY, EKeys::Gamepad_RightX, EKeys::Gamepad_RightY, EKeys::Gamepad_LeftTriggerAxis, EKeys::Gamepad_RightTriggerAxis };
	static const FKey DeskKeys[] = { EKeys::W, EKeys::A, EKeys::S, EKeys::D, EKeys::Up, EKeys::Down, EKeys::Left, EKeys::Right, EKeys::LeftControl, EKeys::LeftShift, EKeys::SpaceBar,
		EKeys::Q, EKeys::E, EKeys::F, EKeys::LeftMouseButton, EKeys::RightMouseButton, EKeys::MiddleMouseButton };
	for (const FKey& K : PadKeys) if (PC->IsInputKeyDown(K)) { bPadInUse = true; return; }
	for (const FKey& K : PadAxes) if (FMath::Abs(PC->GetInputAnalogKeyState(K)) > 0.33f) { bPadInUse = true; return; }
	for (const FKey& K : DeskKeys) if (PC->IsInputKeyDown(K)) { bPadInUse = false; return; }
}

bool UJamHudWidget::GetPromptBandPx(float& OutTop, float& OutBottom)
{
	if (PromptBandTick == 0 || GFrameCounter > PromptBandTick + 1) return false;
	OutTop = PromptBandTop - 4.f;
	OutBottom = PromptBandTop + PromptSlotH + 4.f;
	return true;
}

// Lane A111 (track U1, the screen; docs/experiments/SCREEN.md). Each rule has its neutral value: the screen before this lane.
static TAutoConsoleVariable<int32> CVarJamHudYouMarker(TEXT("jam.hud.YouMarker"), 1,
	TEXT("The yellow chevron over the human's helmet for the whole match, the YOU tag above it: 1 on (default), 0 lane A42's rule (chevron past 20 m only, the tag in the ring)."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHudHintEveryMatch(TEXT("jam.hud.HintEveryMatch"), 0,
	TEXT("The controls strip: 0 the first match of a session only, until its 8 s run out or the first shot (default); 1 in every match."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHudDockCommentary(TEXT("jam.hud.DockCommentary"), 1,
	TEXT("The commentator's strip: 1 docked on the bottom edge (default), 0 at 110 px over it (the earlier place)."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHudLabelPlates(TEXT("jam.hud.LabelPlates"), 1,
	TEXT("The distance labels: 1 on a dark plate and the net badges hidden during the goal pause (default), 0 bare outlined type, always drawn."), ECVF_Default);

// Lane A74 (R-0038): the off-screen puck marker, 0 = off, 1 = on (default).
// Lane A76: 1 = HUD pass 3 (default), 0 = the A74 layout, for the before column of the proof (same package).
static TAutoConsoleVariable<int32> CVarJamHudPass3(TEXT("jam.hud.Pass3"), 1, TEXT("HUD pass 3 (lifted far-net badge, edge slots, rink-sized MATCH card): 1 on, 0 the A74 layout."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHudPuckMarker(TEXT("jam.hud.PuckMarker"), 1,
	TEXT("Edge arrow with the distance pointing at the puck (or its carrier) when it is outside the inner 85 % of the frame: 0 off, 1 on."), ECVF_Default);

// Lane A18 (visual layer, R-0013 point 4): the slate kit of FJamUiStyle over lane A5's elements. Logic (markers, frames, feed) is A5's;
// the effects strip is lane A14's feed drawn as a slot row; the faceoff countdown reads lane A16's FaceoffCountdownFrames when it lands.
namespace
{
	using S = FJamUiStyle;

	// The feed field lane A16 adds; until it is there the countdown is driven by the preview only.
	template <typename T>
	int32 FaceoffFramesOf(const T& F)
	{
		if constexpr (requires { F.FaceoffCountdownFrames; }) return static_cast<int32>(F.FaceoffCountdownFrames);
		else return 0;
	}

	UTextBlock* MakeText(UWidgetTree* Tree, const TCHAR* Name, const FSlateFontInfo& Font, const FLinearColor& Colour)
	{
		UTextBlock* T = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
		T->SetFont(Font);
		T->SetColorAndOpacity(FSlateColor(Colour));
		T->SetVisibility(ESlateVisibility::HitTestInvisible);
		return T;
	}

	UCanvasPanelSlot* Place(UCanvasPanel* Canvas, UWidget* W, FVector2D Anchor, FVector2D Align, FVector2D Pos, FVector2D Size = FVector2D::ZeroVector)
	{
		UCanvasPanelSlot* Slot = Canvas->AddChildToCanvas(W);
		Slot->SetAnchors(FAnchors(Anchor.X, Anchor.Y, Anchor.X, Anchor.Y));
		Slot->SetAlignment(Align);
		Slot->SetPosition(Pos);
		if (Size.IsZero())
		{
			Slot->SetAutoSize(true);
		}
		else
		{
			Slot->SetAutoSize(false);
			Slot->SetSize(Size);
		}
		return Slot;
	}

	// A diamond marker: a rounded box at 45 degrees with an ink outline so it reads over white ice and black void alike.
	UBorder* MakeDiamond(UWidgetTree* Tree, const TCHAR* Name, const FLinearColor& Fill, float Outline)
	{
		UBorder* B = Tree->ConstructWidget<UBorder>(UBorder::StaticClass(), Name);
		B->SetBrush(S::Rounded(Fill, 3.f, FLinearColor(0, 0, 0, 0.9f), Outline));
		B->SetRenderTransformAngle(45.f);
		B->SetRenderTransformPivot(FVector2D(0.5, 0.5));
		B->SetVisibility(ESlateVisibility::HitTestInvisible);
		return B;
	}

	// Lane A42: a chevron (two thin bars meeting at the bottom, a "V" pointing down) in a colour with an ink edge; the caller rotates it
	// to point another way. Size is the box side in Slate units.
	UWidget* MakeChevron(UWidgetTree* Tree, const TCHAR* Name, const FLinearColor& Colour, float Size)
	{
		USizeBox* Box = Tree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), Name);
		Box->SetWidthOverride(Size);
		Box->SetHeightOverride(Size);
		UOverlay* O = Tree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
		const float BarW = FMath::Max(3.f, Size * 0.2f), BarH = Size * 0.78f;
		for (int32 i = 0; i < 2; ++i)
		{
			UBorder* Bar = Tree->ConstructWidget<UBorder>(UBorder::StaticClass());
			Bar->SetBrush(S::Rounded(Colour, BarW * 0.5f, FLinearColor(0, 0, 0, 0.9f), 1.5f));
			Bar->SetPadding(FMargin(BarW * 0.5f, BarH * 0.5f));
			Bar->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
			// Each bar leans 40 degrees and slides so the two lower ends meet at the bottom centre (half the bar's height times sin 40).
			Bar->SetRenderTransformAngle(i == 0 ? -40.f : 40.f);
			Bar->SetRenderTranslation(FVector2D((i == 0 ? -1.f : 1.f) * BarH * 0.5f * 0.643f, 0.f));
			Bar->SetVisibility(ESlateVisibility::HitTestInvisible);
			S::AddO(O, Bar, HAlign_Center, VAlign_Center);
		}
		Box->AddChild(O);
		Box->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
		Box->SetVisibility(ESlateVisibility::HitTestInvisible);
		return Box;
	}
	// The ground ring: a plain chalk ring (T_UI_RingPlain, A48's storyboard finding 2: the countdown ring's cross hairs read as a reticle on
	// the ice), the countdown ring without it, a rounded outline without any art; tinted, drawn as the ellipse the caller sizes.
	UImage* MakeGroundRing(UWidgetTree* Tree, const TCHAR* Name, const FLinearColor& Colour)
	{
		UImage* I = Tree->ConstructWidget<UImage>(UImage::StaticClass(), Name);
		if (UTexture2D* Plain = S::Art(TEXT("T_UI_RingPlain"))) I->SetBrush(S::ImageBrush(Plain, FVector2D(64.f, 64.f)));
		else I->SetBrush(S::Ring(64.f));
		I->SetColorAndOpacity(Colour);
		I->SetVisibility(ESlateVisibility::Collapsed);
		return I;
	}

	// Lane A111: jam.ui.hintreset makes the controls strip due again (the proof's "first match of a session" without a new process).
	FAutoConsoleCommand GHintReset(TEXT("jam.ui.hintreset"), TEXT("The controls hint counts as not yet shown this session."),
		FConsoleCommandDelegate::CreateLambda([]() { UJamHudWidget::SetControlsHintSpent(false); }));
	// Lane A123: the cartridge's stick.passive_take where the tree has the key (R-0048, lane A102); false (the puck is kept) where it has not.
	template <typename T>
	bool PassiveTakeOf(const T& Stick)
	{
		if constexpr (requires { Stick.bPassiveTake; }) return Stick.bPassiveTake;
		else return false;
	}
	// Lane A123: jam.ui.promptreset makes both one-time prompts due again; jam.ui.prompt kept|brake shows one now (the proof's stills).
	FAutoConsoleCommand GPromptReset(TEXT("jam.ui.promptreset"), TEXT("The two one-time prompts count as not yet shown this session."),
		FConsoleCommandDelegate::CreateLambda([]() { UJamHudWidget::SetPromptsSpent(false, false); }));
	FAutoConsoleCommand GPromptPreview(TEXT("jam.ui.prompt"), TEXT("Show a one-time prompt now on every HUD: jam.ui.prompt kept|brake"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			const UJamHudWidget::EPrompt P = Args.Num() && Args[0].Equals(TEXT("brake"), ESearchCase::IgnoreCase) ? UJamHudWidget::EPrompt::Brake : UJamHudWidget::EPrompt::Kept;
			const bool bLow = Args.Num() > 1 && Args[1].Equals(TEXT("low"), ESearchCase::IgnoreCase);   // lane A135: jam.ui.prompt brake low = in the lower slot
			for (TObjectIterator<UJamHudWidget> It; It; ++It) if (It->GetWorld() && !It->HasAnyFlags(RF_ClassDefaultObject)) It->PreviewPrompt(P, bLow);
		}));
	// QA previews for the proof shots (jam.ui.faceoff <frames>, jam.ui.effects icon:label,icon:label).
	FAutoConsoleCommand GPreviewFaceoff(TEXT("jam.ui.faceoff"), TEXT("Preview the faceoff countdown on every HUD: jam.ui.faceoff <frames>"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			const int32 Frames = Args.Num() ? FCString::Atoi(*Args[0]) : 90;
			for (TObjectIterator<UJamHudWidget> It; It; ++It) if (It->GetWorld() && !It->HasAnyFlags(RF_ClassDefaultObject)) It->PreviewFaceoff(Frames);
		}));
	// Lane A42: jam.ui.draw won|lost [frames] previews the draw result in the ring (the live one lasts 40 frames: too short for a capture call).
	FAutoConsoleCommand GPreviewDraw(TEXT("jam.ui.draw"), TEXT("Preview the faceoff draw result on every HUD: jam.ui.draw won|lost [frames]"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			const bool bWon = Args.Num() == 0 || Args[0].Equals(TEXT("won"), ESearchCase::IgnoreCase);
			const int32 Frames = Args.Num() > 1 ? FCString::Atoi(*Args[1]) : UJamHudWidget::DrawResultFrames;
			for (TObjectIterator<UJamHudWidget> It; It; ++It) if (It->GetWorld() && !It->HasAnyFlags(RF_ClassDefaultObject)) It->PreviewDraw(bWon, Frames);
		}));
	FAutoConsoleCommand GPreviewEffects(TEXT("jam.ui.effects"), TEXT("Preview the effect slots on every HUD: jam.ui.effects pickup-speed:speed,pickup-heavy:heavy (empty clears)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			TArray<TPair<FString, FString>> Slots;
			TArray<FString> Items;
			if (Args.Num()) Args[0].ParseIntoArray(Items, TEXT(","));
			for (const FString& I : Items)
			{
				FString Icon, Label;
				if (!I.Split(TEXT(":"), &Icon, &Label)) { Icon = I; Label = I; }
				Slots.Emplace(Icon, Label);
			}
			for (TObjectIterator<UJamHudWidget> It; It; ++It) if (It->GetWorld() && !It->HasAnyFlags(RF_ClassDefaultObject)) It->SetEffectSlots(Slots);
		}));
}

UJamHudWidget* UJamHudWidget::CreateAndAdd(APlayerController* PC)
{
	if (!PC)
	{
		return nullptr;
	}
	UJamHudWidget* W = CreateWidget<UJamHudWidget>(PC, UJamHudWidget::StaticClass());
	if (W)
	{
		W->AddToViewport(0);
	}
	return W;
}

FString UJamHudWidget::FormatTimer(int32 FramesRemaining)
{
	const int32 Secs = FMath::Max(0, FMath::DivideAndRoundUp(FramesRemaining, 60));
	return FString::Printf(TEXT("%d:%02d"), Secs / 60, Secs % 60);
}

FString UJamHudWidget::FormatTake(int32 Take, int32 ScoreToWin)
{
	if (ScoreToWin > 0)
	{
		return FString::Printf(TEXT("PERIOD %d: first to %d"), FMath::Max(1, Take), ScoreToWin);
	}
	return FString::Printf(TEXT("PERIOD %d"), FMath::Max(1, Take));
}

FString UJamHudWidget::CardFor(EJamPhase Phase)
{
	switch (Phase)
	{
	case EJamPhase::Win: return TEXT("YOU WIN");
	case EJamPhase::Lose: return TEXT("REMATCH?");
	case EJamPhase::Paused: return TEXT("PAUSED");
	default: return FString();
	}
}

TSharedRef<SWidget> UJamHudWidget::RebuildWidget()
{
	BuildTree();
	return Super::RebuildWidget();
}

void UJamHudWidget::BuildTree()
{
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	}
	if (WidgetTree->RootWidget)
	{
		return;
	}
	UWidgetTree* T = WidgetTree;
	Root = T->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
	T->RootWidget = Root;

	// ---- Scoreboard: [home plate] [take over timer on a dark panel] [away plate], top centre.
	UHorizontalBox* Board = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Scoreboard"));
	Board->SetVisibility(ESlateVisibility::HitTestInvisible);

	HomeBox = T->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("HomeBox"));
	HomeBox->SetBrush(S::ScorePlate(FLinearColor::White));
	HomeBox->SetBrushColor(S::Blue());
	HomeBox->SetPadding(FMargin(22, 2, 22, 4));
	HomeBox->SetHorizontalAlignment(HAlign_Center);
	ScoreHome = MakeText(T, TEXT("ScoreHome"), S::ScoreFont(56), S::Paper());
	ScoreHome->SetMinDesiredWidth(64.f);
	ScoreHome->SetJustification(ETextJustify::Center);
	HomeBox->SetContent(ScoreHome);
	HomeBox->SetVisibility(ESlateVisibility::HitTestInvisible);

	AwayBox = T->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("AwayBox"));
	AwayBox->SetBrush(S::ScorePlate(FLinearColor::White));
	AwayBox->SetBrushColor(S::Orange());
	AwayBox->SetPadding(FMargin(22, 2, 22, 4));
	AwayBox->SetHorizontalAlignment(HAlign_Center);
	ScoreAway = MakeText(T, TEXT("ScoreAway"), S::ScoreFont(56), S::Paper());
	ScoreAway->SetMinDesiredWidth(64.f);
	ScoreAway->SetJustification(ETextJustify::Center);
	AwayBox->SetContent(ScoreAway);
	AwayBox->SetVisibility(ESlateVisibility::HitTestInvisible);

	// Wide side padding: the tracked capitals measure narrower than they draw, and the panel's corner tiles are 28 px.
	UBorder* Centre = S::Plate(T, S::Panel(0.9f), FMargin(34, 8, 34, 8));
	Centre->SetVisibility(ESlateVisibility::HitTestInvisible);
	UVerticalBox* CentreBox = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	TakeText = MakeText(T, TEXT("TakeText"), S::Font(S::Caption + 2, FName("Bold"), 30), S::Yellow());
	TakeText->SetJustification(ETextJustify::Center);
	S::AddV(CentreBox, TakeText, FMargin(0, 0, 0, 0), HAlign_Center);
	// Lane A123: with no timer the plate held one caption on its top border over an empty panel; the rule ("FIRST TO 3") is the second line, the two centred.
	GoalRuleText = MakeText(T, TEXT("GoalRuleText"), S::Font(S::Label - 2, FName("Black"), 30), S::Paper());
	GoalRuleText->SetJustification(ETextJustify::Center);
	GoalRuleText->SetVisibility(ESlateVisibility::Collapsed);
	S::AddV(CentreBox, GoalRuleText, FMargin(0, -2, 0, 0), HAlign_Center);
	CentrePlate = Centre;
	TimerText = MakeText(T, TEXT("TimerText"), S::ScoreFont(34, false), S::Paper());
	TimerText->SetJustification(ETextJustify::Center);
	S::AddV(CentreBox, TimerText, FMargin(0, -2, 0, 0), HAlign_Center);
	Centre->SetContent(CentreBox);

	S::AddH(Board, HomeBox, FMargin(0, 0, 6, 0), false, VAlign_Center);
	S::AddH(Board, Centre, FMargin(0), false, VAlign_Fill);
	S::AddH(Board, AwayBox, FMargin(6, 0, 0, 0), false, VAlign_Center);
	Place(Root, Board, FVector2D(0.5, 0), FVector2D(0.5, 0), FVector2D(0, 14));

	// ---- Lane A14's active effects, drawn as a slot row (icon + label each) on a panel under the scoreboard.
	EffectsPlate = S::Plate(T, S::Panel(0.8f), FMargin(12, 6));
	EffectsPlate->SetVisibility(ESlateVisibility::Collapsed);
	EffectsRow = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("EffectsRow"));
	EffectsPlate->SetContent(EffectsRow);
	// Under the scoreboard line's slot (lane A8's overlay pill at y 112).
	Place(Root, EffectsPlate, FVector2D(0.5, 0), FVector2D(0.5, 0), FVector2D(0, 158));
	EffectsText = MakeText(T, TEXT("EffectsText"), S::CaptionFont(true), S::Yellow());
	EffectsText->SetVisibility(ESlateVisibility::Collapsed);
	Place(Root, EffectsText, FVector2D(0.5, 0), FVector2D(0.5, 0), FVector2D(0, 158));

	// ---- Lane A42: the rings on the ice go first so every other element draws over them.
	YouRing = MakeGroundRing(T, TEXT("YouRing"), S::Blue());
	Place(Root, YouRing, FVector2D(0, 0), FVector2D(0.5, 0.5), FVector2D(0, 0), FVector2D(64, 20));
	CarrierRing = MakeGroundRing(T, TEXT("CarrierRing"), S::Paper());
	Place(Root, CarrierRing, FVector2D(0, 0), FVector2D(0.5, 0.5), FVector2D(0, 0), FVector2D(48, 16));

	// ---- Far-net marker: a take-yellow diamond with an ink edge and the distance in outlined type.
	NetDiamond = MakeDiamond(T, TEXT("NetDiamond"), S::Yellow(), 2.f);
	Place(Root, NetDiamond, FVector2D(0, 0), FVector2D(0.5, 0.5), FVector2D(0, 0), FVector2D(22, 22));
	// Lane A111: each distance label stands on a dark plate (added before its label, so it draws under it); UpdateLabelPlates sizes and places it.
	auto MakeLabelPlate = [&](const TCHAR* Name) -> UBorder*
	{
		UBorder* B = T->ConstructWidget<UBorder>(UBorder::StaticClass(), Name);
		B->SetBrush(S::Rounded(FLinearColor(0.f, 0.f, 0.f, LabelPlateOpacity), 6.f));
		B->SetVisibility(ESlateVisibility::Collapsed);
		Place(Root, B, FVector2D(0, 0), FVector2D(0.5, 0.5), FVector2D(0, 0), FVector2D(40, 20));
		return B;
	};
	NetPlate = MakeLabelPlate(TEXT("NetPlate"));
	NetText = MakeText(T, TEXT("NetText"), S::LabelFont(true), S::Paper());
	Place(Root, NetText, FVector2D(0, 0), FVector2D(0.5, 0), FVector2D(0, 0));
	NetArrow = MakeChevron(T, TEXT("NetArrow"), S::Yellow(), 22.f);
	NetArrow->SetVisibility(ESlateVisibility::Collapsed);
	Place(Root, NetArrow, FVector2D(0, 0), FVector2D(0.5, 0.5), FVector2D(0, 0));

	// ---- Own-net marker: smaller and muted, so it never competes with the far-net one.
	OwnDiamond = MakeDiamond(T, TEXT("OwnDiamond"), S::Muted(), 1.5f);
	Place(Root, OwnDiamond, FVector2D(0, 0), FVector2D(0.5, 0.5), FVector2D(0, 0), FVector2D(12, 12));
	OwnPlate = MakeLabelPlate(TEXT("OwnPlate"));
	// Lane A111: 16 px bold paper on its plate (was 14 px medium in the muted grey: B's play-through could not read it on clay); the diamond stays muted.
	OwnText = MakeText(T, TEXT("OwnText"), S::Font(S::Body - 2, FName("Bold"), 20, true), S::Paper());
	Place(Root, OwnText, FVector2D(0, 0), FVector2D(0.5, 0), FVector2D(0, 0));
	OwnArrow = MakeChevron(T, TEXT("OwnArrow"), S::Muted(), 16.f);
	OwnArrow->SetVisibility(ESlateVisibility::Collapsed);
	Place(Root, OwnArrow, FVector2D(0, 0), FVector2D(0.5, 0.5), FVector2D(0, 0));

	// ---- Lane A74: the off-screen puck marker (arrow on the frame's edge and the distance beside it); drawn over the net markers.
	PuckArrow = MakeChevron(T, TEXT("PuckArrow"), FLinearColor::White, 28.f);
	PuckArrow->SetVisibility(ESlateVisibility::Collapsed);
	Place(Root, PuckArrow, FVector2D(0, 0), FVector2D(0.5, 0.5), FVector2D(0, 0));
	PuckPlate = MakeLabelPlate(TEXT("PuckPlate"));
	PuckText = MakeText(T, TEXT("PuckText"), S::LabelFont(true), S::Paper());
	PuckText->SetVisibility(ESlateVisibility::Collapsed);
	Place(Root, PuckText, FVector2D(0, 0), FVector2D(0.5, 0.5), FVector2D(0, 0));

	// ---- Lane A42: the chevron over the human's head when the ring has faded (past 24 m), and the YOU tag above it.
	// Lane A111: the chevron is the human's marker for the whole match (take-yellow, YouMarkerSizePx); UpdateYours colours it.
	YouChevron = MakeChevron(T, TEXT("YouChevron"), S::Yellow(), YouMarkerSizePx);
	YouChevron->SetVisibility(ESlateVisibility::Collapsed);
	Place(Root, YouChevron, FVector2D(0, 0), FVector2D(0.5, 1), FVector2D(0, 0));
	YouTag = S::Plate(T, S::Rounded(S::Blue(), 8.f, FLinearColor(0, 0, 0, 0.9f), 2.f), FMargin(12, 3, 12, 4));
	YouTag->SetVisibility(ESlateVisibility::Collapsed);
	YouTagText = MakeText(T, TEXT("YouTagText"), S::Font(S::Label - 2, FName("Black"), S::TrackLabel, true), S::Paper());
	YouTagText->SetText(FText::FromString(TEXT("YOU")));
	YouTag->SetContent(YouTagText);
	Place(Root, YouTag, FVector2D(0, 0), FVector2D(0.5, 1), FVector2D(0, 0));

	// ---- Lane A49: the sprint stamina bar (Shift / LT): 240 x 8 px, bottom centre above the strips; drains in sprint.drain_frames, refills in refill_frames.
	StaminaBack = S::Plate(T, FLinearColor(0.f, 0.f, 0.f, 0.55f), FMargin(0));
	StaminaBack->SetVisibility(ESlateVisibility::Collapsed);
	Place(Root, StaminaBack, FVector2D(0.5, 1), FVector2D(0.5, 1), FVector2D(0, -168), FVector2D(244, 12));
	StaminaFill = S::Plate(T, S::Chalk(), FMargin(0));
	StaminaFill->SetVisibility(ESlateVisibility::Collapsed);
	Place(Root, StaminaFill, FVector2D(0.5, 1), FVector2D(0, 1), FVector2D(-120, -170), FVector2D(240, 8));

	// ---- Lane A84 (R-0041 point 3): the ability slot above the radar frame, right-aligned with it: "F  SHOTGUN  READY" on a dark plate, the cooldown fill
	// along its foot. Hidden while the human holds no ability. 380 wide: the package still clipped "F  SHOTGUN  ARMED" at 240 (Label 24 needs about 330). (First placed right of the stamina bar: the package still showed the commentary
	// strip's line over its "F"; the corner above the radar is free of the strip, the controls hint and the commentary.)
	AbilityPlate = S::Plate(T, FLinearColor(0.f, 0.f, 0.f, 0.62f), FMargin(10, 4, 10, 8));
	AbilityPlate->SetVisibility(ESlateVisibility::Collapsed);
	AbilityText = MakeText(T, TEXT("AbilityText"), S::LabelFont(true), S::Paper());
	AbilityPlate->SetContent(AbilityText);
	Place(Root, AbilityPlate, FVector2D(1, 1), FVector2D(1, 1), FVector2D(-S::EdgePx, -(S::EdgePx + UJamRadarWidget::RadarHeightPx + 30.f)), FVector2D(AbilitySlotW, 34));
	AbilityFill = S::Plate(T, S::Chalk(), FMargin(0));
	AbilityFill->SetVisibility(ESlateVisibility::Collapsed);
	Place(Root, AbilityFill, FVector2D(1, 1), FVector2D(0, 1), FVector2D(-S::EdgePx - AbilitySlotW + 10.f, -(S::EdgePx + UJamRadarWidget::RadarHeightPx + 32.f)), FVector2D(AbilitySlotW - 20.f, 4));

	// ---- Controls hint: a bottom strip for the first 10 s, fading out over the last 2 s.
	ControlsStrip = S::Plate(T, S::Pill(0.72f, 0.f), FMargin(0, 8));
	ControlsStrip->SetVisibility(ESlateVisibility::Collapsed);
	ControlsStrip->SetHorizontalAlignment(HAlign_Center);
	// Lane A64 (eyes grid h19 finding 2, 53 stills): the strip ran under the radar frame and "PAUSE ESC" printed into it. The strip now ends ControlsRightInsetPx
	// short of the right edge (the radar frame, 216 px, and a 16 px gap) and the type is 2 pt smaller with 4-space gaps, so the line fits that width
	// at 1920 x 1080 and 1280 x 720 (16:9 keeps the same layout units).
	ControlsText = MakeText(T, TEXT("ControlsText"), S::Font(S::Body - 2, FName("Bold"), 20), S::Paper());
	// Lane A111 (REVIEW-1 item 8, B-to-A-0038 fault 6): seven verbs with the brake among them (was ten and no brake); the pause card keeps the full table
	// (UJamInputMap::BindingTableText), the ability has its own slot, ESC its own tag.
	ControlsText->SetText(FText::FromString(ControlsHintText()));
	ControlsText->SetJustification(ETextJustify::Center);
	ControlsStrip->SetContent(ControlsText);
	UCanvasPanelSlot* StripSlot = Root->AddChildToCanvas(ControlsStrip);
	StripSlot->SetAnchors(FAnchors(0, 1, 1, 1));
	StripSlot->SetAlignment(FVector2D(0, 1));
	StripSlot->SetOffsets(FMargin(S::EdgePx, -96, ControlsRightInsetPx, 44));

	// ---- Lane A64: a notice plate top-left (a rejected -cut cartridge), 4 s.
	ToastPlate = S::Plate(T, S::Pill(0.86f), FMargin(16, 6));
	ToastPlate->SetVisibility(ESlateVisibility::Collapsed);
	ToastText = MakeText(T, TEXT("ToastText"), S::Font(S::Body, FName("Bold"), 20), S::Yellow());
	ToastText->SetAutoWrapText(true);
	ToastText->SetWrapTextAt(560.f);
	ToastPlate->SetContent(ToastText);
	Place(Root, ToastPlate, FVector2D(0, 0), FVector2D(0, 0), FVector2D(S::EdgePx, S::EdgePx));
	// ---- Lane A101 (track F1): the contact line, bottom centre above the stamina bar: NOT WITH THE PUCK (a refused press), BOUNCED OFF (the human's own recoil).
	ContactPlate = S::Plate(T, S::Pill(0.86f), FMargin(18, 5));
	ContactPlate->SetVisibility(ESlateVisibility::Collapsed);
	ContactText = MakeText(T, TEXT("ContactText"), S::Font(S::Body, FName("Bold"), 22), S::Yellow());
	ContactPlate->SetContent(ContactText);
	Place(Root, ContactPlate, FVector2D(0.5, 1), FVector2D(0.5, 1), FVector2D(0, -192));

	// ---- Lane A123: the one-time prompt, a pill under the scoreboard (over the stands and the far boards, never over the play), in the HUD's yellow.
	PromptPlate = S::Plate(T, S::Pill(0.86f), FMargin(22, 8));
	PromptPlate->SetVisibility(ESlateVisibility::Collapsed);
	PromptTextBlock = MakeText(T, TEXT("PromptText"), S::Font(S::Body + 2, FName("Black"), 30), S::Yellow());
	PromptTextBlock->SetJustification(ETextJustify::Center);
	PromptPlate->SetContent(PromptTextBlock);
	Place(Root, PromptPlate, FVector2D(0.5, 0), FVector2D(0.5, 0), FVector2D(0, PromptSlotY));

	// ---- Lane A42: the commentator's strip, bottom centre, above the controls strip; two lines at most.
	CommentaryStrip = S::Plate(T, S::Pill(0.82f), FMargin(18, 7));
	CommentaryStrip->SetVisibility(ESlateVisibility::Collapsed);
	CommentaryText = MakeText(T, TEXT("CommentaryText"), S::Font(S::Body, FName("Bold")), S::Paper());
	CommentaryText->SetJustification(ETextJustify::Center);
	// A fixed wrap width (auto-wrap in an auto-sized slot wraps every word).
	CommentaryText->SetAutoWrapText(false);
	CommentaryText->SetWrapTextAt(880.f);
	CommentaryStrip->SetContent(CommentaryText);
	Place(Root, CommentaryStrip, FVector2D(0.5, 1), FVector2D(0.5, 1), FVector2D(0, -110));

	// ---- Radar 200 x 100 on a framed panel, bottom-right.
	Radar = T->ConstructWidget<UJamRadarWidget>(UJamRadarWidget::StaticClass(), TEXT("Radar"));
	UBorder* RadarFrame = S::Plate(T, S::Panel(0.85f), FMargin(8));
	RadarFrame->SetVisibility(ESlateVisibility::HitTestInvisible);
	USizeBox* RadarSize = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	RadarSize->SetWidthOverride(UJamRadarWidget::RadarWidthPx);
	RadarSize->SetHeightOverride(UJamRadarWidget::RadarHeightPx);
	RadarSize->AddChild(Radar);
	RadarFrame->SetContent(RadarSize);
	Place(Root, RadarFrame, FVector2D(1, 1), FVector2D(1, 1), FVector2D(-S::EdgePx, -S::EdgePx));

	// ---- Director's Call ready line (lane A5's; the overlay of lane A8 draws the live hint, this one stays for its feed flag).
	CallText = MakeText(T, TEXT("CallText"), S::LabelFont(true), S::Yellow());
	CallText->SetText(FText::FromString(TEXT("CALL READY")));   // R-0021 point 3 (lane A47, B-to-A-0030 fault 2): no film word on the ice
	CallText->SetVisibility(ESlateVisibility::Collapsed);
	Place(Root, CallText, FVector2D(0.5, 1), FVector2D(0.5, 1), FVector2D(0, -40));

	// ---- Faceoff countdown: the chalk ring with the seconds inside, "DROP" at zero.
	UOverlay* Ring = T->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Countdown"));
	UImage* RingArt = T->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("CountdownRing"));
	RingArt->SetBrush(S::Ring(220.f));
	RingArt->SetVisibility(ESlateVisibility::HitTestInvisible);
	S::AddO(Ring, RingArt, HAlign_Center, VAlign_Center);
	// Lane A42: the digit and the word (HOLD) stack inside the ring; DROP / WON / LOST take the digit's block alone.
	UVerticalBox* RingStack = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	CountdownText = MakeText(T, TEXT("CountdownText"), S::Font(72, FName("Black"), 0, true), S::Paper());
	CountdownText->SetJustification(ETextJustify::Center);
	S::AddV(RingStack, CountdownText, FMargin(0), HAlign_Center);
	FaceoffWord = MakeText(T, TEXT("FaceoffWord"), S::Font(S::Label, FName("Black"), S::TrackTitle, true), S::Yellow());
	FaceoffWord->SetJustification(ETextJustify::Center);
	FaceoffWord->SetVisibility(ESlateVisibility::Collapsed);
	S::AddV(RingStack, FaceoffWord, FMargin(0, -6, 0, 0), HAlign_Center);
	S::AddO(Ring, RingStack, HAlign_Center, VAlign_Center);
	Ring->SetVisibility(ESlateVisibility::Collapsed);
	CountdownBox = Ring;
	// Below the TAKE card's slot (0.4): at a match start lane A16's countdown and lane A8's card are up together.
	// Lane A64 (eyes grid h19 finding 14: the WON ring sat on the human's hip and the YOU tag): the ring stands RingOffsetXPx right of centre, on open ice
	// beside the figure (the camera's 0.7 m off-axis puts the human left of centre), clear of the faceoff circle's far half.
	Place(Root, Ring, FVector2D(0.5, 0.66), FVector2D(0.5, 0.5), FVector2D(RingOffsetXPx, 0));
	// The one-line hint under the ring for the first three faceoffs of a session (R-0013 point 2 in the player's words).
	FaceoffHintPlate = S::Plate(T, S::Pill(0.78f), FMargin(18, 6));
	FaceoffHintPlate->SetVisibility(ESlateVisibility::Collapsed);
	UTextBlock* HintLine = MakeText(T, TEXT("FaceoffHint"), S::Font(S::Body, FName("Bold"), 20), S::Paper());
	HintLine->SetText(FText::FromString(FaceoffHintText()));
	HintLine->SetJustification(ETextJustify::Center);
	FaceoffHintPlate->SetContent(HintLine);
	Place(Root, FaceoffHintPlate, FVector2D(0.5, 0.66), FVector2D(0.5, 0), FVector2D(RingOffsetXPx, 124));

	// ---- Win / lose cards on a slate plate with a clapper stripe, and the restart hint under it.
	CardPlate = S::Plate(T, S::Panel(0.94f), FMargin(0, 0, 0, 18));
	CardPlate->SetVisibility(ESlateVisibility::Collapsed);
	UVerticalBox* CardBody = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	S::AddV(CardBody, S::Clapper(T, 10, 14), FMargin(0));
	CardTextBlock = MakeText(T, TEXT("CardText"), S::Font(S::Display, FName("Black"), S::TrackTitle), S::Paper());
	CardTextBlock->SetJustification(ETextJustify::Center);
	S::AddV(CardBody, CardTextBlock, FMargin(48, 14, 48, 2), HAlign_Center);
	HintText = MakeText(T, TEXT("HintText"), S::Font(S::Body, FName("Bold"), 40), S::Yellow());
	HintText->SetText(FText::FromString(TEXT("R  RESTART      ESC  PAUSE")));
	HintText->SetJustification(ETextJustify::Center);
	S::AddV(CardBody, HintText, FMargin(32, 0, 32, 0), HAlign_Center);
	CardPlate->SetContent(CardBody);
	Place(Root, CardPlate, FVector2D(0.5, 0.4), FVector2D(0.5, 0.5), FVector2D(0, 0));
}

void UJamHudWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	TRACE_CPUPROFILER_EVENT_SCOPE(JamHud_Tick);
	if (const UJamPresentation* P = UJamPresentation::Get(this))
	{
		if (P->HasFrame())
		{
			RefreshFromFrame(P->GetFrame());
			// Lane A14's feed: one slot per row that touches the human (icon name, effect id and the seconds left).
			TRACE_CPUPROFILER_EVENT_SCOPE(JamHud_EffectsFeed);
			const AHokejGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AHokejGameMode>() : nullptr;
			if (GM && GM->GetEffects())
			{
				TArray<TPair<FString, FString>> Slots;
				for (const FJamEffectHudEntry& R : GM->GetEffects()->GetHudFeedFor(P->GetFrame().HumanIndex))
				{
					Slots.Emplace(R.Icon.ToString(), FString::Printf(TEXT("%s %.1fs"), *R.EffectId.ToString().Replace(TEXT("_"), TEXT(" ")), R.FramesLeft / 60.0));
				}
				// The feed owns the row while it has rows; an empty feed clears only what the feed put there (a preview stays).
				if (Slots.Num()) { SetEffectSlots(Slots); bEffectsFromFeed = true; }
				else if (bEffectsFromFeed) { SetEffectSlots(Slots); bEffectsFromFeed = false; }
			}
		}
	}
}

FString UJamHudWidget::FormatEffects(const TArray<FJamEffectHudEntry>& Rows)
{
	TArray<FString> Parts;
	for (const FJamEffectHudEntry& R : Rows)
	{
		Parts.Add(FString::Printf(TEXT("%s %s %.1fs"), *R.Icon.ToString().ToUpper(), *R.EffectId.ToString(), R.FramesLeft / 60.0));
	}
	return FString::Join(Parts, TEXT("    "));
}

void UJamHudWidget::SetEffectSlots(const TArray<TPair<FString, FString>>& Slots)
{
	if (!EffectsRow || !EffectsPlate || !WidgetTree) return;
	// Rebuild only when the labels changed (the seconds tick, so most frames rebuild the text only).
	const int32 N = FMath::Min(Slots.Num(), 4);
	if (EffectsRow->GetChildrenCount() != N)
	{
		EffectsRow->ClearChildren();
		for (int32 i = 0; i < N; ++i)
		{
			UHorizontalBox* Cell = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
			UImage* Icon = S::Icon(WidgetTree, Slots[i].Key, 28.f);
			S::AddH(Cell, Icon, FMargin(0, 0, 6, 0));
			UTextBlock* Label = S::Text(WidgetTree, Slots[i].Value, S::CaptionFont(), S::Paper(), false);
			Label->SetVisibility(ESlateVisibility::HitTestInvisible);
			S::AddH(Cell, Label, FMargin(0));
			S::AddH(EffectsRow, Cell, FMargin(i ? 14.f : 0.f, 0, 0, 0));
		}
	}
	else
	{
		for (int32 i = 0; i < N; ++i)
		{
			if (UHorizontalBox* Cell = Cast<UHorizontalBox>(EffectsRow->GetChildAt(i)))
			{
				if (UTextBlock* Label = Cell->GetChildrenCount() > 1 ? Cast<UTextBlock>(Cell->GetChildAt(1)) : nullptr) Label->SetText(FText::FromString(Slots[i].Value));
			}
		}
	}
	EffectsPlate->SetVisibility(N ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

FString UJamHudWidget::GetCountdownText() const
{
	return CountdownText && CountdownBox && CountdownBox->GetVisibility() != ESlateVisibility::Collapsed ? CountdownText->GetText().ToString() : FString();
}

void UJamHudWidget::RefreshFromFrame(const FJamPresentationFrame& F)
{
	if (!ScoreHome)
	{
		return;
	}
	TRACE_CPUPROFILER_EVENT_SCOPE(JamHud_Refresh);
	ScoreHome->SetText(FText::FromString(FString::Printf(TEXT("%02d"), F.ScoreHome)));
	ScoreAway->SetText(FText::FromString(FString::Printf(TEXT("%02d"), F.ScoreAway)));
	HomeBox->SetBrushColor(F.TeamColour0);
	AwayBox->SetBrushColor(F.TeamColour1);
	// Lane A123: one clean centre plate. With no timer: "PERIOD n" over "FIRST TO n"; the plate's content is centred (it hung from the top border).
	const bool bCleanPlate = CVarJamHudCleanPlate.GetValueOnGameThread() != 0;
	const bool bTwoLines = bCleanPlate && !F.bHasTimer && F.ScoreToWin > 0;
	TakeText->SetText(FText::FromString(bTwoLines ? FormatPeriod(F.Take) : FormatTake(F.Take, F.ScoreToWin).ToUpper()));
	if (GoalRuleText)
	{
		GoalRuleText->SetText(FText::FromString(FormatGoalRule(F.ScoreToWin)));
		GoalRuleText->SetVisibility(bTwoLines ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
	if (CentrePlate && CentrePlateMode != (bCleanPlate ? 1 : 0))
	{
		CentrePlateMode = bCleanPlate ? 1 : 0;
		CentrePlate->SetVerticalAlignment(bCleanPlate ? VAlign_Center : VAlign_Fill);
	}
	TimerText->SetText(FText::FromString(F.bHasTimer ? FormatTimer(F.FramesRemaining) : FString()));
	// Lane A49: the stamina bar shows while the sprint runs or the bar is not full; yellow while sprinting, chalk while it refills.
	if (StaminaBack && StaminaFill)
	{
		const bool bShow = F.bHumanSprinting || F.HumanStamina01 < 0.999f;
		StaminaBack->SetVisibility(bShow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		StaminaFill->SetVisibility(bShow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		StaminaFill->SetBrushColor(F.bHumanSprinting ? S::Yellow() : S::Chalk());
		if (UCanvasPanelSlot* FS = Cast<UCanvasPanelSlot>(StaminaFill->Slot)) FS->SetSize(FVector2D(FMath::Max(2.f, 240.f * FMath::Clamp(F.HumanStamina01, 0.f, 1.f)), 8.f));
	}
	// Lane A84 (R-0041 point 3): the ability slot: yellow while armed or a shield runs, chalk when ready, grey while it cools down (the fill grows back to full).
	if (AbilityPlate && AbilityText && AbilityFill)
	{
		const bool bShow = !F.HumanAbility.IsEmpty();
		AbilityPlate->SetVisibility(bShow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		AbilityFill->SetVisibility(bShow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		if (bShow)
		{
			const bool bReady = F.HumanAbilityReady01 >= 0.999f;
			AbilityText->SetText(FText::FromString(FString::Printf(TEXT("F  %s  %s"), *F.HumanAbility, *F.HumanAbilityState)));
			AbilityText->SetColorAndOpacity(FSlateColor(F.bHumanAbilityLive ? S::Yellow() : bReady ? S::Paper() : FLinearColor(0.6f, 0.6f, 0.6f, 1.f)));
			AbilityFill->SetBrushColor(F.bHumanAbilityLive ? S::Yellow() : bReady ? S::Chalk() : FLinearColor(0.45f, 0.45f, 0.45f, 1.f));
			if (UCanvasPanelSlot* FS = Cast<UCanvasPanelSlot>(AbilityFill->Slot)) FS->SetSize(FVector2D(FMath::Max(2.f, (AbilitySlotW - 20.f) * FMath::Clamp(F.HumanAbilityReady01, 0.f, 1.f)), 4.f));
		}
	}
	TimerText->SetVisibility(F.bHasTimer ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

	const FString Card = CardFor(F.Phase);
	CardTextBlock->SetText(FText::FromString(Card));
	const bool bEnd = F.Phase == EJamPhase::Win || F.Phase == EJamPhase::Lose;
	CardTextBlock->SetVisibility(Card.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	// The plate shows for the end cards; lane A8's pause card covers the paused phase.
	CardPlate->SetVisibility(bEnd ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	HintText->SetVisibility(bEnd ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if (bEnd)
	{
		// Lane A17: a season take's end card says what R does next (NEXT TAKE after a win, RETAKE after a loss).
		FString Hint;
		const AHokejGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AHokejGameMode>() : nullptr;
		if (const UJamProgression* P = UJamProgression::Get(this); P && GM && GM->GetCartridge()) Hint = P->EndHint(GM->GetCartridge()->SourcePath);
		HintText->SetText(FText::FromString(Hint.IsEmpty() ? FString(TEXT("R  RESTART      ESC  PAUSE")) : Hint));
		CardTextBlock->SetColorAndOpacity(FSlateColor(F.Phase == EJamPhase::Win ? S::Yellow() : S::Paper()));
	}
	CallText->SetVisibility(ShowDirectorsCall(F) ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

	// Lane A64: the toast counts sim frames from the first frame it is up; a restart (the frame count going back) ends it.
	if (ToastPlate)
	{
		if (bToastPending) { ToastStart = F.SimFrame; bToastPending = false; }
		const bool bToast = ToastStart >= 0 && F.SimFrame >= ToastStart && F.SimFrame < ToastStart + ToastFrames;
		if (!bToast) ToastStart = -1;
		ToastPlate->SetVisibility(bToast ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	UpdateContactLine(F);   // lane A101

	// Controls strip: full for 8 s, fading over the last 2 s of the hint window.
	// Lane A111: the first match of a session only; spent once the window has run out in play or the human has shot (HandleJamEvent).
	const bool bHint = ShowControlsHintLive(F, bControlsHintSpent, CVarJamHudHintEveryMatch.GetValueOnGameThread() != 0);
	if (F.Phase == EJamPhase::Play && F.SimFrame >= ControlsHintFrames) bControlsHintSpent = true;
	bHintThisFrame = bHint;
	ControlsStrip->SetVisibility(bHint ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	ControlsText->SetVisibility(bHint ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if (bHint)
	{
		constexpr int64 FadeFrames = 120;
		const int64 Left = ControlsHintFrames - F.SimFrame;
		ControlsStrip->SetRenderOpacity(Left >= FadeFrames ? 1.f : static_cast<float>(Left) / static_cast<float>(FadeFrames));
	}

	// A restart (R, the pause menu, the season's next match) resets the frame count: the drop and draw memories belong to the old match.
	if (F.SimFrame < LastSimFrame) { LastDropFrame = -1; DrawResultUntil = -1; }
	const bool bSimAdvanced = F.SimFrame != LastSimFrame;
	LastSimFrame = F.SimFrame;

	// Faceoff countdown: the feed's frames (lane A16) or the preview; seconds while counting, DROP for DropFrames at zero.
	// Lane A42: the preview counts sim frames (it used to count render ticks), so a time-dilation freeze holds it for a capture.
	int32 Frames = FaceoffFramesOf(F);
	if (FaceoffPreview > 0) { Frames = FaceoffPreview; if (bSimAdvanced) --FaceoffPreview; }
	if (DrawPreviewFrames > 0) { if (bSimAdvanced) --DrawPreviewFrames; if (DrawResultUntil < F.SimFrame + 1) { DrawResultUntil = F.SimFrame + 1; } }
	if (Frames <= 0 && LastFaceoffFrames > 0) { DropUntil = F.SimFrame + DropFrames; if (LastDropFrame < 0 || LastDropFrame > F.SimFrame) LastDropFrame = F.SimFrame; }
	if (F.SimFrame < LastFaceoffFrames - 1 && Frames <= 0) DropUntil = -1;   // a restart reset the frame count
	// Count the faceoffs of this session on the countdown's first frame (the hint shows for the first three).
	if (Frames > 0 && !bCountdownSeen) { ++FaceoffsThisSession; bCountdownSeen = true; }
	if (Frames <= 0) bCountdownSeen = false;
	LastFaceoffFrames = Frames;
	const bool bResult = DrawResultUntil >= 0 && F.SimFrame < DrawResultUntil && F.Phase == EJamPhase::Play;
	const bool bDrop = !bResult && Frames <= 0 && DropUntil >= 0 && F.SimFrame < DropUntil && F.Phase == EJamPhase::Play;
	const bool bCount = Frames > 0 && F.Phase == EJamPhase::Play;
	const FLinearColor HumanColour = F.Skaters.IsValidIndex(F.HumanIndex) ? F.GetTeamColour(F.Skaters[F.HumanIndex].TeamId) : S::Blue();
	if (bCount)
	{
		// Lane A42: the digit over the word HOLD (R-0013 point 2: an early press loses the draw).
		CountdownText->SetText(FText::FromString(FString::FromInt(FMath::DivideAndRoundUp(Frames, 60))));
		CountdownText->SetColorAndOpacity(FSlateColor(S::Paper()));
		CountdownText->SetFont(S::Font(72, FName("Black"), 0, true));
		FaceoffWord->SetText(FText::FromString(FaceoffCue(Frames, false)));
	}
	else if (bResult)
	{
		CountdownText->SetText(FText::FromString(DrawResultText(bLastDrawWon)));
		CountdownText->SetColorAndOpacity(FSlateColor(HumanColour));
		CountdownText->SetFont(S::Font(S::Title + 8, FName("Black"), S::TrackTitle, true));
	}
	else if (bDrop)
	{
		CountdownText->SetText(FText::FromString(FaceoffCue(0, true)));
		CountdownText->SetColorAndOpacity(FSlateColor(S::Yellow()));
		CountdownText->SetFont(S::Font(S::Title + 8, FName("Black"), S::TrackTitle, true));
	}
	// Lane A123 (2): the ring and its hint stand outside the middle third (set on a change of the rule only).
	if (const float RingX = ClearMiddleOn() ? RingOffsetClearXPx : RingOffsetXPx; RingX != RingXApplied)
	{
		RingXApplied = RingX;
		if (UCanvasPanelSlot* RS = Cast<UCanvasPanelSlot>(CountdownBox->Slot)) RS->SetPosition(FVector2D(RingX, 0));
		if (UCanvasPanelSlot* HS = Cast<UCanvasPanelSlot>(FaceoffHintPlate->Slot)) HS->SetPosition(FVector2D(RingX, 124));
	}
	FaceoffWord->SetVisibility(bCount ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	CountdownBox->SetVisibility(bCount || bDrop || bResult ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	FaceoffHintPlate->SetVisibility(bCount && FaceoffsThisSession <= FaceoffHintCount ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

	bCountingThisFrame = bCount;
	bSimAdvancedThisFrame = bSimAdvanced;
	{ TRACE_CPUPROFILER_EVENT_SCOPE(JamHud_Radar); Radar->SetFrame(F); }
	{ TRACE_CPUPROFILER_EVENT_SCOPE(JamHud_NetMarkers); UpdateMarker(F); UpdateOwnMarker(F); }
	{ TRACE_CPUPROFILER_EVENT_SCOPE(JamHud_Yours); UpdateYours(F); }
	{ TRACE_CPUPROFILER_EVENT_SCOPE(JamHud_PuckMarker); UpdatePuckMarker(F, bCount || Frames > 0 || F.SimFrame < PuckMarkerCardFrames); ResolveEdgeSlots(); UpdateLabelPlates(); }
	{ TRACE_CPUPROFILER_EVENT_SCOPE(JamHud_Commentary); UpdateCommentary(F); }
	UpdatePrompts(F, Frames > 0, bSimAdvanced);
}

// Lane A123 (1): where the human figure's head is. The visual's head_socket (a mesh socket on any of its components: device B's figures; or a scene component of
// that name: the toy), else the top of its largest primitive's bounds. The component is found once per visual (a bounds answer is looked at again every 60 ticks:
// a mesh may arrive after the first frame).
int32 UJamHudWidget::HumanHeadWorld(FVector& OutWorld)
{
	const AHokejGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AHokejGameMode>() : nullptr;
	const AJamSkater* Human = GM ? GM->GetHumanSkater() : nullptr;
	UObject* Visual = Human ? Human->GetVisualObject() : nullptr;
	if (!Visual) { HeadVisual.Reset(); HeadComp.Reset(); HeadCompKind = 0; return 0; }
	const bool bLookAgain = HeadCompKind == 1 && (OverlaySearchTick % 60) == 0;
	if (HeadVisual.Get() != Visual || !HeadComp.IsValid() || bLookAgain)
	{
		HeadVisual = Visual; HeadComp.Reset(); HeadCompKind = 0;
		TArray<USceneComponent*> Candidates;
		if (USceneComponent* SC = Cast<USceneComponent>(Visual)) { Candidates.Add(SC); SC->GetChildrenComponents(true, Candidates); }
		else if (AActor* A = Cast<AActor>(Visual)) A->GetComponents<USceneComponent>(Candidates);
		for (USceneComponent* Cand : Candidates) if (Cand && Cand->DoesSocketExist(JamSkaterSockets::Head)) { HeadComp = Cand; HeadCompKind = 2; break; }
		if (HeadCompKind == 0) for (USceneComponent* Cand : Candidates) if (Cand && Cand->GetFName() == JamSkaterSockets::Head) { HeadComp = Cand; HeadCompKind = 3; break; }
		if (HeadCompKind == 0)
		{
			float Best = 0.f;
			for (USceneComponent* Cand : Candidates)
				if (const UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Cand); Prim && Prim->IsRegistered() && Prim->Bounds.SphereRadius > Best) { Best = Prim->Bounds.SphereRadius; HeadComp = Cand; HeadCompKind = 1; }
		}
	}
	const USceneComponent* Comp = HeadComp.Get();
	if (!Comp) return 0;
	if (HeadCompKind == 2)
	{
		OutWorld = Comp->GetSocketLocation(JamSkaterSockets::Head);
		// The socket floats over the helmet: come back along the head's own axis (it follows a lean and a fall), the world's up without a head bone.
		if (const float Lift = CVarJamHudMarkerLiftCm.GetValueOnGameThread(); Lift != 0.f)
		{
			static const FName HeadBoneName(TEXT("head"));
			FVector Axis = Comp->DoesSocketExist(HeadBoneName) ? (OutWorld - Comp->GetSocketLocation(HeadBoneName)).GetSafeNormal() : FVector::UpVector;
			if (Axis.IsNearlyZero()) Axis = FVector::UpVector;
			OutWorld += Axis * Lift;
		}
		return 2;
	}
	if (HeadCompKind == 3) { OutWorld = Comp->GetComponentLocation(); return 2; }
	OutWorld = FVector(Comp->Bounds.Origin.X, Comp->Bounds.Origin.Y, Comp->Bounds.Origin.Z + Comp->Bounds.BoxExtent.Z);
	return 1;
}

// Lane A123 (1), the proof's probe: per sim frame seen, where the tip stood (pixels of the viewport, not Slate units) against the head bone, how far the rule of
// before (1.8 m over the feet) stood from the tip, the head's height over the ice, and the frames the human was down. One line every N sim frames.
void UJamHudWidget::ProbeMarker(const FJamPresentationFrame& F, int32 Source, const FVector2D& Tip, const FVector2D& OldTip, const FVector& HeadWorld)
{
	const int32 Every = CVarJamHudMarkerProbe.GetValueOnGameThread();
	if (Every <= 0) return;
	if (F.SimFrame < Probe.LastLogged) Probe = FMarkerProbe();   // a restart
	const AHokejGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AHokejGameMode>() : nullptr;
	const bool bDown = GM && GM->GetSim() && GM->GetSim()->GetSkaters().IsValidIndex(F.HumanIndex) && GM->GetSim()->GetSkaters()[F.HumanIndex].IsDown();
	const float Scale = FMath::Max(UWidgetLayoutLibrary::GetViewportScale(this), 0.01f);
	++Probe.Frames;
	if (Source == 2) ++Probe.Socket; else if (Source == 1) ++Probe.Bounds; else ++Probe.Fallback;
	if (bDown) ++Probe.Down;
	const float OldPx = static_cast<float>(FVector2D::Distance(Tip, OldTip)) * Scale;
	Probe.OldMaxPx = FMath::Max(Probe.OldMaxPx, bDown ? 0.f : OldPx);
	Probe.OldMaxDownPx = FMath::Max(Probe.OldMaxDownPx, bDown ? OldPx : 0.f);
	Probe.HeadMinCm = FMath::Min(Probe.HeadMinCm, static_cast<float>(HeadWorld.Z));
	Probe.HeadMaxCm = FMath::Max(Probe.HeadMaxCm, static_cast<float>(HeadWorld.Z));
	// The head bone (the helmet's centre) is the second reference: the tip stands the helmet's radius and the gap over it.
	static const FName HeadBone(TEXT("head"));
	FVector2D BonePx;
	if (const USceneComponent* Comp = HeadComp.Get(); Comp && HeadCompKind == 2 && Comp->DoesSocketExist(HeadBone) && GetOwningPlayer()
		&& UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(GetOwningPlayer(), Comp->GetSocketLocation(HeadBone), BonePx, false))
	{
		Probe.BoneMaxPx = FMath::Max(Probe.BoneMaxPx, static_cast<float>(FVector2D::Distance(Tip, BonePx)) * Scale);
	}
	if (F.SimFrame - Probe.LastLogged >= Every)
	{
		Probe.LastLogged = F.SimFrame;
		UE_LOG(LogHokej, Log, TEXT("JAMSCREEN marker sim_frame=%lld frames=%lld socket=%lld bounds=%lld fallback=%lld down=%lld tip_over_anchor=%.1fpx tip_to_head_bone_max=%.1fpx old_rule_off_max=%.1fpx old_rule_off_down_max=%.1fpx head_cm=%.0f..%.0f scale=%.3f"),
			F.SimFrame, Probe.Frames, Probe.Socket, Probe.Bounds, Probe.Fallback, Probe.Down, YouMarkerGapPx * Scale, Probe.BoneMaxPx, Probe.OldMaxPx, Probe.OldMaxDownPx, Probe.HeadMinCm, Probe.HeadMaxCm, Scale);
	}
}

// Lane A123 (3): the two one-time prompts. Presentation only: reads the frame, the sim's carrier and the cartridge; writes nothing back.
void UJamHudWidget::UpdatePrompts(const FJamPresentationFrame& F, bool bCountingDown, bool bSimAdvanced)
{
	++OverlaySearchTick;
	if (!PromptPlate || !PromptTextBlock) return;
	if (PromptShown > F.SimFrame) { PromptShown = -1; PromptNow = EPrompt::None; }   // a restart
	WatchInputDevice();   // lane A135: the prompt's words follow the device last touched
	if (bSimAdvanced && CVarJamHudPromptProof.GetValueOnGameThread() > 0 && F.SimFrame == CVarJamHudPromptProof.GetValueOnGameThread()) PreviewPrompt(EPrompt::Brake, true);
	const AHokejGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AHokejGameMode>() : nullptr;
	int32 Carrier = -1;
	if (GM && GM->GetSim()) Carrier = GM->GetSim()->CarrierIndex();
	else { float Best = 1.2f; for (int32 i = 0; i < F.Skaters.Num(); ++i) { const float D = FVector2D::Distance(F.Skaters[i].PosM, F.PuckM); if (D < Best) { Best = D; Carrier = i; } } }
	const bool bCarry = F.Skaters.IsValidIndex(F.HumanIndex) && Carrier == F.HumanIndex;
	float Nearest = -1.f;
	if (bCarry)
	{
		for (const FJamPresentationSkater& Sk : F.Skaters)
		{
			if (Sk.TeamId == F.Skaters[F.HumanIndex].TeamId) continue;
			const float D = static_cast<float>(FVector2D::Distance(Sk.PosM, F.Skaters[F.HumanIndex].PosM));
			if (Nearest < 0.f || D < Nearest) Nearest = D;
		}
	}
	const bool bClosing = bCarry && Nearest >= 0.f && LastNearestOpponentM >= 0.f && Nearest < LastNearestOpponentM - 1e-3f;
	if (bSimAdvanced) LastNearestOpponentM = bCarry ? Nearest : -1.f;
	const bool bKeptRule = !(GM && GM->GetCartridge() && PassiveTakeOf(GM->GetCartridge()->Game.Stick));
	// One teaching surface at a time: nothing is due while the controls strip of the first match shows or a faceoff counts.
	const bool bPlayLive = F.Phase == EJamPhase::Play && !bCountingDown && !bHintThisFrame;
	if (PromptNow == EPrompt::None || PromptOpacity(PromptShown, F.SimFrame) <= 0.f)
	{
		const EPrompt Due = PromptPreview != EPrompt::None ? PromptPreview
			: PromptDue(CVarJamHudPrompts.GetValueOnGameThread() != 0, bPlayLive, bCarry, Nearest, bClosing, bKeptPromptSpent, bBrakePromptSpent, bKeptRule);
		PromptPreview = EPrompt::None;
		PromptNow = Due;
		if (Due != EPrompt::None)
		{
			PromptShown = F.SimFrame;
			bPromptLow = bPreviewLow;
			bPreviewLow = false;
			bPromptPlaced = false;
			if (Due == EPrompt::Kept) bKeptPromptSpent = true; else bBrakePromptSpent = true;
			PromptTextBlock->SetText(FText::FromString(LivePromptText(Due)));
			UE_LOG(LogHokej, Log, TEXT("JAMSCREEN prompt %s at sim frame %lld (nearest opponent %.1f m)%s: %s"), Due == EPrompt::Kept ? TEXT("kept") : TEXT("brake"), F.SimFrame, Nearest,
				bPromptLow ? TEXT(" in the lower slot") : TEXT(""), *LivePromptText(Due));
		}
	}
	// Lane A135: a prompt on screen changes its words when the player takes the other device.
	if (PromptNow == EPrompt::Brake && !PromptTextBlock->GetText().ToString().Equals(LivePromptText(PromptNow))) PromptTextBlock->SetText(FText::FromString(LivePromptText(PromptNow)));
	const float Alpha = (PromptNow != EPrompt::None && F.Phase == EJamPhase::Play) ? PromptOpacity(PromptShown, F.SimFrame) : 0.f;
	PromptPlate->SetVisibility(Alpha > 0.f ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if (Alpha <= 0.f) return;
	PromptPlate->SetRenderOpacity(Alpha);
	// Under the scoreboard; one slot lower while the overlay's line (a call's result, a RULE line) holds that place.
	if (!OverlayForPrompt.IsValid() && (OverlaySearchTick % 60) == 1)
	{
		for (TObjectIterator<UJamDirectorsCallHudWidget> It; It; ++It) if (It->GetWorld() == GetWorld() && !It->HasAnyFlags(RF_ClassDefaultObject)) { OverlayForPrompt = *It; break; }
	}
	// The same for a net badge or the puck's arrow standing in the top slot (the far net straight ahead on a long rink). Once low, this prompt stays low.
	const float VWp = static_cast<float>(UWidgetLayoutLibrary::GetViewportSize(this).X) / FMath::Max(UWidgetLayoutLibrary::GetViewportScale(this), 0.01f);
	const float HalfWp = FMath::Max(240.f, static_cast<float>(PromptPlate->GetDesiredSize().X) * 0.5f);
	bool bTaken = OverlayForPrompt.IsValid() && OverlayForPrompt->IsLineVisible();
	// Lane A135, evolution 4: the far-net badge no longer sends the prompt down onto the net; the net's box decides the place instead.
	const bool bOffNet = CVarJamHudPromptOffNet.GetValueOnGameThread() != 0;
	const UWidget* EdgeMarks[] = { bOffNet ? nullptr : NetDiamond.Get(), OwnDiamond.Get(), PuckArrow.Get() };
	for (const UWidget* Mark : EdgeMarks)
	{
		const UCanvasPanelSlot* MS = Mark && Mark->GetVisibility() != ESlateVisibility::Collapsed && Mark->GetRenderOpacity() > 0.f ? Cast<UCanvasPanelSlot>(Mark->Slot) : nullptr;
		if (MS && InPromptSlot(MS->GetPosition(), VWp, HalfWp)) bTaken = true;
	}
	float X = 0.f, Y = PromptSlotY;
	if (!bOffNet)
	{
		PromptGhost = 1.f;
		if (bTaken) bPromptLow = true;
		Y = PromptSlotFor(bPromptLow);
	}
	else
	{
		FVector2D NetMin, NetMax;
		const bool bNet = NetBoxOnScreen(F, NetMin, NetMax);
		// The prompt keeps its place while that place is clear; it moves only off the net or out of a taken top row.
		const bool bInTop = PromptPlaceNow.Y < PromptSlotLowY - 1.f;
		const bool bOnNet = bPromptPlaced && bNet && PromptBoxMeets(PromptPlaceNow, VWp, HalfWp, NetMin, NetMax);
		if (!bOnNet) PromptOnNetSince = -1;
		else if (PromptOnNetSince < 0) PromptOnNetSince = F.SimFrame;
		PromptGhost = FMath::Clamp(PromptGhost + (bOnNet ? -0.125f : 0.125f), PromptGhostAlpha, 1.f);
		PromptPlate->SetRenderOpacity(Alpha * PromptGhost);
		const bool bKeep = bPromptPlaced && !(bInTop && (bTaken || bPromptLow)) && !(bOnNet && F.SimFrame - PromptOnNetSince >= PromptOnNetFrames);
		if (!bKeep)
		{
			PromptOnNetSince = -1;
			PromptPlaceNow = PromptPlace(VWp, HalfWp, bTaken || bPromptLow, bNet, NetMin, NetMax);
			if (bPromptPlaced || PromptPlaceNow.X != 0.f) UE_LOG(LogHokej, Log, TEXT("JAMSCREEN prompt place at sim frame %lld: x %+.0f y %.0f (net box %.0f..%.0f x %.0f..%.0f, net on screen %d, top taken %d)"), F.SimFrame,
				PromptPlaceNow.X, PromptPlaceNow.Y, NetMin.X, NetMax.X, NetMin.Y, NetMax.Y, bNet ? 1 : 0, (bTaken || bPromptLow) ? 1 : 0);
			bPromptPlaced = true;
		}
		X = static_cast<float>(PromptPlaceNow.X);
		Y = static_cast<float>(PromptPlaceNow.Y);
	}
	if (UCanvasPanelSlot* CS = Cast<UCanvasPanelSlot>(PromptPlate->Slot)) CS->SetPosition(FVector2D(X, Y));
	PromptBandTick = GFrameCounter;
	PromptBandTop = Y;
}

FString UJamHudWidget::GetPromptText() const
{
	return PromptPlate && PromptTextBlock && PromptPlate->GetVisibility() != ESlateVisibility::Collapsed ? PromptTextBlock->GetText().ToString() : FString();
}

// Lane A111: the dark plate behind each distance label. The label's own slot stays the anchor every other rule moves (the edge slots, the clamp); the plate
// copies its place one step later, sized by the label's desired size of the last layout pass.
void UJamHudWidget::UpdateLabelPlates()
{
	const bool bOn = CVarJamHudLabelPlates.GetValueOnGameThread() != 0;
	auto Follow = [bOn](UBorder* Plate, UTextBlock* Text)
	{
		if (!Plate) return;
		UCanvasPanelSlot* TS = Text ? Cast<UCanvasPanelSlot>(Text->Slot) : nullptr;
		UCanvasPanelSlot* PS = Cast<UCanvasPanelSlot>(Plate->Slot);
		const FVector2D Sz = Text ? Text->GetDesiredSize() : FVector2D::ZeroVector;
		const bool bShow = bOn && TS && PS && Text->GetVisibility() != ESlateVisibility::Collapsed && Text->GetRenderOpacity() > 0.f && Sz.X > 1.f && !Text->GetText().IsEmpty();
		Plate->SetVisibility(bShow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		if (!bShow) return;
		PS->SetAlignment(TS->GetAlignment());
		PS->SetPosition(LabelPlatePos(TS->GetPosition(), TS->GetAlignment()));
		PS->SetSize(Sz + FVector2D(2.f * LabelPlatePadX, 2.f * LabelPlatePadY));
		Plate->SetRenderOpacity(Text->GetRenderOpacity());
	};
	Follow(NetPlate, NetText);
	Follow(OwnPlate, OwnText);
	Follow(PuckPlate, PuckText);
}

void UJamHudWidget::UpdatePuckMarker(const FJamPresentationFrame& F, bool bCovered)
{
	if (!PuckArrow || !PuckText) return;
	APlayerController* PC = GetOwningPlayer();
	const bool bPaused = F.Phase == EJamPhase::Paused || (GetWorld() && GetWorld()->IsPaused());
	const bool bAdvanced = F.SimFrame != PuckMarkerLastFrame;
	if (F.SimFrame < PuckMarkerLastFrame || bPaused) PuckMarkerAlpha = 0.f;   // a restart or the pause menu: no fade, gone at once
	PuckMarkerLastFrame = F.SimFrame;

	// Where the puck is on the screen: the projected point when it is in front of the camera, else the mirrored direction.
	bool bWant = false;
	FVector2D Dir = FVector2D::ZeroVector;
	float DistM = 0.f;
	if (CVarJamHudPuckMarker.GetValueOnGameThread() != 0 && F.Phase == EJamPhase::Play && !bCovered && !bPaused && PC && PC->PlayerCameraManager && F.Skaters.IsValidIndex(F.HumanIndex))
	{
		const FVector2D VS = UWidgetLayoutLibrary::GetViewportSize(this);
		const float Scale = FMath::Max(UWidgetLayoutLibrary::GetViewportScale(this), 0.01f);
		const float VW = VS.X / Scale, VH = VS.Y / Scale;
		const FVector Puck = UJamSim::ToWorld(F.PuckM, 0.1);
		FVector2D Sp(VW * 0.5f, VH * 0.5f);
		const bool bProj = UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, Puck, Sp, false);
		const FVector Local = FRotationMatrix(PC->PlayerCameraManager->GetCameraRotation()).GetTransposed().TransformVector(Puck - PC->PlayerCameraManager->GetCameraLocation());
		const bool bFront = bProj && Local.X > 1.f;
		if (!bFront) { Dir = FVector2D(Local.Y, -Local.Z); if (FMath::Abs(Dir.X) < 1.f && FMath::Abs(Dir.Y) < 1.f) Dir = FVector2D(Local.Y >= 0.f ? 1.f : -1.f, 0.5f); }
		else Dir = Sp - FVector2D(VW * 0.5f, VH * 0.5f);
		bWant = !bFront || OutsideInnerFrame(Sp, VW, VH);
		DistM = FVector2D::Distance(F.Skaters[F.HumanIndex].PosM, F.PuckM);
		if (bWant)
		{
			Dir = Dir.GetSafeNormal();
			// The rectangle the arrow stands on: under the scoreboard, above the strips; the radar corner is kept clear.
			FVector2D P = PuckMarkerEdgePoint(Dir, VW, VH, 64.f, VW - 64.f, 150.f, VH - 170.f);
			if (P.X > VW - 330.f && P.Y > VH - 200.f) P.Y = VH - 200.f;
			const float Angle = FMath::RadiansToDegrees(FMath::Atan2(-Dir.X, Dir.Y));
			PuckArrow->SetRenderTransformAngle(Angle);
			if (UCanvasPanelSlot* CS = Cast<UCanvasPanelSlot>(PuckArrow->Slot)) CS->SetPosition(P);
			PuckMarkerText = FString::Printf(TEXT("%d m"), FMath::RoundToInt32(DistM));
			PuckText->SetText(FText::FromString(PuckMarkerText));
			if (UCanvasPanelSlot* TS = Cast<UCanvasPanelSlot>(PuckText->Slot)) TS->SetPosition(P - Dir * FVector2D(62.f, 34.f));
			// The carrier's team colour (the sim's carrier when the game mode has one, else the skater on the puck); a loose puck is grey.
			int32 Carrier = -1;
			const AHokejGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AHokejGameMode>() : nullptr;
			if (GM && GM->GetSim()) Carrier = GM->GetSim()->CarrierIndex();
			else { float Best = 1.2f; for (int32 i = 0; i < F.Skaters.Num(); ++i) { const float D = FVector2D::Distance(F.Skaters[i].PosM, F.PuckM); if (D < Best) { Best = D; Carrier = i; } } }
			const FLinearColor Col = F.Skaters.IsValidIndex(Carrier) ? F.GetTeamColour(F.Skaters[Carrier].TeamId) : FLinearColor::FromSRGBColor(FColor(0x6B, 0x72, 0x7A));   // loose: a grey that reads on white ice
			// Re-tint the chevron's bars (the tree built them in S::Muted()).
			if (UPanelWidget* Box = Cast<UPanelWidget>(PuckArrow)) if (UPanelWidget* O = Box->GetChildrenCount() ? Cast<UPanelWidget>(Box->GetChildAt(0)) : nullptr)
				for (int32 i = 0; i < O->GetChildrenCount(); ++i) if (UBorder* Bar = Cast<UBorder>(O->GetChildAt(i))) Bar->SetBrushColor(Col);
		}
	}
	if (bAdvanced || PuckMarkerAlpha <= 0.f)
	{
		const float Step = 1.f / static_cast<float>(PuckMarkerFadeFrames);
		PuckMarkerAlpha = FMath::Clamp(PuckMarkerAlpha + (bWant ? Step : -Step), 0.f, 1.f);
	}
	const ESlateVisibility Vis = PuckMarkerAlpha > 0.f ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
	PuckArrow->SetVisibility(Vis);
	PuckText->SetVisibility(Vis);
	PuckArrow->SetRenderOpacity(PuckMarkerAlpha);
	PuckText->SetRenderOpacity(PuckMarkerAlpha);
}

// Lane A76: the edge-slot rule. Every edge element (the puck chevron, the far-net and own-net chevrons) with its label is one group; the groups are placed
// puck first, far net, own net. A group that stands within EdgeSlotPx of a placed one (anchor to anchor) or whose label rectangle meets a placed
// rectangle slides along its own edge (up or down a side edge, along a top or bottom edge) in EdgeSlotPx steps, away from the blocker first.
// A skater under a label is accepted (labels are over the world); a label over another label is not. The puck never moves.
void UJamHudWidget::ResolveEdgeSlots()
{
	struct FPart { UWidget* W; FVector2D Half; FVector2D Tex; bool bText; };
	struct FGroup { TArray<FPart> Parts; UWidget* Anchor = nullptr; bool bFixed = false; bool bActive = false; FBox2D Box = FBox2D(ForceInit); FVector2D AnchorPos = FVector2D::ZeroVector; };
	auto Visible = [](const UWidget* W) { return W && W->GetVisibility() != ESlateVisibility::Collapsed && W->GetRenderOpacity() > 0.f; };
	auto PosOf = [](UWidget* W, FVector2D& OutPos, FVector2D& OutAlign) -> bool
	{
		UCanvasPanelSlot* CS = W ? Cast<UCanvasPanelSlot>(W->Slot) : nullptr;
		if (!CS) return false;
		OutPos = CS->GetPosition(); OutAlign = CS->GetAlignment();
		return true;
	};
	auto TextSize = [](const UTextBlock* T, float PerChar, float H) { return FVector2D(PerChar * (T ? T->GetText().ToString().Len() : 4), H); };
	if (!NetDiamond || !OwnDiamond || !PuckArrow || CVarJamHudPass3.GetValueOnGameThread() == 0) return;

	FGroup G[3];   // 0 puck, 1 far net, 2 own net
	auto AddPart = [&](FGroup& Gr, UWidget* W, FVector2D Half, bool bText, FVector2D Tex = FVector2D::ZeroVector)
	{
		if (Visible(W)) Gr.Parts.Add({ W, Half, Tex, bText });
	};
	G[0].bFixed = true; G[0].Anchor = PuckArrow;
	AddPart(G[0], PuckArrow, FVector2D(16, 16), false);
	AddPart(G[0], PuckText, FVector2D::ZeroVector, true, TextSize(PuckText, 12.f, 26.f));
	G[1].Anchor = NetArrow;
	AddPart(G[1], NetDiamond, FVector2D(12, 12), false);
	AddPart(G[1], NetArrow, FVector2D(12, 12), false);
	AddPart(G[1], NetText, FVector2D::ZeroVector, true, TextSize(NetText, 12.f, 26.f));
	G[2].Anchor = OwnArrow;
	AddPart(G[2], OwnDiamond, FVector2D(8, 8), false);
	AddPart(G[2], OwnArrow, FVector2D(10, 10), false);
	AddPart(G[2], OwnText, FVector2D::ZeroVector, true, TextSize(OwnText, 10.f, 22.f));

	const FVector2D VS = UWidgetLayoutLibrary::GetViewportSize(this);
	const float VW = VS.X / FMath::Max(UWidgetLayoutLibrary::GetViewportScale(this), 0.01f), VH = VS.Y / FMath::Max(UWidgetLayoutLibrary::GetViewportScale(this), 0.01f);

	auto Measure = [&](FGroup& Gr, const FVector2D& Delta)
	{
		Gr.Box = FBox2D(ForceInit);
		for (const FPart& P : Gr.Parts)
		{
			FVector2D Pos, Al;
			if (!PosOf(P.W, Pos, Al)) continue;
			Pos += Delta;
			if (P.bText) Gr.Box += FBox2D(Pos - FVector2D(Al.X * P.Tex.X, Al.Y * P.Tex.Y), Pos + FVector2D((1.f - Al.X) * P.Tex.X, (1.f - Al.Y) * P.Tex.Y));
			else Gr.Box += FBox2D(Pos - P.Half, Pos + P.Half);
		}
		FVector2D Al;
		Gr.bActive = Gr.Anchor && Visible(Gr.Anchor) && PosOf(Gr.Anchor, Gr.AnchorPos, Al);
		if (Gr.bActive) Gr.AnchorPos += Delta;
	};
	auto Conflicts = [&](const FGroup& A, const FGroup& B)
	{
		if (!A.Box.bIsValid || !B.Box.bIsValid) return false;
		if (A.bActive && B.bActive && FVector2D::Distance(A.AnchorPos, B.AnchorPos) < EdgeSlotPx) return true;
		return A.Box.ExpandBy(4.f).Intersect(B.Box);
	};

	for (int32 i = 0; i < 3; ++i) Measure(G[i], FVector2D::ZeroVector);
	for (int32 i = 1; i < 3; ++i)
	{
		FGroup& Gr = G[i];
		if (!Gr.bActive) continue;   // an on-screen diamond stands on its net: nothing to slide (labels over the world)
		int32 Hit = -1;
		for (int32 j = 0; j < i && Hit < 0; ++j) if (G[j].Parts.Num() && Conflicts(Gr, G[j])) Hit = j;
		if (Hit < 0) continue;
		const float EdgeL = Gr.AnchorPos.X, EdgeR = VW - Gr.AnchorPos.X, EdgeT = Gr.AnchorPos.Y, EdgeB = VH - Gr.AnchorPos.Y;
		const bool bSide = FMath::Min(EdgeL, EdgeR) <= FMath::Min(EdgeT, EdgeB);
		const FVector2D Axis = bSide ? FVector2D(0, 1) : FVector2D(1, 0);
		const float Here = bSide ? Gr.AnchorPos.Y : Gr.AnchorPos.X;
		const float Lo = bSide ? 130.f : 100.f, Hi = bSide ? VH - 200.f : VW - 100.f;
		const float Away = Here >= (bSide ? G[Hit].AnchorPos.Y : G[Hit].AnchorPos.X) ? 1.f : -1.f;
		bool bDone = false;
		for (int32 k = 1; k <= 12 && !bDone; ++k)
		{
			for (const float Sign : { Away, -Away })
			{
				const float To = Here + Sign * EdgeSlotPx * 0.5f * k;
				if (To < Lo || To > Hi) continue;
				const FVector2D Delta = Axis * (To - Here);
				Measure(Gr, Delta);
				bool bFree = true;
				for (int32 j = 0; j < i && bFree; ++j) if (G[j].Parts.Num() && Conflicts(Gr, G[j])) bFree = false;
				if (!bFree) continue;
				for (const FPart& P : Gr.Parts) { FVector2D Pos, Al; if (PosOf(P.W, Pos, Al)) Cast<UCanvasPanelSlot>(P.W->Slot)->SetPosition(Pos + Delta); }
				bDone = true;
				break;
			}
		}
		Measure(Gr, FVector2D::ZeroVector);   // (re-read below: the slots now carry the shift when one was found)
		if (bDone) Measure(Gr, FVector2D::ZeroVector);
	}
}

void UJamHudWidget::UpdateCommentary(const FJamPresentationFrame& F)
{
	if (!CommentaryStrip || !CommentaryText) return;
	if (const UJamCommentarySystem* C = UJamCommentarySystem::Get(this))
	{
		const FJamCommentaryPick& L = C->GetLastLine();
		if (!L.Text.IsEmpty() && L.Frame != CommentaryFrame)
		{
			CommentaryText->SetText(FText::FromString(L.Text));
			CommentaryFrame = L.Frame;
			CommentaryDuration = L.DurationFrames;
		}
		// Lane A47 (B-to-A-0030 fault 7): the caption follows the voice: a cut line (bark, must-play goal call) ends at the cut frame.
		const int64 End = C->GetLineEndFrame();
		if (CommentaryFrame > -1000000 && End >= CommentaryFrame) CommentaryDuration = static_cast<int32>(FMath::Min<int64>(End - CommentaryFrame, L.DurationFrames));
	}
	// Nothing on the strip under the pause card (A48's storyboard, still 9): the pause menu pauses the world (no new frame carries the phase).
	const bool bPaused = F.Phase == EJamPhase::Paused || (GetWorld() && GetWorld()->IsPaused());
	const float Alpha = bPaused ? 0.f : CommentaryOpacityFor(CommentaryFrame, CommentaryDuration, F.SimFrame, 0);   // A47: the caption ends with the line
	CommentaryStrip->SetRenderOpacity(Alpha);
	CommentaryStrip->SetVisibility(Alpha > 0.f ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if (Alpha > 0.f)
	{
		// Clear of the radar frame on the right and the ready pill on the left at every width; above the controls strip while that shows.
		const FVector2D VS = UWidgetLayoutLibrary::GetViewportSize(this);
		const float VW = VS.X / FMath::Max(UWidgetLayoutLibrary::GetViewportScale(this), 0.01f);
		CommentaryText->SetWrapTextAt(FMath::Clamp(VW - 600.f, 560.f, 880.f));
		// Lane A111: docked on the bottom edge (it floated 110 px up, over the play); over the controls strip and the CALL READY line while they show.
		const bool bCallLine = CallText && CallText->GetVisibility() != ESlateVisibility::Collapsed;
		if (UCanvasPanelSlot* CS = Cast<UCanvasPanelSlot>(CommentaryStrip->Slot))
			CS->SetPosition(FVector2D(0, CommentaryStripY(CVarJamHudDockCommentary.GetValueOnGameThread() != 0, bHintThisFrame, bCallLine)));
	}
}

FString UJamHudWidget::GetCommentaryText() const
{
	return CommentaryText && CommentaryStrip && CommentaryStrip->GetVisibility() != ESlateVisibility::Collapsed ? CommentaryText->GetText().ToString() : FString();
}

void UJamHudWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (UGameInstance* GI = GetGameInstance()) Bus = GI->GetSubsystem<UJamEventBus>();
	if (Bus) Bus->OnJamEvent.AddUniqueDynamic(this, &UJamHudWidget::HandleJamEvent);
}

void UJamHudWidget::NativeDestruct()
{
	if (Bus) Bus->OnJamEvent.RemoveDynamic(this, &UJamHudWidget::HandleJamEvent);
	Bus = nullptr;
	Super::NativeDestruct();
}

void UJamHudWidget::HandleJamEvent(FName Event, const FJamEventPayload& Payload)
{
	if (Event != JamEvents::Faceoff && Event != JamEvents::Shot) return;
	const UJamPresentation* P = UJamPresentation::Get(this);
	if (!P || !P->HasFrame()) return;
	const FJamPresentationFrame& F = P->GetFrame();
	// Lane A111: the human's first shot of the session ends the controls hint (he has found the button).
	if (Event == JamEvents::Shot) { if (Payload.Who == F.HumanIndex) bControlsHintSpent = true; return; }
	static const FName KindDrop(TEXT("drop")), KindWon(TEXT("won"));
	if (Payload.Kind == KindDrop)
	{
		LastDropFrame = F.SimFrame;
	}
	else if (Payload.Kind == KindWon && F.Skaters.IsValidIndex(Payload.Who) && F.Skaters.IsValidIndex(F.HumanIndex))
	{
		// WON when the centre that won skates for the human's team (the human or a teammate), LOST otherwise.
		bLastDrawWon = F.Skaters[Payload.Who].TeamId == F.Skaters[F.HumanIndex].TeamId;
		DrawResultUntil = F.SimFrame + DrawResultFrames;
	}
}

FString UJamHudWidget::GetFaceoffCue() const
{
	if (!CountdownBox || CountdownBox->GetVisibility() == ESlateVisibility::Collapsed) return FString();
	if (FaceoffWord && FaceoffWord->GetVisibility() != ESlateVisibility::Collapsed) return FaceoffWord->GetText().ToString();
	return CountdownText ? CountdownText->GetText().ToString() : FString();
}

bool UJamHudWidget::IsYouTagVisible() const { return YouTag && YouTag->GetVisibility() != ESlateVisibility::Collapsed; }

bool UJamHudWidget::ProjectGroundRing(const FVector2D& PosM, float RadiusM, FVector2D& OutCentre, FVector2D& OutSize, float& OutCameraDistM) const
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC || !PC->PlayerCameraManager) return false;
	const FVector Foot = UJamSim::ToWorld(PosM, 0.03);
	const FVector CamLoc = PC->PlayerCameraManager->GetCameraLocation();
	const FVector Local = FRotationMatrix(PC->PlayerCameraManager->GetCameraRotation()).GetTransposed().TransformVector(Foot - CamLoc);
	if (Local.X <= 50.f) return false;
	OutCameraDistM = static_cast<float>(FVector::Dist(Foot, CamLoc) / 100.0);
	FVector2D Centre;
	if (!UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, Foot, Centre, false)) return false;
	// Four points of the circle on the ice give the ellipse's box; the ring art stretches into it.
	const float R = RadiusM * 100.f;
	FVector2D Min = Centre, Max = Centre;
	for (const FVector& Off : { FVector(R, 0, 0), FVector(-R, 0, 0), FVector(0, R, 0), FVector(0, -R, 0) })
	{
		FVector2D Sp;
		if (!UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, Foot + Off, Sp, false)) return false;
		Min = FVector2D::Min(Min, Sp);
		Max = FVector2D::Max(Max, Sp);
	}
	OutCentre = (Min + Max) * 0.5f;
	OutSize = FVector2D(FMath::Max(Max.X - Min.X, 8.f), FMath::Max(Max.Y - Min.Y, 4.f));
	return true;
}

void UJamHudWidget::UpdateYours(const FJamPresentationFrame& F)
{
	if (!YouRing || !CarrierRing || !YouChevron || !YouTag) return;
	auto Hide = [](UWidget* W) { if (W) W->SetVisibility(ESlateVisibility::Collapsed); };
	const bool bPlay = F.Phase == EJamPhase::Play || F.Phase == EJamPhase::GoalPause;
	if (!bPlay || !F.Skaters.IsValidIndex(F.HumanIndex)) { Hide(YouRing); Hide(CarrierRing); Hide(YouChevron); Hide(YouTag); return; }
	const FJamPresentationSkater& Me = F.Skaters[F.HumanIndex];
	const FLinearColor Team = F.GetTeamColour(Me.TeamId);
	APlayerController* PC = GetOwningPlayer();

	// The ring under the human: team colour, 1.1 m; from 20 m it fades and the chevron over the head takes over (24 m: ring gone).
	FVector2D Centre, Size;
	float DistM = 0.f;
	const bool bOn = ProjectGroundRing(Me.PosM, YouRingRadiusM, Centre, Size, DistM);
	const float RingAlpha = bOn ? RingAlphaAt(DistM) : 0.f;
	if (RingAlpha > 0.f)
	{
		YouRing->SetColorAndOpacity(FLinearColor(Team.R, Team.G, Team.B, RingAlpha));
		YouRing->SetVisibility(ESlateVisibility::HitTestInvisible);
		if (UCanvasPanelSlot* CS = Cast<UCanvasPanelSlot>(YouRing->Slot)) { CS->SetPosition(Centre); CS->SetSize(Size); }
	}
	else Hide(YouRing);

	// Over the head: the chevron (team colour) past the fade, the YOU tag the first 5 s and 2 s after every drop.
	FVector2D Head;
	// Just over the helmet (the toy figure's head socket is at 1.65 m; the bark bubbles sit higher, at 2.35 m, so the two never meet).
	// Lane A123 (1): the tip follows the figure's head (its socket, else its bounds): the clay figure is shorter than 1.8 m, crouches at the faceoff, leans and
	// lies down; the point over the feet stood 40 to 60 px over the helmet, beside the puck at the faceoff, and over nothing with the human down.
	FVector2D OldHead;
	const bool bOldHead = bOn && PC && UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, UJamSim::ToWorld(Me.PosM, YouMarkerHeadM), OldHead, false);
	const bool bAnchor = CVarJamHudMarkerAnchor.GetValueOnGameThread() != 0;
	FVector HeadWorld = UJamSim::ToWorld(Me.PosM, YouMarkerHeadM);
	const int32 HeadSource = bAnchor ? HumanHeadWorld(HeadWorld) : 0;
	bool bHead = bOldHead;
	Head = OldHead;
	if (HeadSource > 0) bHead = bOn && PC && UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, HeadWorld, Head, false);
	const FVector2D Tip = YouMarkerTip(Head);
	if (bHead && bOldHead && bSimAdvancedThisFrame) ProbeMarker(F, HeadSource, Tip, YouMarkerTip(OldHead), HeadWorld);
	// The marker never sits on the puck or the dot under it: a ghost while the puck's projection is inside the chevron's box, no tag while it is inside the tag's.
	FVector2D PuckPx, ChevMin, ChevMax, TagMin, TagMax;
	YouChevronBox(Tip, ChevMin, ChevMax);
	YouTagBox(Tip, TagMin, TagMax);
	const bool bPuckPx = bAnchor && bHead && PC && UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, UJamSim::ToWorld(F.PuckM, 0.1), PuckPx, false);
	const float YieldTarget = MarkerYieldAlpha(bPuckPx && MarkerBoxHasPuck(ChevMin, ChevMax, PuckPx));
	if (!bAnchor) MarkerYield = 1.f;
	else if (bSimAdvancedThisFrame) MarkerYield += FMath::Clamp(YieldTarget - MarkerYield, -0.12f, 0.12f);
	const bool bTagOnPuck = bPuckPx && MarkerBoxHasPuck(TagMin, TagMax, PuckPx);
	// Lane A111 (B-to-A-0038 fault 7): the chevron is the human's marker for the whole match, in the HUD's yellow; jam.hud.YouMarker 0 = A42's fade rule.
	const bool bMarker = CVarJamHudYouMarker.GetValueOnGameThread() != 0;
	const float ChevronAlpha = YouMarkerAlpha(bMarker, RingAlpha);
	// A48's storyboard (finding 2): a point 1.9 m over the helmet projects onto whoever stands behind the human at the match camera. While the
	// ring shows, the tag sits inside the ring's top arc (the human's own feet, nobody else's); only past the fade does it go over the head.
	// Lane A111: with the marker on, the tag rides above the chevron (one sign, "YOU" over an arrow at the helmet) and the ring keeps the feet clear.
	const bool bTagAtRing = !bMarker && RingAlpha > 0.f;
	if (bHead && ChevronAlpha > 0.f)
	{
		YouChevron->SetRenderOpacity(ChevronAlpha * (bMarker ? MarkerYield : 1.f));
		YouChevron->SetVisibility(ESlateVisibility::HitTestInvisible);
		if (UCanvasPanelSlot* CS = Cast<UCanvasPanelSlot>(YouChevron->Slot)) CS->SetPosition(Tip);
		// Two bars: take-yellow with a 2 px ink edge as the marker, the team colour under A42's rule. Re-built only when the colour changes (it is on every frame now).
		const FLinearColor Want = bMarker ? S::Yellow() : Team;
		if (!Want.Equals(YouChevronColour))
		{
			YouChevronColour = Want;
			if (USizeBox* Box = Cast<USizeBox>(YouChevron)) if (UOverlay* O = Box->GetChildAt(0) ? Cast<UOverlay>(Box->GetChildAt(0)) : nullptr)
				for (int32 i = 0; i < O->GetChildrenCount(); ++i) if (UBorder* Bar = Cast<UBorder>(O->GetChildAt(i))) Bar->SetBrush(S::Rounded(Want, 4.8f, FLinearColor(0, 0, 0, 0.95f), bMarker ? 2.5f : 1.5f));
		}
	}
	else Hide(YouChevron);
	// Lane A123 (2): with the middle kept clear the tag shows while a faceoff counts down and not after the drop; never on the puck.
	if ((bTagAtRing || bHead) && ShowYouTagLive(bMarker && ClearMiddleOn(), bCountingThisFrame, F.SimFrame, LastDropFrame) && !(bMarker && bTagOnPuck))
	{
		YouTag->SetBrush(S::Rounded(Team, 8.f, FLinearColor(0, 0, 0, 0.9f), 2.f));
		YouTag->SetVisibility(ESlateVisibility::HitTestInvisible);
		if (UCanvasPanelSlot* CS = Cast<UCanvasPanelSlot>(YouTag->Slot))
		{
			// Inside the ring's top arc: the tag's top edge 6 px under the arc (alignment 0.5, 0), else over the helmet (alignment 0.5, 1).
			CS->SetAlignment(FVector2D(0.5f, bTagAtRing ? 0.f : 1.f));
			CS->SetPosition(bTagAtRing ? Centre + FVector2D(0, -Size.Y * 0.5f + 6.f) : Head + FVector2D(0, ChevronAlpha > 0.f ? -(YouMarkerGapPx + YouMarkerSizePx + 4.f) : -8.f));
		}
	}
	else Hide(YouTag);

	// The puck carrier, when it is not you: a thin white ring (the sim's carrier when the game mode has one, else the skater on the puck).
	int32 Carrier = -1;
	const AHokejGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AHokejGameMode>() : nullptr;
	if (GM && GM->GetSim()) Carrier = GM->GetSim()->CarrierIndex();
	else
	{
		float Best = 1.2f;
		for (int32 i = 0; i < F.Skaters.Num(); ++i) { const float D = FVector2D::Distance(F.Skaters[i].PosM, F.PuckM); if (D < Best) { Best = D; Carrier = i; } }
	}
	if (Carrier >= 0 && Carrier != F.HumanIndex && F.Skaters.IsValidIndex(Carrier) && ProjectGroundRing(F.Skaters[Carrier].PosM, CarrierRingRadiusM, Centre, Size, DistM))
	{
		CarrierRing->SetColorAndOpacity(FLinearColor(S::Paper().R, S::Paper().G, S::Paper().B, 0.9f));
		CarrierRing->SetVisibility(ESlateVisibility::HitTestInvisible);
		if (UCanvasPanelSlot* CS = Cast<UCanvasPanelSlot>(CarrierRing->Slot)) { CS->SetPosition(Centre); CS->SetSize(Size); }
	}
	else Hide(CarrierRing);
}

void UJamHudWidget::PlaceMarkerArrow(UWidget* Arrow, const FVector2D& Raw, const FVector2D& Clamped, bool bBehind, float VW)
{
	if (!Arrow) return;
	const FVector2D D = Raw - Clamped;
	const bool bClamped = bBehind || !D.IsNearlyZero(0.5f);
	if (!bClamped) { Arrow->SetVisibility(ESlateVisibility::Collapsed); return; }
	// The chevron is built pointing down; -90 turns it right, 90 left, 180 up. Side edges win over the top (the net is beside us, not above).
	float Angle = 0.f;
	FVector2D Off(0, 22);
	if (bBehind || FMath::Abs(D.X) >= FMath::Abs(D.Y)) { const bool bRight = bBehind ? Clamped.X > VW * 0.5f : D.X > 0; Angle = bRight ? -90.f : 90.f; Off = FVector2D(bRight ? 22 : -22, 0); }
	else if (D.Y < 0) { Angle = 180.f; Off = FVector2D(0, -22); }
	Arrow->SetRenderTransformAngle(Angle);
	Arrow->SetVisibility(ESlateVisibility::HitTestInvisible);
	if (UCanvasPanelSlot* CS = Cast<UCanvasPanelSlot>(Arrow->Slot)) CS->SetPosition(Clamped + Off);
}

void UJamHudWidget::UpdateMarker(const FJamPresentationFrame& F)
{
	APlayerController* PC = GetOwningPlayer();
	FVector Net = F.GetFarNetWorldCm();
	const bool bP3 = CVarJamHudPass3.GetValueOnGameThread() != 0;
	if (bP3) Net.Z = NetCrossbarCm + NetBadgeLiftCm;   // lane A76: 1.2 m over the crossbar's centre (the frame's own Z is 3 m)
	float DistM = 0.f;
	if (F.Skaters.IsValidIndex(F.HumanIndex))
	{
		DistM = FVector2D::Distance(F.Skaters[F.HumanIndex].PosM, FVector2D(Net.X / 100.f, Net.Y / 100.f));
	}
	Marker.DistanceM = FMath::RoundToInt32(DistM);
	Marker.Text = FString::Printf(TEXT("%d m"), Marker.DistanceM);

	const FVector2D VS = UWidgetLayoutLibrary::GetViewportSize(this);
	const float Scale = FMath::Max(UWidgetLayoutLibrary::GetViewportScale(this), 0.01f);
	const float VW = VS.X / Scale, VH = VS.Y / Scale;

	FVector2D Sp(VW * 0.5f, VH * 0.5f);
	bool bProj = PC && UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, Net, Sp, false);
	// Behind the camera the projection is mirrored or fails: park the marker on the side edge the net lies towards.
	bool bBehind = !bProj;
	if (PC && PC->PlayerCameraManager)
	{
		const FVector Local = FRotationMatrix(PC->PlayerCameraManager->GetCameraRotation()).GetTransposed().TransformVector(Net - PC->PlayerCameraManager->GetCameraLocation());
		if (Local.X <= 1.f)
		{
			bBehind = true;
			Sp = FVector2D(Local.Y >= 0.f ? VW : 0.f, VH * 0.5f);
		}
	}
	else if (!bProj)
	{
		Sp = FVector2D(VW * 0.5f, VH * 0.5f);
	}
	// Lane A76 (PUCK-MARKER "not done", hud row "badge covers the crossbar" 3 of 3): the badge never stands on the net. The world point is 1.2 m over
	// the crossbar's centre (A64 aimed 3 m over the net mouth, which at 50 m is 60 px and inside the net's pixels once the clamp bit), and the badge
	// stands at least NetBadgeGapPx over the crossbar's own projection. Off screen the same element is the edge chevron (PlaceMarkerArrow).
	if (bP3 && !bBehind && PC && bProj)
	{
		FVector2D Bar;
		if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, FVector(Net.X, Net.Y, NetCrossbarCm), Bar, false))
			Sp.Y = FMath::Min(Sp.Y, Bar.Y - NetBadgeGapPx);
	}
	Marker.bOnScreen = !bBehind && Sp.X >= 0 && Sp.X <= VW && Sp.Y >= 0 && Sp.Y <= VH;
	if (bBehind && !PC)
	{
		Sp = FVector2D(VW * 0.5f, VH * 0.5f);
	}
	// Lane A42: MarkerEdgePx inside every edge plus the arrow's room on the sides; the top margin keeps the marker below the scoreboard,
	// the bottom one above the strips (the distance text hangs 18 + 30 px under the diamond).
	Marker.ScreenPos = ClampMarker(Sp, VW, VH, 44.f, bP3 ? 120.f : 150.f, 64.f);   // lane A76: top 150 -> 120 (the badge is already lifted off the net; scoreboard plates end near 104)

	if (UCanvasPanelSlot* DS = Cast<UCanvasPanelSlot>(NetDiamond->Slot))
	{
		DS->SetPosition(Marker.ScreenPos);
	}
	NetText->SetText(FText::FromString(Marker.Text));
	if (UCanvasPanelSlot* TS = Cast<UCanvasPanelSlot>(NetText->Slot))
	{
		// Lane A64 (eyes grid h19 finding 11, 34 stills): the distance hung 18 + 30 px under the diamond and printed over the net's crossbar and the end
		// boards. It now stands beside the diamond, on the side away from the nearer screen edge, at the diamond's height (3 m over the net mouth).
		const bool bLeft = Marker.ScreenPos.X > VW - 200.f;
		TS->SetAlignment(FVector2D(bLeft ? 1.f : 0.f, 0.5f));
		TS->SetPosition(Marker.ScreenPos + FVector2D(bLeft ? -24.f : 24.f, 0));
	}
	PlaceMarkerArrow(NetArrow, Sp, Marker.ScreenPos, bBehind, VW);
	// Lane A111 (B-to-A-0038 fault 6: a label sat over the goal sparks): no far-net badge during the goal pause.
	const bool bBadge = ShowNetBadges(F.Phase, CVarJamHudLabelPlates.GetValueOnGameThread() != 0);
	NetDiamond->SetVisibility(bBadge ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	NetText->SetVisibility(bBadge ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if (!bBadge && NetArrow) NetArrow->SetVisibility(ESlateVisibility::Collapsed);
	// Lane A123 (2): the label (a dark plate with the distance) is not drawn while it would stand in the middle third: the net is in front of the player then.
	if (bBadge && Marker.bOnScreen)
	{
		const bool bLeftSide = Marker.ScreenPos.X > VW - 200.f;
		const float LabelW = 12.f * Marker.Text.Len() + 2.f * LabelPlatePadX;
		const FVector2D LMin(Marker.ScreenPos.X + (bLeftSide ? -24.f - LabelW : 24.f), Marker.ScreenPos.Y - 15.f);
		if (!ShowNetLabel(ClearMiddleOn(), LMin, LMin + FVector2D(LabelW, 30.f), VW, VH)) NetText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UJamHudWidget::ShowToast(const FString& Text)
{
	if (!ToastText || !ToastPlate) return;
	ToastText->SetText(FText::FromString(Text));
	bToastPending = true;
	ToastStart = -1;
	ToastPlate->SetVisibility(ESlateVisibility::HitTestInvisible);
}

// Lane A101 (track F1): the contact line. The human's counters in the sim say what happened (a counter survives a frame the hit-stop held; a restart sets
// it back and shows nothing).
void UJamHudWidget::ShowContactLine(const FString& Text)
{
	if (!ContactText || !ContactPlate || Text.IsEmpty()) return;
	ContactText->SetText(FText::FromString(Text));
	bContactPending = true;
	ContactStart = -1;
	ContactPlate->SetRenderOpacity(1.f);
	ContactPlate->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UJamHudWidget::UpdateContactLine(const FJamPresentationFrame& F)
{
	if (!ContactPlate || !ContactText) return;
	const AHokejGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AHokejGameMode>() : nullptr;
	const UJamSim* Sim = GM ? GM->GetSim() : nullptr;
	if (Sim && Sim->GetSkaters().IsValidIndex(Sim->GetHumanIndex()))
	{
		const FJamSkaterState& Hu = Sim->GetSkaters()[Sim->GetHumanIndex()];
		const bool bBounced = Hu.ContactRecoils > ContactSeenRecoils;
		const bool bRefused = Hu.ChecksRefused > ContactSeenRefused;
		// Lane A113: the human's own soft hit, its hit on a skater that just got up, its whiff (counters, as the two above).
		const bool bSoft = Hu.ChecksSoft > ContactSeenSoft, bMissed = Hu.ChecksMissed > ContactSeenMissed, bGraced = Hu.ChecksGraced > ContactSeenGraced;
		ContactSeenRecoils = Hu.ContactRecoils;
		ContactSeenRefused = Hu.ChecksRefused;
		ContactSeenSoft = Hu.ChecksSoft;
		ContactSeenMissed = Hu.ChecksMissed;
		ContactSeenGraced = Hu.ChecksGraced;
		if (bBounced || bRefused || bSoft || bMissed || bGraced)
		{
			const FString Line = ContactLineFor(bRefused, bBounced, bSoft, bMissed, bGraced);
			ShowContactLine(Line);
			UE_LOG(LogTemp, Log, TEXT("JAMHUD frame=%lld contact_line=%s refused=%d recoils=%d soft=%d missed=%d graced=%d"), F.SimFrame, *Line.Replace(TEXT(" "), TEXT("_")), Hu.ChecksRefused, Hu.ContactRecoils,
				Hu.ChecksSoft, Hu.ChecksMissed, Hu.ChecksGraced);
		}
	}
	if (bContactPending) { ContactStart = F.SimFrame; bContactPending = false; }
	const int64 Age = F.SimFrame - ContactStart;
	const bool bShow = ContactStart >= 0 && Age >= 0 && Age < ContactLineFrames;
	if (!bShow) ContactStart = -1;
	ContactPlate->SetVisibility(bShow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if (bShow) ContactPlate->SetRenderOpacity(Age <= ContactLineFrames - ContactFadeFrames ? 1.f : static_cast<float>(ContactLineFrames - Age) / static_cast<float>(ContactFadeFrames));
}

FString UJamHudWidget::GetContactText() const { return ContactText && ContactPlate && ContactPlate->GetVisibility() != ESlateVisibility::Collapsed ? ContactText->GetText().ToString() : FString(); }

FString UJamHudWidget::GetToastText() const { return ToastText && ToastPlate && ToastPlate->GetVisibility() != ESlateVisibility::Collapsed ? ToastText->GetText().ToString() : FString(); }

FString UJamHudWidget::GetCardText() const { return CardTextBlock ? CardTextBlock->GetText().ToString() : FString(); }
FString UJamHudWidget::GetScoreText() const { return ScoreHome && ScoreAway ? ScoreHome->GetText().ToString() + TEXT(" : ") + ScoreAway->GetText().ToString() : FString(); }
FString UJamHudWidget::GetTimerText() const { return TimerText ? TimerText->GetText().ToString() : FString(); }
// Lane A123: the two lines of the clean plate read as the one line of before ("PERIOD 1: FIRST TO 3") to every caller.
FString UJamHudWidget::GetTakeText() const
{
	if (!TakeText) return FString();
	const bool bRule = GoalRuleText && GoalRuleText->GetVisibility() != ESlateVisibility::Collapsed && !GoalRuleText->GetText().IsEmpty();
	return bRule ? TakeText->GetText().ToString() + TEXT(": ") + GoalRuleText->GetText().ToString() : TakeText->GetText().ToString();
}

void UJamHudWidget::UpdateOwnMarker(const FJamPresentationFrame& F)
{
	APlayerController* PC = GetOwningPlayer();
	const FVector Own(-F.AttackSign * (F.RinkLengthM * 0.5f - F.NetInsetM) * 100.f, 0.f, 150.f);
	float DistM = 0.f;
	if (F.Skaters.IsValidIndex(F.HumanIndex))
	{
		DistM = FVector2D::Distance(F.Skaters[F.HumanIndex].PosM, FVector2D(Own.X / 100.f, 0.f));
	}
	OwnMarker.DistanceM = FMath::RoundToInt32(DistM);
	OwnMarker.Text = FString::Printf(TEXT("own %d m"), OwnMarker.DistanceM);
	const FVector2D VS = UWidgetLayoutLibrary::GetViewportSize(this);
	const float Scale = FMath::Max(UWidgetLayoutLibrary::GetViewportScale(this), 0.01f);
	const float VW = VS.X / Scale, VH = VS.Y / Scale;
	FVector2D Sp(VW * 0.5f, VH * 0.5f);
	bool bProj = PC && UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, Own, Sp, false);
	bool bBehind = !bProj;
	if (PC && PC->PlayerCameraManager)
	{
		const FVector Local = FRotationMatrix(PC->PlayerCameraManager->GetCameraRotation()).GetTransposed().TransformVector(Own - PC->PlayerCameraManager->GetCameraLocation());
		if (Local.X <= 1.f)
		{
			bBehind = true;
			Sp = FVector2D(Local.Y >= 0.f ? VW : 0.f, VH * 0.5f);  // behind us: park it on the side edge towards the net
		}
	}
	OwnMarker.bOnScreen = !bBehind && Sp.X >= 0 && Sp.X <= VW && Sp.Y >= 0 && Sp.Y <= VH;
	// Lane A42: the own-net marker was flush on the screen edge (eyes pass); it stays MarkerEdgePx inside with an arrow towards the net.
	// The bottom room (56) keeps its caption above the ready pill (bottom-left, 40 px tall at the 24 px edge).
	OwnMarker.ScreenPos = ClampMarker(Sp, VW, VH, 40.f, 150.f, 56.f);
	if (UCanvasPanelSlot* DS = Cast<UCanvasPanelSlot>(OwnDiamond->Slot)) { DS->SetPosition(OwnMarker.ScreenPos); }
	OwnText->SetText(FText::FromString(OwnMarker.Text));
	if (UCanvasPanelSlot* TS = Cast<UCanvasPanelSlot>(OwnText->Slot)) { TS->SetPosition(OwnMarker.ScreenPos + FVector2D(0, 10)); }
	PlaceMarkerArrow(OwnArrow, Sp, OwnMarker.ScreenPos, bBehind, VW);
	const bool bBadge = ShowNetBadges(F.Phase, CVarJamHudLabelPlates.GetValueOnGameThread() != 0);   // lane A111: as the far net's
	OwnDiamond->SetVisibility(bBadge ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	OwnText->SetVisibility(bBadge ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if (!bBadge && OwnArrow) OwnArrow->SetVisibility(ESlateVisibility::Collapsed);
	if (bBadge && OwnMarker.bOnScreen)   // lane A123 (2): as the far net's label
	{
		const float LabelW = 10.f * OwnMarker.Text.Len() + 2.f * LabelPlatePadX;
		const FVector2D LMin(OwnMarker.ScreenPos.X - LabelW * 0.5f, OwnMarker.ScreenPos.Y + 10.f);
		if (!ShowNetLabel(ClearMiddleOn(), LMin, LMin + FVector2D(LabelW, 28.f), VW, VH)) OwnText->SetVisibility(ESlateVisibility::Collapsed);
	}
}
