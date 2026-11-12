#include "DirectorsCallWidget.h"
#include "Progression.h"
#include "DirectorsCallController.h"
#include "MenuStyle.h"
#include "MenuFlow.h"
#include "HokejGameMode.h"
#include "JamCartridge.h"
#include "JamJson.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/WidgetSwitcher.h"
#include "Components/SizeBox.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/PlayerController.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

using namespace JamMenuStyle;
using S = FJamUiStyle;

// Lane A18 (visual layer): the pause card and the call card sit on the generated call-card frame (T_UI_CallCard; a slate plate with a
// clapper stripe without it); the scoreboard line is a pill under the score; type from FJamUiStyle. Logic is lane A8's and A15's.

// ---------------------------------------------------------------- tunables

double FJamCallTunable::FromNormalized(float V) const
{
	double R = Min + (Max - Min) * FMath::Clamp(static_cast<double>(V), 0.0, 1.0);
	if (bInteger) R = FMath::RoundToDouble(R);
	return FMath::Clamp(R, Min, Max);
}

void UJamCallSlider::Bind(UJamDirectorsCallWidget* InOwner, int32 InRow)
{
	Owner = InOwner;
	Row = InRow;
	OnValueChanged.AddUniqueDynamic(this, &UJamCallSlider::HandleChanged);
}

void UJamCallSlider::HandleChanged(float V)
{
	if (Owner) Owner->OnSliderChanged(Row, V);
}

TArray<FJamCallTunable> UJamDirectorsCallWidget::PickTunables(const FString& CartridgeJson, int32 Max)
{
	TArray<FJamCallTunable> All;
	FString Err;
	TSharedPtr<FJamJson> Root = FJamJson::Parse(CartridgeJson, Err);
	if (!Root || !Root->IsObject()) return All;
	TSharedPtr<FJamJson> Tunables = Root->Get(TEXT("tunables"));
	TSharedPtr<FJamJson> Values = Root->Get(TEXT("values"));
	if (!Tunables || !Tunables->IsArray()) return All;
	for (const TSharedPtr<FJamJson>& T : Tunables->Array)
	{
		if (!T || !T->IsObject()) continue;
		TSharedPtr<FJamJson> K = T->Get(TEXT("key")), Type = T->Get(TEXT("type")), Mn = T->Get(TEXT("min")), Mx = T->Get(TEXT("max")), Def = T->Get(TEXT("default")), Desc = T->Get(TEXT("description"));
		if (!K || !K->IsString() || !Type || !Type->IsString()) continue;
		if (Type->String != TEXT("number") && Type->String != TEXT("integer")) continue;
		FJamCallTunable C;
		C.Key = FName(*K->String);
		C.Label = UJamMenuFlow::LabelFor(K->String);
		C.Description = Desc && Desc->IsString() ? Desc->String : FString();
		C.Min = Mn && Mn->IsNumber() ? Mn->Number : 0.0;
		C.Max = Mx && Mx->IsNumber() ? Mx->Number : 1.0;
		C.bInteger = Type->String == TEXT("integer");
		C.Current = Def && Def->IsNumber() ? Def->Number : C.Min;
		if (Values && Values->IsObject())
		{
			if (TSharedPtr<FJamJson> V = Values->Get(K->String)) if (V->IsNumber()) C.Current = V->Number;
		}
		All.Add(C);
	}
	// The brief's five, in order; a pickup tunable is whatever key says "pickup".
	TArray<FJamCallTunable> Out;
	auto Take = [&](const TFunction<bool(const FJamCallTunable&)>& Pred)
	{
		for (const FJamCallTunable& C : All)
		{
			if (Out.Num() >= Max) return;
			if (Pred(C) && !Out.ContainsByPredicate([&](const FJamCallTunable& O) { return O.Key == C.Key; })) { Out.Add(C); return; }
		}
	};
	Take([](const FJamCallTunable& C) { return C.Key == TEXT("skater_speed"); });
	Take([](const FJamCallTunable& C) { return C.Key == TEXT("shot_speed"); });
	Take([](const FJamCallTunable& C) { return C.Key == TEXT("ice_friction"); });
	Take([](const FJamCallTunable& C) { return C.Key.ToString().Contains(TEXT("pickup")); });
	Take([](const FJamCallTunable& C) { return C.Key == TEXT("ai_red_skill"); });
	for (const FJamCallTunable& C : All)
	{
		if (Out.Num() >= Max) break;
		if (!Out.ContainsByPredicate([&](const FJamCallTunable& O) { return O.Key == C.Key; })) Out.Add(C);
	}
	return Out;
}

FString UJamDirectorsCallWidget::AnnouncementLine(const FString& Announcement)
{
	FString Rest = Announcement.TrimStartAndEnd();
	const FString Prefix(TEXT("director's call:"));
	if (Rest.Left(Prefix.Len()).ToLower() == Prefix) Rest = Rest.Mid(Prefix.Len()).TrimStartAndEnd();
	return FString::Printf(TEXT("DIRECTOR'S CALL: %s"), *Rest);
}

FString UJamDirectorsCallWidget::RuleLine(const FString& Text)
{
	FString Rest = Text.TrimStartAndEnd();
	const FString Prefix(TEXT("rule:"));
	if (Rest.Left(Prefix.Len()).ToLower() == Prefix) Rest = Rest.Mid(Prefix.Len()).TrimStartAndEnd();
	return FString::Printf(TEXT("RULE: %s"), *Rest);
}

// ---------------------------------------------------------------- pause menu and call card

UJamDirectorsCallWidget* UJamDirectorsCallWidget::CreateAndAdd(APlayerController* PC, UJamDirectorsCallController* InOwner)
{
	if (!PC) return nullptr;
	UJamDirectorsCallWidget* W = CreateWidget<UJamDirectorsCallWidget>(PC, UJamDirectorsCallWidget::StaticClass());
	if (W)
	{
		W->Owner = InOwner;
		W->SetIsFocusable(true);
		W->AddToViewport(20);
		W->SetVisibility(ESlateVisibility::Collapsed);
	}
	return W;
}

UJamDesignerSubsystem* UJamDirectorsCallWidget::Designer() const
{
	if (Owner.IsValid()) return Owner->GetDesigner();
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UJamDesignerSubsystem>() : nullptr;
}

TSharedRef<SWidget> UJamDirectorsCallWidget::RebuildWidget()
{
	BuildTree();
	return Super::RebuildWidget();
}

void UJamDirectorsCallWidget::BuildTree()
{
	if (!WidgetTree) WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	if (WidgetTree->RootWidget) return;
	UWidgetTree* T = WidgetTree;

	UOverlay* Root = T->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("PauseRoot"));
	T->RootWidget = Root;
	UBorder* Dimmer = Plate(T, FLinearColor(0, 0, 0, 0.62f), FMargin(0));
	S::AddO(Root, Dimmer);

	USizeBox* CardBox = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	CardBox->SetWidthOverride(800.f);
	S::AddO(Root, CardBox, HAlign_Center, VAlign_Center);
	const bool bFrameArt = S::Art(TEXT("T_UI_CallCard")) != nullptr;
	// The generated frame carries its own clapper stripe (the top 7 %), so the body starts under it.
	UBorder* Card = S::Plate(T, S::CallCard(), bFrameArt ? FMargin(28, 58, 28, 26) : FMargin(0, 0, 0, 18));
	CardBox->AddChild(Card);
	UVerticalBox* Body = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	Card->SetContent(Body);
	if (!bFrameArt) AddV(Body, Clapper(T, 10, 14), FMargin(0));

	Switcher = T->ConstructWidget<UWidgetSwitcher>(UWidgetSwitcher::StaticClass(), TEXT("PauseScreens"));
	AddV(Body, Switcher, bFrameArt ? FMargin(0) : FMargin(26, 10, 26, 0));

	// Screen 0: the pause menu.
	UVerticalBox* Pause = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("PauseScreen"));
	UTextBlock* Paused = S::Text(T, TEXT("TIME OUT"), S::DisplayFont(), S::Chalk());
	Paused->SetJustification(ETextJustify::Center);
	AddV(Pause, Paused, FMargin(0));
	TakeText = S::Text(T, TEXT("PERIOD 1"), S::LabelFont(), S::Yellow());
	TakeText->SetJustification(ETextJustify::Center);
	AddV(Pause, TakeText, FMargin(0, -4, 0, 10));
	UTextBlock* Txt = nullptr;
	ResumeButton = Button(T, TEXT("RESUME"), S::Label, Txt);
	ResumeButton->OnClicked.AddDynamic(this, &UJamDirectorsCallWidget::OnResumeClicked);
	Tinted.Add(TPair<TWeakObjectPtr<UButton>, TWeakObjectPtr<UTextBlock>>(ResumeButton, Txt));
	AddV(Pause, ResumeButton, FMargin(120, 4));
	UTextBlock* CallTxt = nullptr;
	CallButton = Button(T, TEXT("DIRECTOR'S CALL"), S::Label, CallTxt);
	CallButtonText = CallTxt;
	CallButton->OnClicked.AddDynamic(this, &UJamDirectorsCallWidget::OnCallClicked);
	Tinted.Add(TPair<TWeakObjectPtr<UButton>, TWeakObjectPtr<UTextBlock>>(CallButton, CallTxt));
	AddV(Pause, CallButton, FMargin(120, 4));
	RestartButton = Button(T, TEXT("RESTART"), S::Label, Txt);
	RestartButton->OnClicked.AddDynamic(this, &UJamDirectorsCallWidget::OnRestartClicked);
	Tinted.Add(TPair<TWeakObjectPtr<UButton>, TWeakObjectPtr<UTextBlock>>(RestartButton, Txt));
	AddV(Pause, RestartButton, FMargin(120, 4));
	QuitButton = Button(T, TEXT("QUIT TO MENU"), S::Label, Txt);
	QuitButton->OnClicked.AddDynamic(this, &UJamDirectorsCallWidget::OnQuitClicked);
	Tinted.Add(TPair<TWeakObjectPtr<UButton>, TWeakObjectPtr<UTextBlock>>(QuitButton, Txt));
	AddV(Pause, QuitButton, FMargin(120, 4));
	PauseStatus = S::Text(T, TEXT(""), S::BodyFont(), S::Yellow());
	PauseStatus->SetJustification(ETextJustify::Center);
	PauseStatus->SetAutoWrapText(true);
	AddV(Pause, PauseStatus, FMargin(0, 10, 0, 0));
	// Lane A49: the controls card on the pause menu (the same table as UJamInputMap::BindingTableText and docs/experiments/CONTROLS.md).
	UTextBlock* Controls = S::Text(T,
		TEXT("SKATE  WASD / L-STICK      AIM STICK  MOUSE / R-STICK      SHOOT  HOLD LMB / RT      CHECK  RMB / RB\n")
		TEXT("CROSS-CHECK  MMB, MOUSE4 / LB      DEKE  WHEEL, Q / Y      SPRINT  SHIFT / LT      POKE  SPACE / A\n")
		TEXT("PASS  E / X      SIGNALS  1-4 / D-PAD      BRAKE  CTRL / B      RESTART  R / VIEW      PAUSE  ESC / MENU"), S::CaptionFont(), S::Chalk());
	Controls->SetJustification(ETextJustify::Center);
	AddV(Pause, Controls, FMargin(0, 12, 0, 0));
	UTextBlock* Keys = S::Text(T, TEXT("ESC / START  resume      STICK  move      A  select      B  back"), S::CaptionFont(), S::Muted());
	Keys->SetJustification(ETextJustify::Center);
	AddV(Pause, Keys, FMargin(0, 8, 0, 0));
	Switcher->AddChild(Pause);

	// Screen 1: the Director's Call card.
	UVerticalBox* Call = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CallScreen"));
	// Kept clear of the frame's megaphone doodle (top right): symmetric side padding and a smaller black face.
	CallTitle = S::Text(T, TEXT("DIRECTOR'S CALL"), S::Font(28, FName("Black"), S::TrackTitle), S::Yellow());
	CallTitle->SetJustification(ETextJustify::Center);
	AddV(Call, CallTitle, FMargin(30, 0, 150, 0));
	UTextBlock* Rule = S::Text(T, TEXT("One change per period. Move one slider, then APPLY. The scoreboard announces the new rule."), S::BodyFont(), S::Muted());
	Rule->SetJustification(ETextJustify::Center);
	Rule->SetAutoWrapText(true);
	AddV(Call, Rule, FMargin(30, 0, 150, 10));
	RowsBox = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CallRows"));
	AddV(Call, RowsBox, FMargin(0));
	CallStatus = S::Text(T, TEXT(""), S::BodyFont(), S::Yellow());
	CallStatus->SetAutoWrapText(true);
	AddV(Call, CallStatus, FMargin(0, 6, 0, 4));
	UHorizontalBox* Actions = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	ApplyButton = Button(T, TEXT("APPLY"), S::Label, Txt);
	ApplyButton->OnClicked.AddDynamic(this, &UJamDirectorsCallWidget::OnApplyClicked);
	Tinted.Add(TPair<TWeakObjectPtr<UButton>, TWeakObjectPtr<UTextBlock>>(ApplyButton, Txt));
	AddH(Actions, ApplyButton, FMargin(0, 0, 10, 0));
	BackButton = Button(T, TEXT("BACK"), S::Label, Txt);
	BackButton->OnClicked.AddDynamic(this, &UJamDirectorsCallWidget::OnBackClicked);
	Tinted.Add(TPair<TWeakObjectPtr<UButton>, TWeakObjectPtr<UTextBlock>>(BackButton, Txt));
	AddH(Actions, BackButton, FMargin(0));
	AddV(Call, Actions, FMargin(0, 4, 0, 0), HAlign_Center);
	Switcher->AddChild(Call);
	Switcher->SetActiveWidgetIndex(0);
}

void UJamDirectorsCallWidget::RebuildRows()
{
	if (!RowsBox) return;
	RowsBox->ClearChildren();
	Rows.Reset();
	UJamDesignerSubsystem* D = Designer();
	FString Json;
	if (D && D->GetCurrentCartridge()) Json = D->GetCurrentCartridge()->RawJson;
	if (Json.IsEmpty() && Owner.IsValid()) if (AHokejGameMode* GM = Owner->GetGameMode()) if (GM->GetCartridge()) Json = GM->GetCartridge()->RawJson;
	Tunables = PickTunables(Json, 5);
	UWidgetTree* T = WidgetTree;
	for (int32 i = 0; i < Tunables.Num(); ++i)
	{
		const FJamCallTunable& C = Tunables[i];
		FRow R;
		UVerticalBox* RowBox = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		UHorizontalBox* Head = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		R.Label = S::Text(T, C.Label.ToUpper(), S::Font(S::Body, FName("Bold"), S::TrackLabel), S::Chalk());
		AddH(Head, R.Label, FMargin(0, 0, 10, 0), true);
		R.Value = S::Text(T, ValueText(C, C.Current), S::Font(S::Body + 2, FName("Bold")), S::Yellow());
		AddH(Head, R.Value, FMargin(0));
		AddV(RowBox, Head, FMargin(0));
		UJamCallSlider* Sl = T->ConstructWidget<UJamCallSlider>(UJamCallSlider::StaticClass());
		Sl->RequiresControllerLock = false;
		Sl->IsFocusable = true;
		Sl->SetWidgetStyle(S::SliderStyle());
		Sl->SetStepSize(C.bInteger && C.Max > C.Min ? static_cast<float>(1.0 / (C.Max - C.Min)) : 0.02f);
		Sl->SetValue(C.ToNormalized(C.Current));
		Sl->Bind(this, i);
		R.Slider = Sl;
		R.Start = Sl->GetValue();
		AddV(RowBox, Sl, FMargin(0, 2, 0, 0));
		UTextBlock* Desc = S::Text(T, FString::Printf(TEXT("%s  (%s .. %s)"), *C.Description, *FJamJson::NumberToString(C.Min), *FJamJson::NumberToString(C.Max)), S::CaptionFont(), S::Muted());
		Desc->SetAutoWrapText(true);
		AddV(RowBox, Desc, FMargin(0, 0, 0, 2));
		AddV(RowsBox, RowBox, FMargin(0, 4));
		Rows.Add(R);
	}
	if (!Tunables.Num()) CallStatus->SetText(FText::FromString(TEXT("This match has no numbers to call.")));
}

FString UJamDirectorsCallWidget::ValueText(const FJamCallTunable& C, double V) const
{
	if (C.Key == TEXT("goal_pause_frames") || C.Key.ToString().EndsWith(TEXT("_frames"))) return FString::Printf(TEXT("%s  (%.1f s)"), *FJamJson::NumberToString(V), V / 60.0);
	return FJamJson::NumberToString(V);
}

void UJamDirectorsCallWidget::OnSliderChanged(int32 Row, float V)
{
	if (!Rows.IsValidIndex(Row)) return;
	const FJamCallTunable& C = Tunables[Row];
	Rows[Row].Value->SetText(FText::FromString(ValueText(C, C.FromNormalized(V))));
	// One change per take: moving a slider puts the others back where they were.
	for (int32 i = 0; i < Rows.Num(); ++i)
	{
		if (i != Row && Rows[i].Slider && !FMath::IsNearlyEqual(Rows[i].Slider->GetValue(), Rows[i].Start, 1e-4f))
		{
			Rows[i].Slider->SetValue(Rows[i].Start);
			Rows[i].Value->SetText(FText::FromString(ValueText(Tunables[i], Tunables[i].Current)));
		}
	}
}

void UJamDirectorsCallWidget::Open()
{
	RefreshState();
	ShowPause();
	SetVisibility(ESlateVisibility::Visible);
}

void UJamDirectorsCallWidget::Close()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

bool UJamDirectorsCallWidget::IsOpen() const
{
	return GetVisibility() == ESlateVisibility::Visible;
}

void UJamDirectorsCallWidget::RefreshState()
{
	UJamDesignerSubsystem* D = Designer();
	const bool bAvailable = D && D->IsDirectorsCallAvailable();
	int32 Take = 1;
	if (Owner.IsValid()) if (AHokejGameMode* GM = Owner->GetGameMode()) Take = FMath::Max(1, GM->GetTake());
	if (TakeText) TakeText->SetText(FText::FromString(FString::Printf(TEXT("PERIOD %d"), Take)));
	if (CallTitle) CallTitle->SetText(FText::FromString(FString::Printf(TEXT("DIRECTOR'S CALL  -  PERIOD %d"), Take)));
	if (CallButton)
	{
		CallButton->SetIsEnabled(bAvailable);
		CallButtonText->SetText(FText::FromString(bAvailable ? TEXT("DIRECTOR'S CALL") : TEXT("DIRECTOR'S CALL  (used this period)")));
	}
	if (PauseStatus) PauseStatus->SetText(FText::FromString(bAvailable ? TEXT("Your call is ready: change one thing about this match.") : LastAnnouncement));
	if (ApplyButton) ApplyButton->SetIsEnabled(bAvailable);
	for (FRow& R : Rows) if (R.Slider) R.Slider->SetIsEnabled(bAvailable);
}

void UJamDirectorsCallWidget::ShowPause()
{
	bOnCall = false;
	RefreshState();
	if (Switcher) Switcher->SetActiveWidgetIndex(0);
	PendingFocus = ResumeButton.Get();
}

void UJamDirectorsCallWidget::ShowCall()
{
	bOnCall = true;
	RebuildRows();
	RefreshState();
	if (CallStatus && Tunables.Num()) CallStatus->SetText(FText::GetEmpty());
	if (Switcher) Switcher->SetActiveWidgetIndex(1);
	PendingFocus = Rows.Num() ? static_cast<UWidget*>(Rows[0].Slider) : static_cast<UWidget*>(BackButton.Get());
}

FJamDirectorsCall UJamDirectorsCallWidget::ApplySelected()
{
	FJamDirectorsCall None;
	for (int32 i = 0; i < Rows.Num(); ++i)
	{
		if (Rows[i].Slider && !FMath::IsNearlyEqual(Rows[i].Slider->GetValue(), Rows[i].Start, 1e-4f))
		{
			return ApplyValue(Tunables[i].Key, Tunables[i].FromNormalized(Rows[i].Slider->GetValue()));
		}
	}
	None.Reason = TEXT("move one slider first");
	if (CallStatus) CallStatus->SetText(FText::FromString(TEXT("Move one slider first.")));
	return None;
}

FJamDirectorsCall UJamDirectorsCallWidget::ApplyValue(FName Key, double Value)
{
	FJamDirectorsCall Call;
	UJamDesignerSubsystem* D = Designer();
	if (!D) { Call.Reason = TEXT("no Designer"); return Call; }
	Call = D->DirectorsCall(Key, Value);
	AfterCall(Call);
	return Call;
}

void UJamDirectorsCallWidget::AfterCall(const FJamDirectorsCall& Call)
{
	if (Call.bApplied)
	{
		LastAnnouncement = AnnouncementLine(Call.Announcement);
		if (CallStatus) CallStatus->SetText(FText::FromString(LastAnnouncement));
		ShowPause();
	}
	else if (CallStatus)
	{
		CallStatus->SetText(FText::FromString(FString::Printf(TEXT("Not applied: %s"), *Call.Reason)));
	}
}

void UJamDirectorsCallWidget::OnResumeClicked() { if (Owner.IsValid()) Owner->Resume(); }
void UJamDirectorsCallWidget::OnCallClicked() { ShowCall(); }
void UJamDirectorsCallWidget::OnRestartClicked() { if (Owner.IsValid()) Owner->RestartMatch(); }
void UJamDirectorsCallWidget::OnQuitClicked() { if (Owner.IsValid()) Owner->QuitToMenu(); }
void UJamDirectorsCallWidget::OnApplyClicked() { ApplySelected(); }
void UJamDirectorsCallWidget::OnBackClicked() { ShowPause(); }

void UJamDirectorsCallWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (PendingFocus.IsValid())
	{
		PendingFocus->SetFocus();
		PendingFocus = nullptr;
	}
	for (const TPair<TWeakObjectPtr<UButton>, TWeakObjectPtr<UTextBlock>>& P : Tinted)
	{
		if (P.Key.IsValid() && P.Value.IsValid()) Tint(P.Key.Get(), P.Value.Get());
	}
	for (FRow& R : Rows)
	{
		if (!R.Slider || !R.Label) continue;
		const bool bHot = R.Slider->HasAnyUserFocus() || R.Slider->IsHovered();
		R.Label->SetColorAndOpacity(FSlateColor(bHot ? S::Yellow() : S::Chalk()));
	}
}

FReply UJamDirectorsCallWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey K = InKeyEvent.GetKey();
	if (K == EKeys::Escape || K == EKeys::Gamepad_Special_Right)
	{
		if (Owner.IsValid()) Owner->Resume();
		return FReply::Handled();
	}
	if (IsBackKey(K))
	{
		if (bOnCall) ShowPause();
		else if (Owner.IsValid()) Owner->Resume();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

// ---------------------------------------------------------------- overlay

UJamDirectorsCallHudWidget* UJamDirectorsCallHudWidget::CreateAndAdd(APlayerController* PC)
{
	if (!PC) return nullptr;
	UJamDirectorsCallHudWidget* W = CreateWidget<UJamDirectorsCallHudWidget>(PC, UJamDirectorsCallHudWidget::StaticClass());
	if (W) W->AddToViewport(5);
	return W;
}

TSharedRef<SWidget> UJamDirectorsCallHudWidget::RebuildWidget()
{
	BuildTree();
	return Super::RebuildWidget();
}

namespace
{
	// The pills around the scoreboard line and the ready hint are the text's parent; show and hide both together.
	void SetLine(UTextBlock* Txt, bool bVisible)
	{
		if (!Txt) return;
		const ESlateVisibility V = bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
		Txt->SetVisibility(V);
		if (UWidget* P = Txt->GetParent()) P->SetVisibility(V);
	}
}

void UJamDirectorsCallHudWidget::BuildTree()
{
	if (!WidgetTree) WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	if (WidgetTree->RootWidget) return;
	UWidgetTree* T = WidgetTree;
	Root = T->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("OverlayRoot"));
	T->RootWidget = Root;
	auto Place = [&](UWidget* W, FVector2D Anchor, FVector2D Align, FVector2D Pos)
	{
		UCanvasPanelSlot* CS = Root->AddChildToCanvas(W);
		CS->SetAnchors(FAnchors(Anchor.X, Anchor.Y, Anchor.X, Anchor.Y));
		CS->SetAlignment(Align);
		CS->SetPosition(Pos);
		CS->SetAutoSize(true);
	};
	// The period card, where lane A5 draws its win/lose card (never at the same time: this one shows only in play).
	// R-0021 point 3 (lane A47, B-to-A-0030 fault 2): a plain banner on A18's panel, "MATCH n" over "PERIOD n - FACEOFF"; the clapperboard art
	// (T_UI_MatchCard) stays in the menu's world. TakeArt is the plate so the show / hide logic is unchanged.
	UBorder* Banner = S::Plate(T, S::Panel(0.92f), FMargin(44, 18, 44, 22));
	UVerticalBox* BannerBody = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("BannerBody"));
	Banner->SetContent(BannerBody);
	TakeCard = S::IceText(T, TEXT(""), S::Font(77, FName("Black"), S::TrackTitle, true), S::Chalk());
	TakeCard->SetJustification(ETextJustify::Center);
	TakeSub = S::IceText(T, TEXT("FACEOFF"), S::LabelFont(true), S::Yellow());
	TakeSub->SetJustification(ETextJustify::Center);
	if (UVerticalBoxSlot* VS = BannerBody->AddChildToVerticalBox(TakeCard)) { VS->SetHorizontalAlignment(HAlign_Center); VS->SetPadding(FMargin(0)); }
	if (UVerticalBoxSlot* VS = BannerBody->AddChildToVerticalBox(TakeSub)) { VS->SetHorizontalAlignment(HAlign_Center); VS->SetPadding(FMargin(0, 2, 0, 0)); }
	TakeArt = Banner;
	TakeArt->SetVisibility(ESlateVisibility::Collapsed);
	// Lane A42 (A48's storyboard finding 3): the banner sits in the top band under the scoreboard (centre at y MatchCardCentreY), so the
	// human and the countdown ring at 0.66 stay clear for its 150 frames.
	Place(TakeArt, FVector2D(0.5, 0), FVector2D(0.5, 0.5), FVector2D(0, MatchCardCentreY));
	// The scoreboard line: a pill directly under the scoreboard (whose plates end near y 104 at 1080p).
	UBorder* LinePill = S::Plate(T, S::Pill(0.86f), FMargin(18, 6));
	LinePill->SetVisibility(ESlateVisibility::Collapsed);
	Announcement = S::IceText(T, TEXT(""), S::Font(S::Label - 2, FName("Bold"), 30), S::Yellow());
	Announcement->SetVisibility(ESlateVisibility::Collapsed);
	LinePill->SetContent(Announcement);
	Place(LinePill, FVector2D(0.5, 0), FVector2D(0.5, 0), FVector2D(0, 112));
	UBorder* HintPill = S::Plate(T, S::Pill(0.8f), FMargin(14, 5));
	HintPill->SetVisibility(ESlateVisibility::Collapsed);
	ReadyHint = S::IceText(T, TEXT("ESC / START   CALL READY"), S::CaptionFont(), S::Yellow());   // R-0021 point 3 (lane A47): the menu keeps the film words
	ReadyHint->SetVisibility(ESlateVisibility::Collapsed);
	HintPill->SetContent(ReadyHint);
	Place(HintPill, FVector2D(0, 1), FVector2D(0, 1), FVector2D(S::EdgePx, -S::EdgePx));
}

void UJamDirectorsCallHudWidget::ShowTake(int32 Take, int64 UntilFrame)
{
	if (!TakeCard) return;
	TakeUntil = UntilFrame;
	// R-0021 point 3: a match card. The big number is the season's match (1 outside a season); the small type carries the season's
	// count, the period (Take is lane A1's period number) and FACEOFF; lane A17's season note replaces that line on the first period.
	int32 Match = 1, Count = 0;
	FString Note;
	const AHokejGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AHokejGameMode>() : nullptr;
	if (const UJamProgression* P = UJamProgression::Get(this); P && GM && GM->GetCartridge())
	{
		const FString Path = GM->GetCartridge()->SourcePath;
		if (!P->ActiveTitle(Path).IsEmpty())
		{
			const FJamSeasonState St = P->GetSeasonState();
			if (St.bLoaded) { Match = (St.ActiveTake >= 0 ? St.ActiveTake : St.CurrentTake) + 1; Count = St.TakeCount; }
		}
		if (Take <= 1) Note = P->ActiveNote(Path);
	}
	TakeCard->SetText(FText::FromString(FString::Printf(TEXT("MATCH %d"), Match)));
	const FString Line = Count > 0 ? FString::Printf(TEXT("OF %d   -   PERIOD %d   -   FACEOFF"), Count, FMath::Max(1, Take)) : FString::Printf(TEXT("PERIOD %d   -   FACEOFF"), FMath::Max(1, Take));
	TakeSub->SetText(FText::FromString(Note.IsEmpty() ? Line : FString::Printf(TEXT("COACH'S NOTE: %s"), *Note)));
	// A note needs reading time: the card stays twice as long (2 x TakeCardFrames).
	if (!Note.IsEmpty()) TakeUntil = UntilFrame + UJamDirectorsCallController::TakeCardFrames;
	TakeCard->SetVisibility(ESlateVisibility::HitTestInvisible);
	TakeSub->SetVisibility(ESlateVisibility::HitTestInvisible);
	if (TakeArt) TakeArt->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UJamDirectorsCallHudWidget::ShowAnnouncement(const FString& Line, int64 UntilFrame)
{
	if (!Announcement) return;
	Announcement->SetText(FText::FromString(Line));
	AnnouncementUntil = UntilFrame;
	// Lane A15's "RULE: ..." shares the slot: paper on a blue-edged pill; the Director's Call stays take-yellow on the plain pill.
	const bool bRule = Line.StartsWith(TEXT("RULE:"));
	Announcement->SetColorAndOpacity(FSlateColor(bRule ? S::Paper() : S::Yellow()));
	if (UBorder* Pill = Cast<UBorder>(Announcement->GetParent()))
	{
		Pill->SetBrush(bRule ? S::Rounded(FLinearColor(S::Ink().R, S::Ink().G, S::Ink().B, 0.86f), S::Radius, S::Blue(), 2.f) : S::Pill(0.86f));
	}
	SetLine(Announcement, true);
}

void UJamDirectorsCallHudWidget::TickFrame(const FJamPresentationFrame& F, bool bCallReady)
{
	if (!TakeCard) return;
	// A restart (R, the pause menu, the season's next take) resets the simulation frame. The opening TAKE card of the new match was
	// requested against the old count (the controller's CurrentSimFrame still read it), so a card with frames left is rebased onto
	// the new count instead of cleared; the scoreboard line belonged to the old match and ends now.
	if (F.SimFrame < LastFrame)
	{
		AnnouncementUntil = -1;
		if (TakeUntil > LastFrame)
		{
			TakeUntil = F.SimFrame + (TakeUntil - LastFrame);   // requested against the old count: keep what is left
		}
		else if (!(IsTakeCardVisible() && TakeUntil > F.SimFrame))
		{
			// No fresh request (the restart's period event is not relayed, or came before the reset): the overlay opens the
			// new match's card itself from the feed's take number, for the usual TakeCardFrames.
			ShowTake(F.Take, F.SimFrame + UJamDirectorsCallController::TakeCardFrames);
		}
	}
	LastFrame = F.SimFrame;
	const bool bEnd = F.Phase == EJamPhase::Win || F.Phase == EJamPhase::Lose;
	const bool bTake = !bEnd && TakeUntil >= 0 && F.SimFrame < TakeUntil;
	TakeCard->SetVisibility(bTake ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	TakeSub->SetVisibility(bTake ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if (TakeArt) TakeArt->SetVisibility(bTake ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	const bool bLine = AnnouncementUntil >= 0 && F.SimFrame < AnnouncementUntil;
	SetLine(Announcement, bLine);
	SetLine(ReadyHint, bCallReady && !bEnd && !bLine);
}

FString UJamDirectorsCallHudWidget::GetTakeCardText() const
{
	if (!TakeCard) return FString();
	// With the slate art the text block holds only the number; the card still reads "MATCH n" to callers.
	// Lane A47's plain banner holds the whole "MATCH n" line; only the old slate art held the bare number.
	const FString Text = TakeCard->GetText().ToString();
	return (TakeArt && !Text.StartsWith(TEXT("MATCH"))) ? FString::Printf(TEXT("MATCH %s"), *Text) : Text;
}
FString UJamDirectorsCallHudWidget::GetAnnouncementText() const { return Announcement ? Announcement->GetText().ToString() : FString(); }
bool UJamDirectorsCallHudWidget::IsTakeCardVisible() const { return TakeCard && TakeCard->GetVisibility() != ESlateVisibility::Collapsed; }
