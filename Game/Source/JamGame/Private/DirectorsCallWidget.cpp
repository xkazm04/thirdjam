#include "DirectorsCallWidget.h"
#include "InputMapping.h"
#include "SettingsWidget.h"
#include "Progression.h"
#include "DirectorsCallController.h"
#include "MenuStyle.h"
#include "MenuFlow.h"
#include "HUDWidget.h"
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
#include "Components/HorizontalBox.h"
#include "Components/Border.h"
#include "GameFramework/PlayerController.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

using namespace JamMenuStyle;
using S = FJamUiStyle;
using L = FJamMenuLook;

// Lane A18 (visual layer), lane A195 (U2 evolution 6): the pause card and the call card are drawn from the front end's style table (FJamMenuLook:
// panel, items, hints); the scoreboard line is a pill under the score; type from FJamUiStyle. Logic is lane A8's and A15's.

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
	return FString::Printf(TEXT("RULE CHANGE: %s"), *Rest);   // lane A195 (evolution 7, the Director's word): no film word on the scoreboard
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

// Lane A195 (evolution 7): the two mouse buttons by possession.
FString UJamDirectorsCallWidget::PauseStickText()
{
	return FString::Printf(TEXT("WITH THE PUCK      %s\nWITHOUT IT      %s"), *UJamHudWidget::ButtonsBadgeLine(true), *UJamHudWidget::ButtonsBadgeLine(false));
}

FString UJamDirectorsCallWidget::PauseControlsText()
{
	return UJamInputMap::PauseCardText();   // lane A183: the words from the live bindings
}

void UJamDirectorsCallWidget::BuildTree()
{
	if (!WidgetTree) WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	if (WidgetTree->RootWidget) return;
	UWidgetTree* T = WidgetTree;

	// Lane A195 (U2 evolution 6): the pause card in the front end's look (FJamMenuLook): the scrim, a panel with the two team colours on its top edge,
	// the title and the period line, the items with the focus bar, the controls from the live bindings, the key hints in the footer's look. No clapper,
	// no film words: CHANGE A RULE is the old Director's Call (what it does is unchanged).
	UOverlay* Root = T->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("PauseRoot"));
	T->RootWidget = Root;
	UBorder* Dimmer = Plate(T, L::Scrim(), FMargin(0));
	S::AddO(Root, Dimmer);

	UVerticalBox* Body = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("PauseBody"));
	UWidget* Card = L::MakePanel(T, Body, PauseCardWidth, FMargin(PauseCardPad, L::Unit * 3.f));
	S::AddO(Root, Card, HAlign_Center, VAlign_Center);

	Switcher = T->ConstructWidget<UWidgetSwitcher>(UWidgetSwitcher::StaticClass(), TEXT("PauseScreens"));
	AddV(Body, Switcher, FMargin(0));
	HintBox = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("PauseHints"));
	AddV(Body, L::MakeTeamRule(T, 1.f), FMargin(0, L::Unit * 2.f, 0, L::Unit * 1.5f));
	AddV(Body, HintBox, FMargin(0));

	auto Item = [&](const TCHAR* Label, UTextBlock*& OutText) -> UButton*
	{
		UWidget* Bar = nullptr;
		UButton* B = L::MakeItem(T, Label, PauseItemSize, OutText, Bar);
		Painter.Add(B, OutText, Bar);
		return B;
	};

	// Screen 0: the pause menu.
	UVerticalBox* Pause = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("PauseScreen"));
	UTextBlock* Paused = L::MakeText(T, TEXT("PAUSED"), L::Font(L::Display - 8, FName("Black"), 120), L::Text());
	AddV(Pause, Paused, FMargin(0));
	TakeText = L::MakeText(T, TEXT("PERIOD 1"), L::Font(L::Caption, FName("Bold"), L::TrackLabel), L::TextDim());
	AddV(Pause, TakeText, FMargin(0, 0, 0, L::Unit * 2.f));
	UTextBlock* Txt = nullptr;
	ResumeButton = Item(TEXT("RESUME"), Txt);
	ResumeButton->OnClicked.AddDynamic(this, &UJamDirectorsCallWidget::OnResumeClicked);
	AddV(Pause, ResumeButton, FMargin(0, 0, 0, L::Gap));
	UTextBlock* CallTxt = nullptr;
	CallButton = Item(TEXT("CHANGE A RULE"), CallTxt);
	CallButtonText = CallTxt;
	CallButton->OnClicked.AddDynamic(this, &UJamDirectorsCallWidget::OnCallClicked);
	AddV(Pause, CallButton, FMargin(0, 0, 0, L::Gap));
	RestartButton = Item(TEXT("RESTART"), Txt);
	RestartButton->OnClicked.AddDynamic(this, &UJamDirectorsCallWidget::OnRestartClicked);
	AddV(Pause, RestartButton, FMargin(0, 0, 0, L::Gap));
	ControlsButton = Item(TEXT("CONTROLS"), Txt);   // lane A191: rebind mid-match (the CONTROLS screen, SettingsWidget.cpp)
	ControlsButton->OnClicked.AddDynamic(this, &UJamDirectorsCallWidget::OnControlsClicked);
	AddV(Pause, ControlsButton, FMargin(0, 0, 0, L::Gap));
	QuitButton = Item(TEXT("QUIT TO MENU"), Txt);
	QuitButton->OnClicked.AddDynamic(this, &UJamDirectorsCallWidget::OnQuitClicked);
	AddV(Pause, QuitButton, FMargin(0));
	PauseStatus = L::MakeText(T, TEXT(""), L::Font(L::Body, FName("Regular")), L::TextDim());
	PauseStatus->SetWrapTextAt(PauseCardInnerWidth);
	AddV(Pause, PauseStatus, FMargin(0, L::Unit * 1.5f, 0, 0));
	// Lane A195 (evolution 7): the two lines that matter, the two mouse buttons by possession, from the live bindings (the five-line table of lanes
	// A49 / A183 is CONTROLS' job now: PauseControlsText() keeps it for its tests and the HUD).
	PauseStick = L::MakeText(T, PauseStickText(), L::Font(L::Body, FName("Bold"), L::TrackLabel), L::Text());
	PauseStick->SetWrapTextAt(PauseCardInnerWidth);
	PauseStick->SetLineHeightPercentage(1.25f);
	AddV(Pause, PauseStick, FMargin(0, L::Unit * 1.5f, 0, 0));
	UTextBlock* Every = L::MakeText(T, TEXT("EVERY KEY, AND TO REBIND IT: CONTROLS"), L::Font(L::Caption, FName("Bold"), L::TrackLabel), L::TextDim());
	AddV(Pause, Every, FMargin(0, 4, 0, 0));
	Switcher->AddChild(Pause);

	// Screen 1: change a rule (the Director's Call of lane A8: one change per period).
	UVerticalBox* Call = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CallScreen"));
	CallTitle = L::MakeText(T, TEXT("CHANGE A RULE"), L::Font(L::Title, FName("Black"), 120), L::Text());
	AddV(Call, CallTitle, FMargin(0));
	UTextBlock* Rule = L::MakeText(T, TEXT("One change per period. Move one slider, press APPLY. The scoreboard tells everyone the new rule."), L::Font(L::Body, FName("Regular")), L::TextDim());
	Rule->SetWrapTextAt(PauseCardInnerWidth);
	AddV(Call, Rule, FMargin(0, L::Unit, 0, L::Unit * 1.5f));
	RowsBox = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CallRows"));
	AddV(Call, RowsBox, FMargin(0));
	CallStatus = L::MakeText(T, TEXT(""), L::Font(L::Body, FName("Bold")), L::Focus());
	CallStatus->SetWrapTextAt(PauseCardInnerWidth);
	AddV(Call, CallStatus, FMargin(0, 6, 0, 4));
	UHorizontalBox* Actions = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	ApplyButton = Item(TEXT("APPLY"), Txt);
	ApplyButton->OnClicked.AddDynamic(this, &UJamDirectorsCallWidget::OnApplyClicked);
	AddH(Actions, ApplyButton, FMargin(0, 0, L::Gap, 0));
	BackButton = Item(TEXT("BACK"), Txt);
	BackButton->OnClicked.AddDynamic(this, &UJamDirectorsCallWidget::OnBackClicked);
	AddH(Actions, BackButton, FMargin(0));
	AddV(Call, Actions, FMargin(0, L::Unit, 0, 0), HAlign_Left);
	Switcher->AddChild(Call);
	// Screen 2 (lane A191): the CONTROLS screen, the same one the front end shows.
	ControlsScreen = UJamSettingsWidget::MakeControlsScreen(this);
	if (ControlsScreen)
	{
		ControlsScreen->OnBack.AddDynamic(this, &UJamDirectorsCallWidget::OnControlsBack);
		Switcher->AddChild(ControlsScreen);
	}
	BuildLeaveConfirm();   // lane A195 (F6)
	Switcher->SetActiveWidgetIndex(0);
}

void UJamDirectorsCallWidget::ShowControls()
{
	if (!Switcher || !ControlsScreen) return;
	bOnCall = false;
	ControlsScreen->Refresh();
	Switcher->SetActiveWidgetIndex(2);
	PendingFocus = ControlsScreen->GetFirstFocusable();
	RefreshHints();
}

bool UJamDirectorsCallWidget::IsOnControls() const
{
	return Switcher && ControlsScreen && Switcher->GetActiveWidgetIndex() == 2;
}

void UJamDirectorsCallWidget::OnControlsClicked() { ShowControls(); }
void UJamDirectorsCallWidget::OnControlsBack() { ShowPause(); PendingFocus = ControlsButton.Get(); }

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
		R.Label = L::MakeText(T, C.Label.ToUpper(), L::Font(L::Body, FName("Bold"), L::TrackLabel), L::Text());
		AddH(Head, R.Label, FMargin(0, 0, 10, 0), true);
		R.Value = L::MakeText(T, ValueText(C, C.Current), L::Font(L::Body + 2, FName("Black")), L::Text());
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
		UTextBlock* Desc = L::MakeText(T, FString::Printf(TEXT("%s  (%s .. %s)"), *C.Description, *FJamJson::NumberToString(C.Min), *FJamJson::NumberToString(C.Max)), L::Font(L::Caption, FName("Regular")), L::TextDim());
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
	// Lane A144 (the level wire): the pause card names the level in force and whose it is.
	FString LevelLine;
	if (Owner.IsValid()) if (AHokejGameMode* GM = Owner->GetGameMode())
	{
		Take = FMath::Max(1, GM->GetTake());
		const FString Level = UJamDirectorsCallHudWidget::LevelLabel(GM->GetLevelInForce());
		if (!Level.IsEmpty()) LevelLine = FString::Printf(TEXT("   -   %s (%s)"), *Level, GM->IsLevelFromSettings() ? TEXT("YOUR LEVEL") : TEXT("THIS MODE'S OWN"));
	}
	if (TakeText) TakeText->SetText(FText::FromString(FString::Printf(TEXT("PERIOD %d%s"), Take, *LevelLine)));
	if (CallTitle) CallTitle->SetText(FText::FromString(FString::Printf(TEXT("CHANGE A RULE  -  PERIOD %d"), Take)));
	if (CallButton)
	{
		CallButton->SetIsEnabled(bAvailable);
		CallButtonText->SetText(FText::FromString(bAvailable ? TEXT("CHANGE A RULE") : TEXT("CHANGE A RULE  (USED THIS PERIOD)")));
	}
	if (PauseStatus) PauseStatus->SetText(FText::FromString(bAvailable ? TEXT("One rule change is ready this period: make them pay.") : LastAnnouncement));
	if (ApplyButton) ApplyButton->SetIsEnabled(bAvailable);
	for (FRow& R : Rows) if (R.Slider) R.Slider->SetIsEnabled(bAvailable);
}

void UJamDirectorsCallWidget::ShowPause()
{
	bOnCall = false;
	RefreshState();
	if (Switcher) Switcher->SetActiveWidgetIndex(0);
	PendingFocus = ResumeButton.Get();
	RefreshHints();
}

void UJamDirectorsCallWidget::ShowCall()
{
	bOnCall = true;
	RebuildRows();
	RefreshState();
	if (CallStatus && Tunables.Num()) CallStatus->SetText(FText::GetEmpty());
	if (Switcher) Switcher->SetActiveWidgetIndex(1);
	PendingFocus = Rows.Num() ? static_cast<UWidget*>(Rows[0].Slider) : static_cast<UWidget*>(BackButton.Get());
	RefreshHints();
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
		// Lane A195: on the pause card the line says what happened in the front end's words (the scoreboard keeps its own line).
		LastAnnouncement = AnnouncementLine(Call.Announcement);   // lane A195: "RULE CHANGE: ...", the scoreboard's own line
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
void UJamDirectorsCallWidget::OnQuitClicked() { ShowLeaveConfirm(); }   // lane A195 (F6): QUIT TO MENU throws the match away: it asks once
void UJamDirectorsCallWidget::OnApplyClicked() { ApplySelected(); }
void UJamDirectorsCallWidget::OnBackClicked() { ShowPause(); }

void UJamDirectorsCallWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	APlayerController* PC = GetOwningPlayer();
	// Lane A195: no dead key. The keyboard focus never rests on the card itself (a click on its cloth puts it there and Enter then pressed nothing):
	// it goes home to RESUME on the pause menu, to the first slider (or BACK) on the call card. CONTROLS keeps its own focus.
	if (!PendingFocus.IsValid() && PC && IsOpen() && HasUserFocus(PC) && !IsOnControls())
	{
		PendingFocus = IsOnLeaveConfirm() ? static_cast<UWidget*>(LeaveNo.Get()) : bOnCall ? (Rows.Num() ? static_cast<UWidget*>(Rows[0].Slider) : static_cast<UWidget*>(BackButton.Get())) : static_cast<UWidget*>(ResumeButton.Get());
	}
	if (PendingFocus.IsValid())
	{
		PendingFocus->SetFocus();
		PendingFocus = nullptr;
	}
	Painter.Paint(PC, true);
	for (FRow& R : Rows)
	{
		if (!R.Slider || !R.Label) continue;
		const bool bHot = R.Slider->HasAnyUserFocus() || R.Slider->IsHovered();
		R.Label->SetColorAndOpacity(FSlateColor(bHot ? L::Focus() : L::Text()));
	}
	RefreshHints();
}

void UJamDirectorsCallWidget::RefreshHints()
{
	if (!HintBox || !WidgetTree) return;
	TArray<TPair<FString, FString>> Hints;
	if (IsOnLeaveConfirm()) Hints = { { TEXT("ENTER / A"), TEXT("SELECT") }, { TEXT("ESC / B"), TEXT("NO, KEEP PLAYING") } };   // lane A195 (F6)
	else if (IsOnControls()) Hints = { { TEXT("ENTER / A"), TEXT("REBIND") }, { TEXT("ARROWS / STICK"), TEXT("MOVE") }, { TEXT("ESC / B"), TEXT("BACK") } };
	else if (bOnCall) Hints = { { TEXT("LEFT / RIGHT"), TEXT("SLIDE") }, { TEXT("ENTER / A"), TEXT("SELECT") }, { TEXT("ESC / B"), TEXT("BACK") } };
	else Hints = { { TEXT("ESC / START"), TEXT("RESUME") }, { TEXT("ENTER / A"), TEXT("SELECT") }, { L::RestartKeyCap(), TEXT("RESTART") } };   // three: four ran past the card (A195 still)
	const FString Key = L::HintsLine(Hints);
	if (Key == HintKey) return;
	HintKey = Key;
	L::FillHints(WidgetTree, HintBox, Hints);
}

TArray<FString> UJamDirectorsCallWidget::GetPauseLabels() const
{
	TArray<FString> Out;
	for (const UButton* B : { ResumeButton.Get(), CallButton.Get(), RestartButton.Get(), ControlsButton.Get(), QuitButton.Get() })
	{
		FString Label;
		if (B) if (const UPanelWidget* Row = Cast<UPanelWidget>(B->GetContent()))
			for (int32 i = 0; i < Row->GetChildrenCount(); ++i) if (const UTextBlock* Tb = Cast<UTextBlock>(Row->GetChildAt(i))) Label = Tb->GetText().ToString();
		Out.Add(Label);
	}
	return Out;
}

FReply UJamDirectorsCallWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey K = InKeyEvent.GetKey();
	if (IsOnLeaveConfirm() && (K == EKeys::Escape || IsBackKey(K))) { AnswerLeave(false); return FReply::Handled(); }   // lane A195 (F6): Esc / B is NO
	if (IsOnLeaveConfirm() && (K == EKeys::Enter || K == EKeys::Gamepad_FaceButton_Bottom)) { PendingFocus = LeaveNo.Get(); return FReply::Handled(); }
	if (IsOnControls() && (K == EKeys::Escape || IsBackKey(K))) { OnControlsBack(); return FReply::Handled(); }   // lane A191: Esc on CONTROLS goes back, not on
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
	// Lane A195: no dead key. Enter or the pad's A that reach the card itself (no item had the focus) press the screen's first item.
	if (!InKeyEvent.IsRepeat() && (K == EKeys::Enter || K == EKeys::Gamepad_FaceButton_Bottom) && !IsOnControls())
	{
		if (!bOnCall) { OnResumeClicked(); return FReply::Handled(); }
		if (BackButton) { PendingFocus = BackButton.Get(); return FReply::Handled(); }
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

// Lane A123 (track U1, evolution 2): the ready pill waits for the first goal, as the HUD's own CALL READY line does.
static TAutoConsoleVariable<int32> CVarJamHudCallPillAfterGoal(TEXT("jam.hud.CallPillAfterGoal"), 1,
	TEXT("The ESC / START  CALL READY pill: 1 only once a goal has been scored (default, the HUD's own rule), 0 from the first faceoff."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamUiCardMode(TEXT("jam.ui.CardMode"), 0,
	TEXT("MATCH card on rinks over 95 m: 0 = 40 pt in the usual place, 1 = 40 pt lifted to y 150 (lane A76)."), ECVF_Default);

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
	UBorder* Banner = S::Plate(T, S::Panel(0.92f), FMargin(36, 8, 36, 12));   // lane A64: tighter (was 44, 18, 44, 22)
	UVerticalBox* BannerBody = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("BannerBody"));
	Banner->SetContent(BannerBody);
	TakeCard = S::IceText(T, TEXT(""), S::Font(MatchCardFontPt, FName("Black"), S::TrackTitle, true), S::Chalk());
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
	ReadyHint = S::IceText(T, TEXT("ESC / START   RULE CHANGE READY"), S::CaptionFont(), S::Yellow());   // lane A195 (evolution 7): the item's own words
	ReadyHint->SetVisibility(ESlateVisibility::Collapsed);
	HintPill->SetContent(ReadyHint);
	Place(HintPill, FVector2D(0, 1), FVector2D(0, 1), FVector2D(S::EdgePx, -S::EdgePx));
}

// Lane A144 (the level wire): "rookie" -> "ROOKIE", "all_star" -> "ALL-STAR" (the settings button's own labels); empty for no level.
FString UJamDirectorsCallHudWidget::LevelLabel(const FString& Level)
{
	EJamMenuDifficulty D;
	return FJamSettings::ParseDifficulty(Level, D) ? FJamSettings::DifficultyLabel(D) : FString();
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
	FString Line = Count > 0 ? FString::Printf(TEXT("OF %d   -   PERIOD %d   -   FACEOFF"), Count, FMath::Max(1, Take)) : FString::Printf(TEXT("PERIOD %d   -   FACEOFF"), FMath::Max(1, Take));
	// Lane A67 (R-0036): a Director's choice names itself on the card ("POND   -   PERIOD 1   -   FACEOFF").
	if (GM && GM->GetCartridge())
	{
		const FString Mode = UJamMenuFlow::ModeDisplayForId(GM->GetCartridge()->Id);
		if (!Mode.IsEmpty()) Line = FString::Printf(TEXT("%s   -   %s"), *Mode.ToUpper(), *Line);
	}
	// Lane A144 (the level wire): the level in force closes the line ("PERIOD 1   -   FACEOFF   -   ROOKIE"), so nobody plays pro believing it is rookie.
	if (GM)
	{
		const FString Level = LevelLabel(GM->GetLevelInForce());
		if (!Level.IsEmpty()) Line = FString::Printf(TEXT("%s   -   %s"), *Line, *Level);
	}
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
	// Lane A76: the card's type and place follow the rink (and jam.ui.CardMode for the proof).
	const IConsoleVariable* P3 = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.hud.Pass3"));
	const int32 Mode = (P3 && P3->GetInt() == 0) ? -2 : CVarJamUiCardMode.GetValueOnGameThread();   // -2: the A74 card (50 pt, y 190)
	if (F.RinkLengthM != CardRinkLen || Mode != CardMode)
	{
		CardRinkLen = F.RinkLengthM; CardMode = Mode;
		TakeCard->SetFont(S::Font(Mode == -2 ? MatchCardFontPt : MatchCardPtFor(F.RinkLengthM), FName("Black"), S::TrackTitle, true));
		if (TakeArt) if (UCanvasPanelSlot* CS = Cast<UCanvasPanelSlot>(TakeArt->Slot)) CS->SetPosition(FVector2D(0, MatchCardCentreYFor(F.RinkLengthM, Mode > 0)));
	}
	const bool bEnd = F.Phase == EJamPhase::Win || F.Phase == EJamPhase::Lose;
	const bool bTake = !bEnd && TakeUntil >= 0 && F.SimFrame < TakeUntil;
	TakeCard->SetVisibility(bTake ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	TakeSub->SetVisibility(bTake ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if (TakeArt) TakeArt->SetVisibility(bTake ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	const bool bLine = AnnouncementUntil >= 0 && F.SimFrame < AnnouncementUntil;
	SetLine(Announcement, bLine);
	SetLine(ReadyHint, ShowReadyPill(bCallReady, F.ScoreHome + F.ScoreAway, bEnd, bLine, CVarJamHudCallPillAfterGoal.GetValueOnGameThread() != 0));
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
bool UJamDirectorsCallHudWidget::IsLineVisible() const { return Announcement && Announcement->GetVisibility() != ESlateVisibility::Collapsed; }

float UJamDirectorsCallWidget::GetHintRowWidth() const { return HintBox ? static_cast<float>(HintBox->GetDesiredSize().X) : 0.f; }

// ---------------------------------------------------------------- lane A195 (F6): QUIT TO MENU asks once

void UJamDirectorsCallWidget::BuildLeaveConfirm()
{
	UWidgetTree* T = WidgetTree;
	UVerticalBox* Leave = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LeaveScreen"));
	AddV(Leave, L::MakeText(T, TEXT("LEAVE THE MATCH?"), L::Font(L::Display - 8, FName("Black"), 120), L::Text()), FMargin(0));
	UTextBlock* Why = L::MakeText(T, TEXT("The score of this match is thrown away. The season keeps what you have won."), L::Font(L::Body, FName("Regular")), L::TextDim());
	Why->SetWrapTextAt(PauseCardInnerWidth);
	AddV(Leave, Why, FMargin(0, L::Unit, 0, L::Unit * 3.f));
	UTextBlock* Txt = nullptr;
	UWidget* Bar = nullptr;
	LeaveNo = L::MakeItem(T, TEXT("NO, KEEP PLAYING"), PauseItemSize, Txt, Bar);
	Painter.Add(LeaveNo, Txt, Bar);
	LeaveNo->OnClicked.AddDynamic(this, &UJamDirectorsCallWidget::OnLeaveNo);
	AddV(Leave, LeaveNo, FMargin(0, 0, 0, L::Gap));
	LeaveYes = L::MakeItem(T, TEXT("YES, LEAVE TO THE MENU"), PauseItemSize, Txt, Bar);
	Painter.Add(LeaveYes, Txt, Bar);
	LeaveYes->OnClicked.AddDynamic(this, &UJamDirectorsCallWidget::OnLeaveYes);
	AddV(Leave, LeaveYes, FMargin(0));
	LeaveIndex = Switcher->GetChildrenCount();
	Switcher->AddChild(Leave);
}

bool UJamDirectorsCallWidget::IsOnLeaveConfirm() const
{
	return Switcher && LeaveIndex >= 0 && Switcher->GetActiveWidgetIndex() == LeaveIndex;
}

void UJamDirectorsCallWidget::ShowLeaveConfirm()
{
	if (!Switcher || LeaveIndex < 0) return;
	bOnCall = false;
	Switcher->SetActiveWidgetIndex(LeaveIndex);
	PendingFocus = LeaveNo.Get();   // NO by default: a stray second press keeps the match
	RefreshHints();
	UE_LOG(LogTemp, Log, TEXT("JAMPAUSE confirm leave open (focus=no)"));
}

void UJamDirectorsCallWidget::AnswerLeave(bool bYes)
{
	UE_LOG(LogTemp, Log, TEXT("JAMPAUSE confirm leave answer=%s"), bYes ? TEXT("yes") : TEXT("no"));
	if (bYes) { if (Owner.IsValid()) Owner->QuitToMenu(); return; }
	ShowPause();
	PendingFocus = QuitButton.Get();
}

void UJamDirectorsCallWidget::OnLeaveNo() { AnswerLeave(false); }
void UJamDirectorsCallWidget::OnLeaveYes() { AnswerLeave(true); }
