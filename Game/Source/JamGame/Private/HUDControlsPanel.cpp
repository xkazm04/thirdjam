// Lane A222 (track U3; B-to-A-0044, the operator's order of 15 Nov 10:50, verbatim: "Improve visually panel to show key bindings for player and keep
// it visible throughout the game, switchable in settings to take off permanently"). The controls panel, in the front end's own look (FJamMenuLook):
// a dark translucent panel with the menu's corner and its two team colours along the top edge, one row per verb: a light keycap with the key's short
// name in the menu's type, the verb in caps beside it. Bottom left on the safe margin, all match, every match and mode, while play runs. It replaces
// the 8-second controls strip and the bottom-left pill (one legend, not three); SETTINGS' CONTROLS PANEL OFF takes all three away for good.
// Presentation only: reads the bindings, the sim's possession (JamControls::HasPuckForButtons) and the human's buttons; writes nothing back.
#include "HUDWidget.h"
#include "MenuStyle.h"
#include "MenuWidget.h"
#include "InputMapping.h"
#include "SettingsWidget.h"
#include "DirectorsCallWidget.h"
#include "HokejGameMode.h"
#include "RadarWidget.h"
#include "JamSim.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "UObject/UObjectIterator.h"
#include "UnrealClient.h"

static TAutoConsoleVariable<int32> CVarJamHudControlsPanel(TEXT("jam.hud.ControlsPanel"), 1,
	TEXT("Lane A222: 1 (default) the controls panel bottom left all match (SETTINGS' CONTROLS PANEL switches it off for good), no 8-second strip, no pill; 0 = the screen of before (the strip in the first match, the bottom-left pill)."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHudPanelHoldFrames(TEXT("jam.hud.PanelHoldFrames"), 12,
	TEXT("Lane A222 wave 2: the controls panel swaps to WITHOUT THE PUCK only once \"without\" has held this many sim frames (or at once on an LMB / RMB press), and no swap follows another inside it unless a press forces it; gaining the puck swaps at once. 0 = the half follows possession every frame (PANEL READY)."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHudPanelStills(TEXT("jam.hud.PanelStills"), 0,
	TEXT("Lane A222 proof: 1 takes a still on the frames of a possession swap (0 to 6 after it, the fade), on the first frame a row lights, and 30 frames into a pad / keys change: a222-panel-<what>-f<frame>.png. 0 off."), ECVF_Default);

namespace
{
	bool GJamControlsPanelSetting = true;
	bool GJamControlsPanelSettingRead = false;
	float GJamControlsPanelTop = 0.f;   // the shown panel's top above the bottom edge, units (0 = not shown)

	using L = FJamMenuLook;

	FString FirstName(const TArray<FKey>& Keys)
	{
		if (Keys.Num() == 0) return TEXT("-");
		if (Keys[0] == EKeys::MouseScrollUp || Keys[0] == EKeys::MouseScrollDown) return TEXT("WHEEL");
		return UJamInputMap::KeyName(Keys[0]);
	}

	FSlateFontInfo CapFont() { return L::Font(L::Caption, FName("Black"), 40); }
	FSlateFontInfo WordFont() { return L::Font(L::Caption + 2, FName("Bold"), L::TrackLabel); }
	FSlateFontInfo HeadFont() { return L::Font(L::Caption - 2, FName("Bold"), L::TrackLabel); }
	FLinearColor CapFill() { const FLinearColor C = L::Text(); return FLinearColor(C.R, C.G, C.B, 0.92f); }
	FLinearColor PanelFill() { const FLinearColor C = L::Panel(); return FLinearColor(C.R, C.G, C.B, 0.78f); }
}

// ---------------------------------------------------------------- the rule and the setting

void UJamHudWidget::FControlsPanelHeld::Set(EJamVerb V) { DownMask |= 1u << static_cast<uint32>(V); }
bool UJamHudWidget::FControlsPanelHeld::Has(EJamVerb V) const { return (DownMask & (1u << static_cast<uint32>(V))) != 0; }

bool UJamHudWidget::ControlsPanelRuleOn() { return CVarJamHudControlsPanel.GetValueOnGameThread() != 0; }

bool UJamHudWidget::IsControlsPanelSettingOn()
{
	if (!GJamControlsPanelSettingRead) { GJamControlsPanelSettingRead = true; GJamControlsPanelSetting = FJamSettings::Load().bControlsPanel; }
	return GJamControlsPanelSetting;
}

void UJamHudWidget::SetControlsPanelSetting(bool bOn)
{
	GJamControlsPanelSettingRead = true;
	if (GJamControlsPanelSetting != bOn) UE_LOG(LogTemp, Log, TEXT("JAMHUD panel setting=%s"), bOn ? TEXT("on") : TEXT("off"));
	GJamControlsPanelSetting = bOn;
}

float UJamHudWidget::ControlsPanelTopNow() { return GJamControlsPanelTop; }

int32 UJamHudWidget::PanelHoldFrames() { return FMath::Max(0, CVarJamHudPanelHoldFrames.GetValueOnGameThread()); }

bool UJamHudWidget::FControlsPanelSwap::Step(bool bWith, bool bPress, int64 Frame, int32 Hold)
{
	if (Frame < LastFrame) *this = FControlsPanelSwap();   // a restart
	LastFrame = Frame;
	const int32 Truth = bWith ? 1 : 0;
	if (Truth == 1) LossStart = -1; else if (LossStart < 0) LossStart = Frame;
	if (Shown < 0) { Shown = Truth; return false; }
	if (Truth == Shown) return false;
	bool bGo;
	if (Hold <= 0 || bPress) bGo = true;                  // the panel never lies about a press he is making
	else if (Frame - LastSwap < Hold) bGo = false;        // no swap inside the hold after a swap
	else if (Truth == 1) bGo = true;                      // gaining: at once
	else bGo = Frame - LossStart >= Hold;                 // losing: once "without" has held Hold frames
	if (!bGo) return false;
	Shown = Truth;
	LastSwap = Frame;
	return true;
}

UJamHudWidget::ELegend UJamHudWidget::LegendNow() { return LegendFor(ControlsPanelRuleOn(), IsControlsPanelSettingOn()); }

FAutoConsoleCommand GJamUiPanel(TEXT("jam.ui.panel"), TEXT("Lane A222 (QA): jam.ui.panel 0|1 sets SETTINGS' CONTROLS PANEL for this process (as the row does, without the save)."),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& A) { if (A.Num()) UJamHudWidget::SetControlsPanelSetting(A[0].ToBool() || A[0] == TEXT("1")); }));
FAutoConsoleCommand GJamUiPadLast(TEXT("jam.ui.padlast"), TEXT("Lane A222 (QA): jam.ui.padlast 1|0 notes the pad (1) or the keys (0) as the device touched last: the words follow it."),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& A) { if (A.Num()) UJamInputMap::NoteInputDevice(A[0] == TEXT("1")); }));
FAutoConsoleCommand GJamUiRebind(TEXT("jam.ui.rebind"), TEXT("Lane A222 (QA): jam.ui.rebind <VerbId> <KeyName> rebinds as the CONTROLS screen does, in this process only (no save); jam.ui.rebind reset gives the defaults."),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& A)
	{
		if (A.Num() == 1 && A[0] == TEXT("reset")) { UJamInputMap::Get()->ApplyBindings(FJamBindings::Defaults()); return; }
		if (A.Num() < 2) return;
		FJamBindings B = UJamInputMap::Get()->GetBindings();
		for (int32 I = 0; I < FJamBindings::NumVerbs; ++I)
		{
			if (A[0] != FJamBindings::VerbId(static_cast<EJamVerb>(I))) continue;
			const bool bOk = B.Rebind(static_cast<EJamVerb>(I), FKey(*A[1]));
			UE_LOG(LogTemp, Log, TEXT("JAMHUD panel rebind %s %s ok=%d"), *A[0], *A[1], bOk ? 1 : 0);
			if (bOk) UJamInputMap::Get()->ApplyBindings(B);
		}
	}));

static FAutoConsoleCommandWithWorldAndArgs GJamUiHold(TEXT("jam.ui.hold"), TEXT("Lane A222 (QA): jam.ui.hold <KeyName> 1|0 presses or releases a key through the first player's own input stack (UJamPlayerInput::DebugInjectKey), as a hand does: the panel's row lights."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& A, UWorld* World)
	{
		if (A.Num() < 2 || !World) return;
		APlayerController* PC = World->GetFirstPlayerController();
		const FKey K(*A[0]);
		if (!PC || !K.IsValid()) return;
		UJamPlayerInput::DebugInjectKey(PC, K, A[1] == TEXT("1") ? 1.f : 0.f, K.IsGamepadKey());
		UE_LOG(LogTemp, Log, TEXT("JAMHUD panel hold %s %s"), *A[0], *A[1]);
	}));

// ---------------------------------------------------------------- the rows (pure)

FString UJamHudWidget::ControlsPanelKey(const FJamBindings& B, EJamVerb V, bool bPad)
{
	if (V == EJamVerb::SkateForward)
	{
		if (bPad) return TEXT("L-STICK");
		const FString F = FirstName(B[EJamVerb::SkateForward].Keys), Le = FirstName(B[EJamVerb::SkateLeft].Keys), Ba = FirstName(B[EJamVerb::SkateBack].Keys), R = FirstName(B[EJamVerb::SkateRight].Keys);
		if (F.Len() == 1 && Le.Len() == 1 && Ba.Len() == 1 && R.Len() == 1) return F + Le + Ba + R;
		return FString::Printf(TEXT("%s %s %s %s"), *F, *Le, *Ba, *R).Left(UJamInputMap::KeyNameMax);
	}
	return FirstName(bPad ? B[V].Pads : B[V].Keys);
}

TArray<UJamHudWidget::FControlsPanelRow> UJamHudWidget::ControlsPanelRows(const FJamBindings& B, bool bPad, bool bWithPuck, bool bAbility, const FControlsPanelHeld& Held)
{
	TArray<FControlsPanelRow> Rows;
	auto Add = [&](EJamVerb V, const FString& Word, bool bPoss)
	{
		FControlsPanelRow R;
		R.Verb = V;
		R.Key = ControlsPanelKey(B, V, bPad);
		R.Word = Word;
		R.bLit = Held.Has(V);
		R.bPossession = bPoss;
		Rows.Add(R);
	};
	if (bWithPuck)
	{
		Add(EJamVerb::Shoot, FJamBindings::VerbLabel(EJamVerb::Shoot), true);
		Add(EJamVerb::Pass, Held.bSaucer && Held.Has(EJamVerb::Pass) ? FString(TEXT("SAUCER")) : FJamBindings::VerbLabel(EJamVerb::Pass), true);
		Add(EJamVerb::Deke, FJamBindings::VerbLabel(EJamVerb::Deke), true);
		Add(EJamVerb::Spin, FJamBindings::VerbLabel(EJamVerb::Spin), true);
	}
	else
	{
		Add(EJamVerb::CrossCheck, Held.bBraced && Held.Has(EJamVerb::CrossCheck) ? FString(TEXT("BRACED")) : FJamBindings::VerbLabel(EJamVerb::CrossCheck), true);
		Add(EJamVerb::Check, FJamBindings::VerbLabel(EJamVerb::Check), true);
		Add(EJamVerb::Poke, FJamBindings::VerbLabel(EJamVerb::Poke), true);
	}
	Add(EJamVerb::SkateForward, TEXT("SKATE"), false);
	Add(EJamVerb::Sprint, FJamBindings::VerbLabel(EJamVerb::Sprint), false);
	Add(EJamVerb::Brake, FJamBindings::VerbLabel(EJamVerb::Brake), false);
	if (bAbility) Add(EJamVerb::Ability, FJamBindings::VerbLabel(EJamVerb::Ability), false);
	return Rows;
}

// ---------------------------------------------------------------- the widget

void UJamHudWidget::BuildControlsPanel()
{
	if (!WidgetTree || !Root) return;
	UWidgetTree* T = WidgetTree;
	IsControlsPanelSettingOn();   // the saved setting, read once a process (FJamSettings::Apply and the SETTINGS row keep it live after)
	CPanel = T->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("ControlsPanel"));
	CPanel->SetMaxDesiredWidth(ControlsPanelMaxW);
	CPanel->SetMinDesiredWidth(ControlsPanelMinW);
	CPanel->SetVisibility(ESlateVisibility::Collapsed);
	UVerticalBox* Outer = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	// The front end's panel: the two team colours on its top edge over the panel fill (FJamMenuLook::MakePanel), translucent over the ice.
	if (UVerticalBoxSlot* RS = Outer->AddChildToVerticalBox(L::MakeTeamRule(T, 3.f))) RS->SetHorizontalAlignment(HAlign_Fill);
	CPanelPlate = T->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ControlsPanelPlate"));
	CPanelPlate->SetBrush(L::FillBrush(PanelFill(), true));
	CPanelPlate->SetPadding(FMargin(ControlsPanelPad, ControlsPanelPad - 4.f, ControlsPanelPad, ControlsPanelPad));
	if (UVerticalBoxSlot* PS = Outer->AddChildToVerticalBox(CPanelPlate)) PS->SetHorizontalAlignment(HAlign_Fill);
	UVerticalBox* Body = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	CPanelPlate->SetContent(Body);

	// The head: a dot in his team's colour and whether the puck is his.
	UHorizontalBox* Head = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	CPanelDot = T->ConstructWidget<UBorder>(UBorder::StaticClass());
	CPanelDot->SetBrush(FSlateBrush(FSlateRoundedBoxBrush(FLinearColor::White, 5.f)));
	USizeBox* DotBox = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	DotBox->SetWidthOverride(10.f);
	DotBox->SetHeightOverride(10.f);
	DotBox->AddChild(CPanelDot);
	if (UHorizontalBoxSlot* DS = Head->AddChildToHorizontalBox(DotBox)) { DS->SetVerticalAlignment(VAlign_Center); DS->SetPadding(FMargin(2.f, 0, 8.f, 0)); }
	CPanelHead = L::MakeText(T, TEXT("WITH THE PUCK"), HeadFont(), L::TextDim());
	if (UHorizontalBoxSlot* HS = Head->AddChildToHorizontalBox(CPanelHead)) HS->SetVerticalAlignment(VAlign_Center);
	if (UVerticalBoxSlot* S = Body->AddChildToVerticalBox(Head)) S->SetPadding(FMargin(0, 0, 0, 6.f));

	// The rows: the possession half (at most 4) in its own box (it fades in on a swap), a hairline, the always half (at most 4).
	CPanelPossBox = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	Body->AddChildToVerticalBox(CPanelPossBox);
	UBorder* Hair = T->ConstructWidget<UBorder>(UBorder::StaticClass());
	Hair->SetBrushColor(L::PanelEdge());
	USizeBox* HairBox = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	HairBox->SetHeightOverride(1.f);
	HairBox->AddChild(Hair);
	if (UVerticalBoxSlot* S = Body->AddChildToVerticalBox(HairBox)) { S->SetHorizontalAlignment(HAlign_Fill); S->SetPadding(FMargin(0, 2.f, 0, 6.f)); }
	UVerticalBox* Always = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	Body->AddChildToVerticalBox(Always);
	for (int32 I = 0; I < ControlsPanelMaxRows; ++I)
	{
		UHorizontalBox* Row = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		UBorder* Cap = T->ConstructWidget<UBorder>(UBorder::StaticClass());
		Cap->SetBrush(L::FillBrush(CapFill(), false));
		Cap->SetPadding(FMargin(L::Unit, 0.f));
		Cap->SetHorizontalAlignment(HAlign_Center);
		Cap->SetVerticalAlignment(VAlign_Center);
		UTextBlock* CapText = L::MakeText(T, TEXT(""), CapFont(), L::Ink());
		Cap->SetContent(CapText);
		USizeBox* CapBox = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		CapBox->SetHeightOverride(ControlsPanelKeycapH);
		CapBox->SetMinDesiredWidth(ControlsPanelKeycapMinW);
		CapBox->AddChild(Cap);
		if (UHorizontalBoxSlot* CS = Row->AddChildToHorizontalBox(CapBox)) CS->SetVerticalAlignment(VAlign_Center);
		UTextBlock* Word = L::MakeText(T, TEXT(""), WordFont(), L::Text());
		if (UHorizontalBoxSlot* WS = Row->AddChildToHorizontalBox(Word)) { WS->SetVerticalAlignment(VAlign_Center); WS->SetPadding(FMargin(10.f, 0, 0, 0)); }
		UVerticalBox* Into = I < 4 ? CPanelPossBox.Get() : Always;
		if (UVerticalBoxSlot* S = Into->AddChildToVerticalBox(Row)) S->SetPadding(FMargin(0, 0, 0, ControlsPanelRowGap));
		Row->SetVisibility(ESlateVisibility::Collapsed);
		CPanelRowW.Add(Row);
		CPanelCap.Add(Cap);
		CPanelCapText.Add(CapText);
		CPanelWord.Add(Word);
		CPanelPainted.Add(FString());
	}
	CPanel->AddChild(Outer);
	UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(CPanel);
	// Bottom left on the safe margin, where the pill stood: the radar and the ability slot are bottom right, the score top centre.
	PanelSlot->SetAnchors(FAnchors(0, 1, 0, 1));
	PanelSlot->SetAlignment(FVector2D(0, 1));
	PanelSlot->SetAutoSize(true);
	PanelSlot->SetPosition(FVector2D(FJamUiStyle::EdgePx, -FJamUiStyle::EdgePx));
}

bool UJamHudWidget::IsControlsPanelShown() const { return CPanel && CPanel->GetVisibility() != ESlateVisibility::Collapsed; }

void UJamHudWidget::ApplyControlsPanel(const FControlsPanelInput& In)
{
	if (!CPanel) return;
	const bool bShow = LegendNow() == ELegend::Panel && ShowControlsPanel(In.Phase, In.bPauseCardOrPaused, In.bGoalBannerUp);
	if (bShow != bCPanelShown) UE_LOG(LogTemp, Log, TEXT("JAMHUD frame=%lld panel=%s"), In.SimFrame, bShow ? TEXT("shown") : TEXT("hidden"));
	bCPanelShown = bShow;
	CPanel->SetVisibility(bShow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if (!bShow) { CPanelRowsShown.Reset(); GJamControlsPanelTop = 0.f; return; }
	GJamControlsPanelTop = FJamUiStyle::EdgePx + ControlsPanelReserveH;   // the tallest panel's top: what stands over it never hops with the rows
	const bool bPad = UJamInputMap::IsPadLast();
	// The swap: on the frame possession changes the possession half starts from nothing and fades in over ControlsPanelFadeFrames.
	// Wave 2: the half shown (FControlsPanelSwap): gaining at once, losing after the hold or on a press; the fade starts on the swap.
	if (In.SimFrame < CPanelSwapFrame) CPanelSwapFrame = -1000;   // a restart
	// The proof's count: every change of possession as the buttons read it (what PANEL READY swapped on), beside the swaps the panel draws.
	if (CPanelSwap.Shown >= 0 && CPanelSwap.LastFrame >= 0 && In.SimFrame != CPanelSwap.LastFrame && (In.bWithPuck ? 1 : 0) != CPanelTruth)
		UE_LOG(LogTemp, Log, TEXT("JAMHUD frame=%lld panel truth with=%d"), In.SimFrame, In.bWithPuck ? 1 : 0);
	CPanelTruth = In.bWithPuck ? 1 : 0;
	if (CPanelSwap.Step(In.bWithPuck, In.bPress, In.SimFrame, PanelHoldFrames()))
	{
		UE_LOG(LogTemp, Log, TEXT("JAMHUD frame=%lld panel swap with=%d held=%lld press=%d"), In.SimFrame, CPanelSwap.Shown, In.SimFrame - CPanelSwapFrame, In.bPress ? 1 : 0);
		CPanelPrevHeld = In.SimFrame - CPanelSwapFrame;
		CPanelSwapFrame = In.SimFrame;
	}
	const int32 With = CPanelSwap.Shown;
	const bool bShowWith = With == 1;
	CPanelWith = With;
	const int64 Since = In.SimFrame - CPanelSwapFrame;
	CPanelRowsShown = ControlsPanelRows(UJamInputMap::Get()->GetBindings(), bPad, bShowWith, In.bAbility, In.Held);
	CPanelSwapAlpha = FMath::Clamp(static_cast<float>(Since + 1) / static_cast<float>(ControlsPanelFadeFrames), 0.f, 1.f);
	CPanelPossBox->SetRenderOpacity(CPanelSwapAlpha);
	CPanelPlate->SetRenderOpacity((In.bDown && UJamInputMap::WordsTonight()) || In.bBoxed ? BadgeDownOpacity : 1.f);   // lane A227: boxed dims too   // the pill's rule: he lies on the ice, the buttons do nothing
	const FString Head = bShowWith ? TEXT("WITH THE PUCK") : TEXT("WITHOUT THE PUCK");
	if (CPanelHead->GetText().ToString() != Head) CPanelHead->SetText(FText::FromString(Head));
	CPanelDot->SetBrushColor(In.TeamColour);
	CPanelHead->SetColorAndOpacity(FSlateColor(bShowWith ? L::Text() : L::TextDim()));
	// Paint the rows into their slots: the possession rows into the first four, the always rows into the last four.
	int32 Poss = 0, Alw = 4;
	TArray<int32> SlotOf;
	for (const FControlsPanelRow& R : CPanelRowsShown) SlotOf.Add(R.bPossession ? Poss++ : Alw++);
	FString Logged;
	for (int32 I = 0; I < ControlsPanelMaxRows; ++I)
	{
		const int32 RowIx = SlotOf.IndexOfByKey(I);
		if (RowIx == INDEX_NONE)
		{
			if (CPanelRowW[I]->GetVisibility() != ESlateVisibility::Collapsed) CPanelRowW[I]->SetVisibility(ESlateVisibility::Collapsed);
			CPanelPainted[I].Reset();
			continue;
		}
		const FControlsPanelRow& R = CPanelRowsShown[RowIx];
		const bool bTeach = (In.Teach == 0 && R.Verb == EJamVerb::Pass) || (In.Teach == 1 && R.Verb == EJamVerb::Check);
		const int32 Look = R.bLit ? 2 : bTeach ? 1 : (R.Verb == EJamVerb::Sprint && In.bLegsSpent) ? 3 : 0;   // lane A227: 3 = the spent legs' dimmed word
		const FString Paint = FString::Printf(TEXT("%s|%s|%d"), *R.Key, *R.Word, Look);
		Logged += FString::Printf(TEXT("%s%s=%s%s"), Logged.IsEmpty() ? TEXT("") : TEXT(","), *R.Key, *R.Word, R.bLit ? TEXT("*") : TEXT(""));
		if (CPanelRowW[I]->GetVisibility() == ESlateVisibility::Collapsed) CPanelRowW[I]->SetVisibility(ESlateVisibility::HitTestInvisible);
		if (Paint == CPanelPainted[I]) continue;
		CPanelPainted[I] = Paint;
		CPanelCapText[I]->SetText(FText::FromString(R.Key));
		CPanelWord[I]->SetText(FText::FromString(R.Word));
		// Lit: the menu's focus (the away orange fill, white type), as an item a pad stands on; the teaching pulse: the take-yellow keycap.
		const FLinearColor Fill = Look == 2 ? L::Focus() : Look == 1 ? FJamUiStyle::Yellow() : CapFill();
		CPanelCap[I]->SetBrush(L::FillBrush(Fill, false));
		CPanelCapText[I]->SetColorAndOpacity(FSlateColor(Look == 2 ? L::TextOnFocus() : L::Ink()));
		CPanelWord[I]->SetColorAndOpacity(FSlateColor(Look == 2 ? L::TextOnFocus() : Look == 1 ? FJamUiStyle::Yellow() : Look == 3 ? L::TextDim() : L::Text()));
	}
	if (Logged != CPanelLogged)
	{
		UE_LOG(LogTemp, Log, TEXT("JAMHUD frame=%lld panel with=%d pad=%d rows=%s"), In.SimFrame, With, bPad ? 1 : 0, *Logged);
		CPanelLogged = Logged;
	}
}

void UJamHudWidget::UpdateControlsPanel(const FJamPresentationFrame& F)
{
	if (!CPanel) return;
	const AHokejGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AHokejGameMode>() : nullptr;
	const UJamSim* Sim = GM ? GM->GetSim() : nullptr;
	FControlsPanelInput In;
	In.Phase = F.Phase;
	In.SimFrame = F.SimFrame;
	if (!CPanelPause.IsValid() && (++CPanelSearch % 30) == 1)
	{
		for (TObjectIterator<UJamDirectorsCallWidget> It; It; ++It) if (It->GetWorld() == GetWorld() && !It->HasAnyFlags(RF_ClassDefaultObject)) { CPanelPause = *It; break; }
	}
	In.bPauseCardOrPaused = (GetWorld() && GetWorld()->IsPaused()) || (CPanelPause.IsValid() && CPanelPause->IsOpen());
	In.bGoalBannerUp = GoalBannerPlate && GoalBannerPlate->GetVisibility() != ESlateVisibility::Collapsed;
	if (!Sim || !Sim->GetSkaters().IsValidIndex(F.HumanIndex)) { In.Phase = EJamPhase::Paused; ApplyControlsPanel(In); return; }
	In.bWithPuck = JamControls::HasPuckForButtons(*Sim, F.HumanIndex);
	In.bAbility = !F.HumanAbility.IsEmpty();
	In.bDown = bHumanDownNow;
	In.TeamId = F.Skaters.IsValidIndex(F.HumanIndex) ? F.Skaters[F.HumanIndex].TeamId : 0;
	In.TeamColour = F.GetTeamColour(In.TeamId);
	In.Teach = TeachShown;
	// What is down: the two-role buttons as the human's last step resolved them (the hold's words: the pill's thresholds), every other row by its keys.
	const FJamButtonVerbs& Btn = JamControls::LastHumanButtons();
	if (Btn.bShootHeld) In.Held.Set(EJamVerb::Shoot);
	if (Btn.bPassHeld) In.Held.Set(EJamVerb::Pass);
	if (Btn.bCrossHeld || Btn.bLift) In.Held.Set(EJamVerb::CrossCheck);
	if (Btn.bCheck) In.Held.Set(EJamVerb::Check);
	const int32 SaucerWord = UJamInputMap::WordsTonight() && Sim->PassPowerOn() ? 0 : JamControls::SaucerHoldFrames(Sim->GetConfig().Pass.SaucerHalfFrames);
	In.Held.bSaucer = SaucerWord > 0 && Btn.PassHoldSteps >= SaucerWord;
	In.Held.bBraced = JamControls::BraceHoldFrames() > 0 && Btn.CrossHoldSteps >= JamControls::BraceHoldFrames();
	APlayerController* PC = GetOwningPlayer();
	const FJamBindings& B = UJamInputMap::Get()->GetBindings();
	auto Down = [PC](const TArray<FKey>& Keys)
	{
		if (!PC) return false;
		for (const FKey& K : Keys) if (K.IsAnalog() ? PC->GetInputAnalogKeyState(K) > 0.5f : PC->IsInputKeyDown(K)) return true;
		return false;
	};
	// Wave 2: a two-role button down this frame (the step's own press or level, or the key itself) forces the half to the truth.
	In.bPress = Btn.bShootHeld || Btn.bPassHeld || Btn.bCrossHeld || Btn.bCheck || Btn.bLift || Btn.bShootPress || Btn.bPassPress;
	for (const EJamVerb V : { EJamVerb::Shoot, EJamVerb::Pass, EJamVerb::CrossCheck, EJamVerb::Check }) In.bPress |= Down(B[V].Keys) || Down(B[V].Pads);
	for (const EJamVerb V : { EJamVerb::Shoot, EJamVerb::Pass, EJamVerb::CrossCheck, EJamVerb::Check })
	{
		// A two-role key down shows on the verb it is doing now (the possession half lights only its own verb).
		const bool bRole = FJamBindings::Role(V) == 1 ? In.bWithPuck : !In.bWithPuck;
		if (bRole && (Down(B[V].Keys) || Down(B[V].Pads))) In.Held.Set(V);
	}
	for (const EJamVerb V : { EJamVerb::Poke, EJamVerb::Deke, EJamVerb::Spin, EJamVerb::Ability, EJamVerb::Sprint, EJamVerb::Brake })
		if (Down(B[V].Keys) || Down(B[V].Pads)) In.Held.Set(V);
	bool bSkate = false;
	for (const EJamVerb V : { EJamVerb::SkateForward, EJamVerb::SkateBack, EJamVerb::SkateLeft, EJamVerb::SkateRight }) bSkate |= Down(B[V].Keys);
	if (APawn* P = GetOwningPlayerPawn())
	{
		if (const UJamPlayerInput* Pi = P->FindComponentByClass<UJamPlayerInput>())
		{
			bSkate |= !Pi->GetAxis().IsNearlyZero(0.2f);
			if (Pi->IsSprintHeld()) In.Held.Set(EJamVerb::Sprint);
			if (Pi->IsBrakeHeld()) In.Held.Set(EJamVerb::Brake);
		}
	}
	if (bSkate) In.Held.Set(EJamVerb::SkateForward);
	// Lane A227: the SPRINT row carries the stamina (jam.hud.SprintOnIce 0): lit while he sprints, dimmed while the legs are spent.
	if (!SprintOnIceOn())
	{
		In.Held.DownMask &= ~(1u << static_cast<uint32>(EJamVerb::Sprint));
		if (F.bHumanSprinting) In.Held.Set(EJamVerb::Sprint);
		In.bLegsSpent = LegsSpent(F.HumanStamina01, F.bHumanSprinting);
	}
	// Lane A227: in the box no row is lit and the panel dims (jam.cam.BoxFollowsPlay 1).
	if (BoxFollowsPlayOn() && Sim->GetSkaters()[F.HumanIndex].InBox()) { In.bBoxed = true; In.Held = FControlsPanelHeld(); }
	const uint32 LitBefore = [this]() { uint32 M = 0; for (const FControlsPanelRow& R : CPanelRowsShown) if (R.bLit) M |= 1u << static_cast<uint32>(R.Verb); return M; }();
	const int32 WithBefore = CPanelWith;
	ApplyControlsPanel(In);
	// The proof's stills (jam.hud.PanelStills 1): the swap frame by frame, the first frame a row lights.
	if (CVarJamHudPanelStills.GetValueOnGameThread() != 0 && bCPanelShown)
	{
		// At most two swaps filmed (each frame of its fade) and the first light of each word, a process.
		static int64 LastShot = -1;
		static int32 SwapsFilmed = 0;
		static TSet<FString> LitFilmed;
		const int64 Since = F.SimFrame - CPanelSwapFrame;
		FString What;
		if (WithBefore >= 0 && Since >= 0 && Since <= ControlsPanelFadeFrames && SwapsFilmed < 2 && CPanelPrevHeld >= 60)
		{
			What = FString::Printf(TEXT("swap%d-s%lld"), CPanelWith, Since);
			if (Since == ControlsPanelFadeFrames) ++SwapsFilmed;
		}
		else
		{
			uint32 LitNow = 0;
			for (const FControlsPanelRow& R : CPanelRowsShown) if (R.bLit) LitNow |= 1u << static_cast<uint32>(R.Verb);
			const uint32 New = LitNow & ~LitBefore & ~(1u << static_cast<uint32>(EJamVerb::SkateForward));
			if (New) for (const FControlsPanelRow& R : CPanelRowsShown)
				if ((New & (1u << static_cast<uint32>(R.Verb))) && !LitFilmed.Contains(R.Word)) { LitFilmed.Add(R.Word); What = TEXT("lit-") + R.Word.Replace(TEXT(" "), TEXT("")); break; }
		}
		if (!What.IsEmpty() && F.SimFrame != LastShot)
		{
			LastShot = F.SimFrame;
			FScreenshotRequest::RequestScreenshot(FString::Printf(TEXT("a222-panel-%s-f%06lld.png"), *What, F.SimFrame), true, false);
		}
	}
}

// ---------------------------------------------------------------- boxes (tests and QA)

FBox2D UJamHudWidget::LayoutBoxOf(const UWidget* W, const FVector2D& View) const
{
	// Climb to the HUD canvas's own child; its slot and desired size give its box in View units.
	const UWidget* C = W;
	while (C && !(C->Slot && C->Slot->Parent.Get() == static_cast<UPanelWidget*>(Root.Get()))) C = C->GetParent();
	const UCanvasPanelSlot* S = C ? Cast<UCanvasPanelSlot>(C->Slot) : nullptr;
	if (!S) return FBox2D(ForceInit);
	const FAnchors A = S->GetAnchors();
	const FMargin O = S->GetOffsets();
	const FVector2D Al = S->GetAlignment();
	const FVector2D Desired = C->GetDesiredSize();
	FVector2D Min, Max;
	for (int32 Ax = 0; Ax < 2; ++Ax)
	{
		const double A0 = Ax == 0 ? A.Minimum.X : A.Minimum.Y, A1 = Ax == 0 ? A.Maximum.X : A.Maximum.Y, V = Ax == 0 ? View.X : View.Y;
		const double Lo = Ax == 0 ? O.Left : O.Top, Hi = Ax == 0 ? O.Right : O.Bottom, Ali = Ax == 0 ? Al.X : Al.Y;
		double P0, P1;
		if (!FMath::IsNearlyEqual(A0, A1)) { P0 = A0 * V + Lo; P1 = A1 * V - Hi; }   // stretched: the offsets are insets
		else
		{
			const double Size = S->GetAutoSize() ? (Ax == 0 ? Desired.X : Desired.Y) : Hi;
			P0 = A0 * V + Lo - Ali * Size;
			P1 = P0 + Size;
		}
		if (Ax == 0) { Min.X = P0; Max.X = P1; } else { Min.Y = P0; Max.Y = P1; }
	}
	return FBox2D(Min, Max);
}

FBox2D UJamHudWidget::ControlsPanelBox(const FVector2D& View) const { return CPanel ? LayoutBoxOf(CPanel, View) : FBox2D(ForceInit); }

TArray<TPair<FString, FBox2D>> UJamHudWidget::CornerBoxesForTest(const FVector2D& View) const
{
	TArray<TPair<FString, FBox2D>> Out;
	if (Radar) Out.Add({ TEXT("radar"), LayoutBoxOf(Radar, View) });
	if (HomeBox) Out.Add({ TEXT("score"), LayoutBoxOf(HomeBox, View) });
	if (AbilityPlate) Out.Add({ TEXT("slot"), LayoutBoxOf(AbilityPlate, View) });
	if (CommentaryStrip) Out.Add({ TEXT("commentary"), LayoutBoxOf(CommentaryStrip, View) });
	if (StaminaBack) Out.Add({ TEXT("stamina"), LayoutBoxOf(StaminaBack, View) });
	if (CallText) Out.Add({ TEXT("call"), LayoutBoxOf(CallText, View) });
	if (ControlsStrip) Out.Add({ TEXT("old-strip"), LayoutBoxOf(ControlsStrip, View) });
	if (ButtonsBadge) Out.Add({ TEXT("old-pill"), LayoutBoxOf(ButtonsBadge, View) });
	return Out;
}
