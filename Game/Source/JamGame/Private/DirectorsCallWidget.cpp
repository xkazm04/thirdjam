#include "DirectorsCallWidget.h"
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
#include "GameFramework/PlayerController.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

using namespace JamMenuStyle;

// ---------------------------------------------------------------- tunables

double FJamCallTunable::FromNormalized(float S) const
{
	double V = Min + (Max - Min) * FMath::Clamp(static_cast<double>(S), 0.0, 1.0);
	if (bInteger) V = FMath::RoundToDouble(V);
	return FMath::Clamp(V, Min, Max);
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
	UOverlaySlot* DS = Root->AddChildToOverlay(Dimmer);
	DS->SetHorizontalAlignment(HAlign_Fill);
	DS->SetVerticalAlignment(VAlign_Fill);

	USizeBox* CardBox = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	CardBox->SetWidthOverride(780.f);
	UOverlaySlot* CS = Root->AddChildToOverlay(CardBox);
	CS->SetHorizontalAlignment(HAlign_Center);
	CS->SetVerticalAlignment(VAlign_Center);
	UBorder* Card = Plate(T, Slate(), FMargin(0, 0, 0, 16));
	CardBox->AddChild(Card);
	UVerticalBox* Body = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	Card->SetContent(Body);
	AddV(Body, Clapper(T, 10, 14), FMargin(0));

	Switcher = T->ConstructWidget<UWidgetSwitcher>(UWidgetSwitcher::StaticClass(), TEXT("PauseScreens"));
	AddV(Body, Switcher, FMargin(26, 10, 26, 0));

	// Screen 0: the pause menu.
	UVerticalBox* Pause = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("PauseScreen"));
	UTextBlock* Paused = Text(T, TEXT("CUT!"), 54, Chalk());
	Paused->SetJustification(ETextJustify::Center);
	AddV(Pause, Paused, FMargin(0));
	TakeText = Text(T, TEXT("TAKE 1"), 22, Yellow());
	TakeText->SetJustification(ETextJustify::Center);
	AddV(Pause, TakeText, FMargin(0, -6, 0, 8));
	UTextBlock* Txt = nullptr;
	ResumeButton = Button(T, TEXT("RESUME"), 24, Txt);
	ResumeButton->OnClicked.AddDynamic(this, &UJamDirectorsCallWidget::OnResumeClicked);
	Tinted.Add(TPair<TWeakObjectPtr<UButton>, TWeakObjectPtr<UTextBlock>>(ResumeButton, Txt));
	AddV(Pause, ResumeButton, FMargin(110, 4));
	UTextBlock* CallTxt = nullptr;
	CallButton = Button(T, TEXT("DIRECTOR'S CALL"), 24, CallTxt);
	CallButtonText = CallTxt;
	CallButton->OnClicked.AddDynamic(this, &UJamDirectorsCallWidget::OnCallClicked);
	Tinted.Add(TPair<TWeakObjectPtr<UButton>, TWeakObjectPtr<UTextBlock>>(CallButton, CallTxt));
	AddV(Pause, CallButton, FMargin(110, 4));
	RestartButton = Button(T, TEXT("RESTART"), 24, Txt);
	RestartButton->OnClicked.AddDynamic(this, &UJamDirectorsCallWidget::OnRestartClicked);
	Tinted.Add(TPair<TWeakObjectPtr<UButton>, TWeakObjectPtr<UTextBlock>>(RestartButton, Txt));
	AddV(Pause, RestartButton, FMargin(110, 4));
	QuitButton = Button(T, TEXT("QUIT TO MENU"), 24, Txt);
	QuitButton->OnClicked.AddDynamic(this, &UJamDirectorsCallWidget::OnQuitClicked);
	Tinted.Add(TPair<TWeakObjectPtr<UButton>, TWeakObjectPtr<UTextBlock>>(QuitButton, Txt));
	AddV(Pause, QuitButton, FMargin(110, 4));
	PauseStatus = Text(T, TEXT(""), 16, Yellow(), FName("Regular"));
	PauseStatus->SetJustification(ETextJustify::Center);
	PauseStatus->SetAutoWrapText(true);
	AddV(Pause, PauseStatus, FMargin(0, 8, 0, 0));
	UTextBlock* Keys = Text(T, TEXT("ESC / START  resume      STICK  move      A  select      B  back"), 13, Dim());
	Keys->SetJustification(ETextJustify::Center);
	AddV(Pause, Keys, FMargin(0, 8, 0, 0));
	Switcher->AddChild(Pause);

	// Screen 1: the Director's Call card.
	UVerticalBox* Call = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CallScreen"));
	CallTitle = Text(T, TEXT("DIRECTOR'S CALL"), 34, Yellow());
	CallTitle->SetJustification(ETextJustify::Center);
	AddV(Call, CallTitle, FMargin(0));
	UTextBlock* Rule = Text(T, TEXT("One change per take. Move one slider, then APPLY. The scoreboard announces the cut."), 15, Dim(), FName("Regular"));
	Rule->SetJustification(ETextJustify::Center);
	AddV(Call, Rule, FMargin(0, 0, 0, 8));
	RowsBox = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CallRows"));
	AddV(Call, RowsBox, FMargin(0));
	CallStatus = Text(T, TEXT(""), 16, Yellow(), FName("Regular"));
	CallStatus->SetAutoWrapText(true);
	AddV(Call, CallStatus, FMargin(0, 6, 0, 4));
	UHorizontalBox* Actions = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	ApplyButton = Button(T, TEXT("APPLY"), 22, Txt);
	ApplyButton->OnClicked.AddDynamic(this, &UJamDirectorsCallWidget::OnApplyClicked);
	Tinted.Add(TPair<TWeakObjectPtr<UButton>, TWeakObjectPtr<UTextBlock>>(ApplyButton, Txt));
	AddH(Actions, ApplyButton, FMargin(0, 0, 10, 0));
	BackButton = Button(T, TEXT("BACK"), 22, Txt);
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
		R.Label = Text(T, C.Label.ToUpper(), 18, Chalk());
		AddH(Head, R.Label, FMargin(0, 0, 10, 0), true);
		R.Value = Text(T, ValueText(C, C.Current), 18, Yellow());
		AddH(Head, R.Value, FMargin(0));
		AddV(RowBox, Head, FMargin(0));
		UJamCallSlider* S = T->ConstructWidget<UJamCallSlider>(UJamCallSlider::StaticClass());
		S->RequiresControllerLock = false;
		S->IsFocusable = true;
		S->SetStepSize(C.bInteger && C.Max > C.Min ? static_cast<float>(1.0 / (C.Max - C.Min)) : 0.02f);
		S->SetSliderBarColor(Dim());
		S->SetSliderHandleColor(Chalk());
		S->SetValue(C.ToNormalized(C.Current));
		S->Bind(this, i);
		R.Slider = S;
		R.Start = S->GetValue();
		AddV(RowBox, S, FMargin(0, 2, 0, 0));
		UTextBlock* Desc = Text(T, FString::Printf(TEXT("%s  (%s .. %s)"), *C.Description, *FJamJson::NumberToString(C.Min), *FJamJson::NumberToString(C.Max)), 12, Dim(), FName("Regular"));
		Desc->SetAutoWrapText(true);
		AddV(RowBox, Desc, FMargin(0, 0, 0, 2));
		AddV(RowsBox, RowBox, FMargin(0, 3));
		Rows.Add(R);
	}
	if (!Tunables.Num()) CallStatus->SetText(FText::FromString(TEXT("This cut has no number tunables to call.")));
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
	if (TakeText) TakeText->SetText(FText::FromString(FString::Printf(TEXT("TAKE %d"), Take)));
	if (CallTitle) CallTitle->SetText(FText::FromString(FString::Printf(TEXT("DIRECTOR'S CALL  -  TAKE %d"), Take)));
	if (CallButton)
	{
		CallButton->SetIsEnabled(bAvailable);
		CallButtonText->SetText(FText::FromString(bAvailable ? TEXT("DIRECTOR'S CALL") : TEXT("DIRECTOR'S CALL  (used this take)")));
	}
	if (PauseStatus) PauseStatus->SetText(FText::FromString(bAvailable ? TEXT("Your call is ready: change one thing about this cut.") : LastAnnouncement));
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
		R.Label->SetColorAndOpacity(FSlateColor(bHot ? Yellow() : Chalk()));
		R.Slider->SetSliderHandleColor(bHot ? Yellow() : Chalk());
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

void UJamDirectorsCallHudWidget::BuildTree()
{
	if (!WidgetTree) WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	if (WidgetTree->RootWidget) return;
	UWidgetTree* T = WidgetTree;
	Root = T->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("OverlayRoot"));
	T->RootWidget = Root;
	auto Place = [&](UWidget* W, FVector2D Anchor, FVector2D Align, FVector2D Pos)
	{
		UCanvasPanelSlot* S = Root->AddChildToCanvas(W);
		S->SetAnchors(FAnchors(Anchor.X, Anchor.Y, Anchor.X, Anchor.Y));
		S->SetAlignment(Align);
		S->SetPosition(Pos);
		S->SetAutoSize(true);
	};
	// The period card, where lane A5 draws its win/lose card (never at the same time: this one shows only in play).
	// With the generated clapperboard (R-0008) the number is chalked into its box; without it the text says "TAKE n".
	TakeArt = nullptr;
	if (UTexture2D* Slate = Art(TEXT("T_UI_TakeCard")))
	{
		TakeArt = Picture(T, Slate, 640.f, 360.f);
		TakeArt->SetVisibility(ESlateVisibility::Collapsed);
		Place(TakeArt, FVector2D(0.5, 0.4), FVector2D(0.5, 0.5), FVector2D(0, 0));
	}
	TakeCard = Text(T, TEXT(""), TakeArt ? 96 : 84, Chalk());
	TakeCard->SetShadowOffset(FVector2D(3, 3));
	TakeCard->SetVisibility(ESlateVisibility::Collapsed);
	// The chalk box of the slate art sits at 66% of its height: 0.16 * 360 px below the centre of a 640 px wide card.
	Place(TakeCard, FVector2D(0.5, 0.4), FVector2D(0.5, 0.5), FVector2D(0, TakeArt ? 58 : 0));
	TakeSub = Text(T, TEXT("ACTION"), 24, Yellow());
	TakeSub->SetVisibility(ESlateVisibility::Collapsed);
	Place(TakeSub, FVector2D(0.5, 0.4), FVector2D(0.5, 0), FVector2D(0, TakeArt ? 190 : 56));
	// The scoreboard line, under lane A5's score and take row.
	Announcement = Text(T, TEXT(""), 26, Yellow());
	Announcement->SetVisibility(ESlateVisibility::Collapsed);
	// Below lane A5's take row (y 82) and lane A10's bark line (about y 165 in Slate units at 1080p).
	Place(Announcement, FVector2D(0.5, 0), FVector2D(0.5, 0), FVector2D(0, 200));
	ReadyHint = Text(T, TEXT("ESC / START   DIRECTOR'S CALL READY"), 16, Yellow());
	ReadyHint->SetVisibility(ESlateVisibility::Collapsed);
	Place(ReadyHint, FVector2D(0, 1), FVector2D(0, 1), FVector2D(24, -24));
}

void UJamDirectorsCallHudWidget::ShowTake(int32 Take, int64 UntilFrame)
{
	if (!TakeCard) return;
	TakeCard->SetText(FText::FromString(TakeArt ? FString::FromInt(FMath::Max(1, Take)) : FString::Printf(TEXT("TAKE %d"), FMath::Max(1, Take))));
	TakeUntil = UntilFrame;
	TakeCard->SetVisibility(ESlateVisibility::HitTestInvisible);
	TakeSub->SetVisibility(ESlateVisibility::HitTestInvisible);
	if (TakeArt) TakeArt->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UJamDirectorsCallHudWidget::ShowAnnouncement(const FString& Line, int64 UntilFrame)
{
	if (!Announcement) return;
	Announcement->SetText(FText::FromString(Line));
	AnnouncementUntil = UntilFrame;
	Announcement->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UJamDirectorsCallHudWidget::TickFrame(const FJamPresentationFrame& F, bool bCallReady)
{
	if (!TakeCard) return;
	// A restart resets the simulation frame: cards timed against the old count end now.
	if (F.SimFrame < LastFrame) { TakeUntil = -1; AnnouncementUntil = -1; }
	LastFrame = F.SimFrame;
	const bool bEnd = F.Phase == EJamPhase::Win || F.Phase == EJamPhase::Lose;
	const bool bTake = !bEnd && TakeUntil >= 0 && F.SimFrame < TakeUntil;
	TakeCard->SetVisibility(bTake ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	TakeSub->SetVisibility(bTake ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if (TakeArt) TakeArt->SetVisibility(bTake ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	const bool bLine = AnnouncementUntil >= 0 && F.SimFrame < AnnouncementUntil;
	Announcement->SetVisibility(bLine ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	ReadyHint->SetVisibility(bCallReady && !bEnd && !bLine ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

FString UJamDirectorsCallHudWidget::GetTakeCardText() const
{
	if (!TakeCard) return FString();
	// With the slate art the text block holds only the number; the card still reads "TAKE n" to callers.
	return TakeArt ? FString::Printf(TEXT("TAKE %s"), *TakeCard->GetText().ToString()) : TakeCard->GetText().ToString();
}
FString UJamDirectorsCallHudWidget::GetAnnouncementText() const { return Announcement ? Announcement->GetText().ToString() : FString(); }
bool UJamDirectorsCallHudWidget::IsTakeCardVisible() const { return TakeCard && TakeCard->GetVisibility() != ESlateVisibility::Collapsed; }
