#include "SettingsWidget.h"
#include "InputMapping.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Blueprint/WidgetTree.h"
#include "GameFramework/PlayerController.h"
#include "MenuStyle.h"
#include "MenuWidget.h"
#include "LookTierSwitcher.h"
#include "SignalSystem.h"
#include "JamDesignerSubsystem.h"
#include "HAL/IConsoleManager.h"
#include "Components/Slider.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/SizeBox.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/App.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

using namespace JamMenuStyle;
using S = FJamUiStyle;

DEFINE_LOG_CATEGORY_STATIC(LogJamSettings, Log, All);

// Lane A18 (visual layer): two columns (key, difficulty, look | volume, voice, barks) with kit icons, the controls under both;
// type and plates from FJamUiStyle. The settings themselves (load, save, apply) are lane A8's.

const TCHAR* FJamSettings::Section = TEXT("Jam.Settings");
const TCHAR* FJamSettings::ControlsSection = TEXT("Jam.Controls");   // lane A183

namespace
{
	TOptional<FJamSettings> GJamSettingsForTest;
}

void FJamSettings::SetOverrideForTest(const FJamSettings* Settings)
{
	if (Settings) GJamSettingsForTest = *Settings; else GJamSettingsForTest.Reset();
}

namespace
{
	const TCHAR* const SavedKeys[] = { TEXT("Difficulty"), TEXT("DifficultyChosen"), TEXT("LookTier"), TEXT("Volume"), TEXT("Barks"), TEXT("BarkIntensity") };
}

FJamSettings FJamSettings::FromSaved(const TMap<FString, FString>& Saved)
{
	FJamSettings St;
	bool bHasLevel = false, bChosenKey = false;
	if (const FString* D = Saved.Find(TEXT("Difficulty"))) bHasLevel = ParseDifficulty(*D, St.Difficulty);
	if (const FString* C = Saved.Find(TEXT("DifficultyChosen"))) bChosenKey = C->ToBool();
	St.bDifficultyChosen = IsDifficultyChosen(bChosenKey, bHasLevel, St.Difficulty);
	if (const FString* L = Saved.Find(TEXT("LookTier"))) St.LookTierOverride = *L;
	if (!St.LookTierOverride.IsEmpty() && !ULookTierSwitcher::GetKnownTiers().Contains(St.LookTierOverride)) St.LookTierOverride.Empty();
	if (const FString* V = Saved.Find(TEXT("Volume"))) St.Volume = FMath::Clamp(FCString::Atof(**V), 0.f, 1.f);
	if (const FString* B = Saved.Find(TEXT("Barks"))) St.Barks = FMath::Clamp(FCString::Atoi(**B), -1, 1);
	if (const FString* I = Saved.Find(TEXT("BarkIntensity"))) St.BarkIntensity = *I;
	if (St.BarkIntensity != TEXT("quiet") && St.BarkIntensity != TEXT("normal") && St.BarkIntensity != TEXT("loud")) St.BarkIntensity.Empty();
	// Lane A183: the bindings, "Controls.<verb>" -> "Keys|Pads".
	for (const TPair<FString, FString>& P : Saved) if (P.Key.StartsWith(TEXT("Controls."))) St.Controls.Add(P.Key.Mid(9), P.Value);
	return St;
}

TMap<FString, FString> FJamSettings::ToSaved() const
{
	TMap<FString, FString> Out;
	Out.Add(TEXT("Difficulty"), DifficultyString(Difficulty));
	Out.Add(TEXT("DifficultyChosen"), bDifficultyChosen ? TEXT("True") : TEXT("False"));
	Out.Add(TEXT("LookTier"), LookTierOverride);
	Out.Add(TEXT("Volume"), FString::SanitizeFloat(Volume));
	Out.Add(TEXT("Barks"), FString::FromInt(Barks));
	Out.Add(TEXT("BarkIntensity"), BarkIntensity);
	for (const TPair<FString, FString>& P : Controls) Out.Add(TEXT("Controls.") + P.Key, P.Value);   // lane A183
	return Out;
}

FJamSettings FJamSettings::Load()
{
	if (GJamSettingsForTest.IsSet()) return GJamSettingsForTest.GetValue();
	if (!GConfig) return FJamSettings();
	TMap<FString, FString> Saved;
	for (const TCHAR* Key : SavedKeys)
	{
		FString Value;
		if (GConfig->GetString(Section, Key, Value, GGameUserSettingsIni)) Saved.Add(Key, Value);
	}
	// Lane A183: the bindings' own section, one key per verb.
	for (int32 I = 0; I < FJamBindings::NumVerbs; ++I)
	{
		FString Value;
		const TCHAR* Id = FJamBindings::VerbId(static_cast<EJamVerb>(I));
		if (GConfig->GetString(ControlsSection, Id, Value, GGameUserSettingsIni)) Saved.Add(FString(TEXT("Controls.")) + Id, Value);
	}
	return FromSaved(Saved);
}

void FJamSettings::Save() const
{
	// A test's profile stays in memory: nothing of a test reaches the player's file.
	if (GJamSettingsForTest.IsSet()) { GJamSettingsForTest = *this; return; }
	if (!GConfig) return;
	// Every key in one save and one flush: the level and its pick key never travel apart.
	for (const TPair<FString, FString>& P : ToSaved())
	{
		// Lane A183: the bindings go to their own section, in the same save and flush.
		if (P.Key.StartsWith(TEXT("Controls."))) GConfig->SetString(ControlsSection, *P.Key.Mid(9), *P.Value, GGameUserSettingsIni);
		else GConfig->SetString(Section, *P.Key, *P.Value, GGameUserSettingsIni);
	}
	GConfig->Flush(false, GGameUserSettingsIni);
	// Lane A164 (evolution 4): say whether this save reaches the disk. A process started with -Multiprocess (the gate, the off-screen stills, an
	// uncooked `-game -Multiprocess` window) keeps it in memory only (the engine's ConfigContext / AreWritesAllowedGlobally), so a restart of such a
	// process reads the file as it was: that is the flag, not the game (A155's editor-mode "pick lost").
	UE_LOG(LogJamSettings, Log, TEXT("JAMSETTINGS saved difficulty=%s chosen=%d to_disk=%d file=%s"), *DifficultyString(Difficulty), bDifficultyChosen ? 1 : 0,
		WritesReachDisk() ? 1 : 0, *GGameUserSettingsIni);
}

bool FJamSettings::WritesReachDisk()
{
	const TCHAR* Cmd = FCommandLine::Get();
	if (FParse::Param(Cmd, TEXT("nowrite"))) return false;
	return !FParse::Param(Cmd, TEXT("Multiprocess")) || FParse::Param(Cmd, TEXT("MultiprocessSaveConfig"));
}

void FJamSettings::Apply() const
{
	FApp::SetVolumeMultiplier(FMath::Clamp(Volume, 0.f, 1.f));
	// Barks on/off through lane A12's console variable; the intensity override only when a variable for it exists.
	if (IConsoleVariable* V = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.Barks"))) V->Set(Barks, ECVF_SetByCode);
	if (IConsoleVariable* I = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.BarkIntensity"))) I->Set(*BarkIntensity, ECVF_SetByCode);
}

FString FJamSettings::BarksLabel(int32 Barks, const FString& Intensity)
{
	if (Barks == 0) return TEXT("OFF");
	const FString Base = Barks < 0 ? TEXT("MODE'S OWN") : TEXT("ON");
	return Intensity.IsEmpty() ? Base : Base + TEXT(", ") + Intensity.ToUpper();
}

FString FJamSettings::DifficultyString(EJamMenuDifficulty D)
{
	switch (D)
	{
	case EJamMenuDifficulty::Rookie: return TEXT("rookie");
	case EJamMenuDifficulty::AllStar: return TEXT("all_star");
	default: return TEXT("pro");
	}
}

bool FJamSettings::ParseDifficulty(const FString& Str, EJamMenuDifficulty& Out)
{
	if (Str == TEXT("rookie")) { Out = EJamMenuDifficulty::Rookie; return true; }
	if (Str == TEXT("pro")) { Out = EJamMenuDifficulty::Pro; return true; }
	if (Str == TEXT("all_star")) { Out = EJamMenuDifficulty::AllStar; return true; }
	return false;
}

FString FJamSettings::DifficultyLabel(EJamMenuDifficulty D)
{
	switch (D)
	{
	case EJamMenuDifficulty::Rookie: return TEXT("ROOKIE");
	case EJamMenuDifficulty::AllStar: return TEXT("ALL-STAR");
	default: return TEXT("PRO");
	}
}

FString FJamSettings::TierLabel(const FString& Tier)
{
	if (Tier.IsEmpty()) return TEXT("EACH RINK'S OWN");
	if (Tier == TEXT("l1_day")) return TEXT("DAY (L1)");
	if (Tier == TEXT("l2_night")) return TEXT("NIGHT (L2)");
	if (Tier == TEXT("l3_neon")) return TEXT("NEON (L3)");
	if (Tier == TEXT("p3_clay")) return TEXT("CLAY (P3)");   // B-to-A-0018 (lane A21)
	if (Tier == TEXT("p4_broadcast")) return TEXT("BROADCAST (P4)");   // B-to-A-0025 (lane A31)
	return Tier.ToUpper();
}

TSharedRef<SWidget> UJamSettingsWidget::RebuildWidget()
{
	BuildTree();
	return Super::RebuildWidget();
}

void UJamSettingsWidget::BuildTree()
{
	if (!WidgetTree) WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	if (WidgetTree->RootWidget) return;
	UWidgetTree* T = WidgetTree;
	Settings = FJamSettings::Load();
	using L = FJamMenuLook;
	Painter = MakeShared<FJamMenuPainter>();

	// The panel sits on the 880 card inside 32 units of padding a side: two columns of 394 with 28 between them.
	const float ColumnTextW = 386.f;

	Root = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("SettingsRoot"));
	T->RootWidget = Root;

	// Lane A195 (U2 evolution 6): the front end's look (FJamMenuLook): a heading in white caps with its caption in the dim face (the kit icons and the
	// yellow of the slate kit are gone), the buttons in the item look. The words name modes and rinks, not cuts. What each setting does is unchanged.
	auto Heading = [&](UVerticalBox* Col, const FString& Title, const FString& Caption)
	{
		AddV(Col, L::MakeText(T, Title, L::Font(L::Body, FName("Black"), L::TrackLabel), L::Text()), FMargin(0, L::Unit * 2.f, 0, 2));
		UTextBlock* C = L::MakeText(T, Caption, L::Font(L::Caption + 1, FName("Regular")), L::TextDim());
		C->SetWrapTextAt(ColumnTextW);   // lane A144: a fixed wrap width, so the screen's measure does not wait for a layout pass
		AddV(Col, C, FMargin(0, 0, 0, L::Unit));
	};
	auto Item = [&](const TCHAR* Label, UTextBlock*& OutText) -> UButton*
	{
		UWidget* Bar = nullptr;
		UButton* B = L::MakeItem(T, Label, L::ItemSmall, OutText, Bar);
		Painter->Add(B, OutText, Bar);
		return B;
	};

	UHorizontalBox* Columns = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	UVerticalBox* Left = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("SettingsLeft"));
	UVerticalBox* Right = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("SettingsRight"));
	AddH(Columns, Left, FMargin(0, 0, 14, 0), true, VAlign_Top);
	AddH(Columns, Right, FMargin(14, 0, 0, 0), true, VAlign_Top);
	AddV(Root, Columns, FMargin(0));
	UTextBlock* Tmp = nullptr;

	// Left column: the AI difficulty, the look, the API key.
	Heading(Left, TEXT("AI LEVEL"), TEXT("For PLAY NOW. The season and the modes set their own."));
	DifficultyButton = Item(TEXT(""), Tmp);
	DifficultyText = Tmp;
	DifficultyButton->OnClicked.AddDynamic(this, &UJamSettingsWidget::OnDifficultyClicked);
	AddV(Left, DifficultyButton, FMargin(0), HAlign_Left);

	Heading(Left, TEXT("LOOK"), TEXT("Each rink's own look, or one look for every rink."));
	TierButton = Item(TEXT(""), Tmp);
	TierText = Tmp;
	TierButton->OnClicked.AddDynamic(this, &UJamSettingsWidget::OnTierClicked);
	AddV(Left, TierButton, FMargin(0), HAlign_Left);

	Heading(Left, TEXT("API KEY"), TEXT("Typed here, kept on this machine only: MAKE YOUR OWN writes new games live with it."));
	UHorizontalBox* KeyRow = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	KeyBox = T->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("KeyBox"));
	KeyBox->SetIsPassword(true);
	KeyBox->SetHintText(FText::FromString(TEXT("paste key, Enter")));
	KeyBox->SetMinDesiredWidth(220.f);
	KeyBox->WidgetStyle.SetFont(L::Font(L::Body, FName("Regular")));
	KeyBox->WidgetStyle.SetPadding(FMargin(12, 8));
	// Lane A144: light type on dark (the default light box hid its grey hint); lane A195: the panel's fill and edge.
	KeyBox->WidgetStyle.SetBackgroundColor(FLinearColor(0.16f, 0.17f, 0.20f, 1.f));
	KeyBox->WidgetStyle.SetForegroundColor(L::Text());
	KeyBox->WidgetStyle.SetFocusedForegroundColor(L::Text());
	KeyBox->OnTextCommitted.AddDynamic(this, &UJamSettingsWidget::OnKeyCommitted);
	AddH(KeyRow, KeyBox, FMargin(0, 0, L::Gap, 0), true);
	ClearKeyButton = Item(TEXT("CLEAR"), Tmp);
	ClearKeyText = Tmp;
	ClearKeyButton->OnClicked.AddDynamic(this, &UJamSettingsWidget::OnClearKey);
	AddH(KeyRow, ClearKeyButton, FMargin(0));
	AddV(Left, KeyRow, FMargin(0, 0, 0, 4));
	KeyState = L::MakeText(T, TEXT(""), L::Font(L::Caption, FName("Bold"), L::TrackLabel), L::TextDim());
	AddV(Left, KeyState, FMargin(0, 0, 0, 6));

	// Right column: the volume, the voice signals, the barks.
	Heading(Right, TEXT("VOLUME"), TEXT("Everything the game plays."));
	UHorizontalBox* VolRow = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	VolumeSlider = T->ConstructWidget<USlider>(USlider::StaticClass(), TEXT("VolumeSlider"));
	VolumeSlider->SetStepSize(0.05f);
	VolumeSlider->RequiresControllerLock = false;
	VolumeSlider->IsFocusable = true;
	FSliderStyle Sl = S::SliderStyle();
	Sl.SetHoveredThumbImage(FSlateBrush(FSlateRoundedBoxBrush(L::Focus(), 7.f)));
	Sl.SetNormalThumbImage(FSlateBrush(FSlateRoundedBoxBrush(L::Text(), 7.f)));
	VolumeSlider->SetWidgetStyle(Sl);
	VolumeSlider->SetSliderHandleColor(L::Text());
	VolumeSlider->SetSliderBarColor(L::Chosen());
	VolumeSlider->OnValueChanged.AddDynamic(this, &UJamSettingsWidget::OnVolumeChanged);
	USizeBox* VolBox = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	VolBox->SetWidthOverride(260.f);
	VolBox->AddChild(VolumeSlider);
	AddH(VolRow, VolBox, FMargin(0, 0, 12, 0));
	VolumeText = L::MakeText(T, TEXT(""), L::Font(L::Body + 2, FName("Black")), L::Text());
	AddH(VolRow, VolumeText);
	AddV(Right, VolRow, FMargin(0, 4, 0, 6));

	// Voice signals (lane A11's recognizer); keyboard signals 1-4 stay on regardless.
	Heading(Right, TEXT("VOICE SIGNALS"), TEXT("Say a signal instead of pressing 1-4; the keys always work."));
	UHorizontalBox* VoiceRow = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	VoiceButton = Item(TEXT(""), Tmp);
	VoiceText = Tmp;
	VoiceButton->OnClicked.AddDynamic(this, &UJamSettingsWidget::OnVoiceClicked);
	AddH(VoiceRow, VoiceButton, FMargin(0, 0, 12, 0));
	VoiceStatusText = L::MakeText(T, TEXT(""), L::Font(L::Caption, FName("Regular")), L::TextDim());
	VoiceStatusText->SetWrapTextAt(200.f);
	AddH(VoiceRow, VoiceStatusText, FMargin(0), true);
	AddV(Right, VoiceRow, FMargin(0, 0, 0, 6));

	// Barks (lane A12): off, the mode's own, on; the intensity override rides along.
	Heading(Right, TEXT("BARKS"), TEXT("What the other skaters shout: off, the mode's own, or on (quiet, normal, loud)."));
	BarksButton = Item(TEXT(""), Tmp);
	BarksText = Tmp;
	BarksButton->OnClicked.AddDynamic(this, &UJamSettingsWidget::OnBarksClicked);
	AddV(Right, BarksButton, FMargin(0), HAlign_Left);

	// Lane A144: the controls have a screen of their own; lane A195: say where, in the words of the screens.
	Heading(Right, TEXT("CONTROLS"), TEXT("Rebind any key: CONTROLS on HOME, or Esc in a match, then CONTROLS."));

	RefreshLabels();
}

void UJamSettingsWidget::Refresh()
{
	Settings = FJamSettings::Load();
	RefreshLabels();
}

void UJamSettingsWidget::RefreshLabels()
{
	if (!DifficultyText) return;
	DifficultyText->SetText(FText::FromString(FJamSettings::DifficultyLabel(Settings.EffectiveDifficulty())));
	TierText->SetText(FText::FromString(FJamSettings::TierLabel(Settings.LookTierOverride)));
	VolumeSlider->SetValue(Settings.Volume);
	VolumeText->SetText(FText::FromString(FString::Printf(TEXT("%d %%"), FMath::RoundToInt(Settings.Volume * 100.f))));
	bool bHasKey = false;
	if (UGameInstance* GI = GetGameInstance()) if (UJamDesignerSubsystem* D = GI->GetSubsystem<UJamDesignerSubsystem>()) bHasKey = D->HasApiKey();
	KeyState->SetText(FText::FromString(bHasKey ? TEXT("KEY: SET") : TEXT("KEY: NONE (SAVED GAMES ONLY)")));
	KeyState->SetColorAndOpacity(FSlateColor(bHasKey ? FJamMenuLook::Chosen() : FJamMenuLook::TextDim()));
	ClearKeyButton->SetIsEnabled(bHasKey);
	if (VoiceText) VoiceText->SetText(FText::FromString(UJamSignalSystem::IsVoiceSignalsEnabled() ? TEXT("VOICE: ON") : TEXT("VOICE: OFF")));
	if (VoiceStatusText) VoiceStatusText->SetText(FText::FromString(VoiceStatus()));
	if (BarksText) BarksText->SetText(FText::FromString(FJamSettings::BarksLabel(Settings.Barks, Settings.BarkIntensity)));
}

FString UJamSettingsWidget::VoiceStatus() const
{
	UWorld* W = GetWorld();
	UJamSignalSystem* Sig = W ? W->GetSubsystem<UJamSignalSystem>() : nullptr;
	return Sig ? Sig->GetVoiceStatus() : FString(TEXT("no signal system in this world"));
}

void UJamSettingsWidget::OnVoiceClicked()
{
	UJamSignalSystem::SetVoiceSignalsEnabled(!UJamSignalSystem::IsVoiceSignalsEnabled());
	RefreshLabels();
}

void UJamSettingsWidget::OnBarksClicked()
{
	Settings = FJamSettings::Load();   // lane A164: a press changes its own field on the profile as saved, never on a copy older than the switch's pick
	// Cycle: cut's own -> on, quiet -> on, normal -> on, loud -> off -> cut's own.
	if (Settings.Barks < 0) { Settings.Barks = 1; Settings.BarkIntensity = TEXT("quiet"); }
	else if (Settings.Barks == 1 && Settings.BarkIntensity == TEXT("quiet")) Settings.BarkIntensity = TEXT("normal");
	else if (Settings.Barks == 1 && Settings.BarkIntensity == TEXT("normal")) Settings.BarkIntensity = TEXT("loud");
	else if (Settings.Barks == 1) { Settings.Barks = 0; Settings.BarkIntensity.Empty(); }
	else { Settings.Barks = -1; Settings.BarkIntensity.Empty(); }
	Settings.Apply();
	Settings.Save();
	RefreshLabels();
}

UWidget* UJamSettingsWidget::GetFirstFocusable() const
{
	return DifficultyButton;
}

TArray<UWidget*> UJamSettingsWidget::GetFocusables() const
{
	TArray<UWidget*> Out;
	Out.Add(DifficultyButton.Get());   // lane A195: in reading order (the level and the look head the left column now)
	Out.Add(TierButton.Get());
	Out.Add(KeyBox.Get());
	Out.Add(ClearKeyButton.Get());
	Out.Add(VolumeSlider.Get());
	Out.Add(VoiceButton.Get());
	Out.Add(BarksButton.Get());
	return Out;
}

void UJamSettingsWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (Painter) Painter->Paint(GetOwningPlayer(), true);   // lane A195: the item look (the orange focus fill and bar); the hovered item takes the focus
	// The recognizer's state changes on its own (microphone, start-up): re-read it twice a second.
	if (VoiceStatusText && ++VoiceStatusTick % 30 == 0) VoiceStatusText->SetText(FText::FromString(VoiceStatus()));
}

void UJamSettingsWidget::OnKeyCommitted(const FText& InText, ETextCommit::Type Commit)
{
	if (Commit != ETextCommit::OnEnter) return;
	const FString Key = InText.ToString().TrimStartAndEnd();
	if (Key.IsEmpty()) return;
	if (UGameInstance* GI = GetGameInstance()) if (UJamDesignerSubsystem* D = GI->GetSubsystem<UJamDesignerSubsystem>()) D->SetApiKey(Key);
	KeyBox->SetText(FText::GetEmpty());   // never shown back
	RefreshLabels();
}

void UJamSettingsWidget::OnClearKey()
{
	if (UGameInstance* GI = GetGameInstance()) if (UJamDesignerSubsystem* D = GI->GetSubsystem<UJamDesignerSubsystem>()) D->SetApiKey(FString());
	KeyBox->SetText(FText::GetEmpty());
	RefreshLabels();
}

void UJamSettingsWidget::OnDifficultyClicked()
{
	// Lane A144: the button steps on from what it shows (rookie on a first run), and a press is the player's pick from now on.
	Settings = FJamSettings::Load();   // lane A164: from the level in force now (the first screen's switch may have moved it since this panel loaded)
	Settings.Difficulty = static_cast<EJamMenuDifficulty>((static_cast<uint8>(Settings.EffectiveDifficulty()) + 1) % 3);
	Settings.bDifficultyChosen = true;
	Settings.Save();
	RefreshLabels();
}

void UJamSettingsWidget::OnTierClicked()
{
	Settings = FJamSettings::Load();   // lane A164: the profile as saved, so this save cannot put back an older level or pick key
	TArray<FString> Options;
	Options.Add(FString());
	Options.Append(ULookTierSwitcher::GetKnownTiers());
	const int32 I = Options.IndexOfByKey(Settings.LookTierOverride);
	Settings.LookTierOverride = Options[(I + 1) % Options.Num()];
	Settings.Save();
	RefreshLabels();
}

void UJamSettingsWidget::OnVolumeChanged(float V)
{
	// Lane A164: the slider also fires when RefreshLabels sets it (USlider::SetValue broadcasts a change): the volume goes onto the profile as saved.
	Settings = FJamSettings::Load();
	Settings.Volume = FMath::Clamp(V, 0.f, 1.f);
	Settings.Apply();
	Settings.Save();
	VolumeText->SetText(FText::FromString(FString::Printf(TEXT("%d %%"), FMath::RoundToInt(Settings.Volume * 100.f))));
}

// ---------------------------------------------------------------- lane A183: the CONTROLS screen

UJamControlsWidget* UJamSettingsWidget::MakeControlsScreen(UObject* Outer)
{
	if (UWidget* W = Cast<UWidget>(Outer)) return CreateWidget<UJamControlsWidget>(W, UJamControlsWidget::StaticClass());
	if (APlayerController* PC = Cast<APlayerController>(Outer)) return CreateWidget<UJamControlsWidget>(PC, UJamControlsWidget::StaticClass());
	if (UWorld* World = Cast<UWorld>(Outer)) return CreateWidget<UJamControlsWidget>(World, UJamControlsWidget::StaticClass());
	if (UGameInstance* GI = Cast<UGameInstance>(Outer)) return CreateWidget<UJamControlsWidget>(GI, UJamControlsWidget::StaticClass());
	if (Outer && Outer->GetWorld()) return CreateWidget<UJamControlsWidget>(Outer->GetWorld(), UJamControlsWidget::StaticClass());
	return nullptr;
}

namespace
{
	// The rows in the order a player reads them (the two-role buttons first, as two rows each).
	const EJamVerb GControlRowsOrder[] = { EJamVerb::Shoot, EJamVerb::CrossCheck, EJamVerb::Pass, EJamVerb::Check, EJamVerb::Poke, EJamVerb::Deke, EJamVerb::Spin,
		EJamVerb::Ability, EJamVerb::Sprint, EJamVerb::Brake, EJamVerb::SkateForward, EJamVerb::SkateBack, EJamVerb::SkateLeft, EJamVerb::SkateRight, EJamVerb::Restart };

	FString RowLabel(EJamVerb V)
	{
		switch (V)
		{
		case EJamVerb::Shoot: return TEXT("SHOOT (WITH THE PUCK)");
		case EJamVerb::CrossCheck: return TEXT("CROSS-CHECK (WITHOUT)");
		case EJamVerb::Pass: return TEXT("PASS (WITH THE PUCK)");
		case EJamVerb::Check: return TEXT("CHECK (WITHOUT)");
		default: return FJamBindings::VerbLabel(V);
		}
	}

	bool IsSkate(EJamVerb V) { return V >= EJamVerb::SkateForward && V <= EJamVerb::SkateRight; }
}

TSharedRef<SWidget> UJamControlsWidget::RebuildWidget()
{
	BuildTree();
	return Super::RebuildWidget();
}

void UJamControlsWidget::BuildTree()
{
	if (!WidgetTree) WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	if (WidgetTree->RootWidget) return;
	UWidgetTree* T = WidgetTree;
	SetIsFocusable(true);
	using L = FJamMenuLook;
	Painter = MakeShared<FJamMenuPainter>();

	// Lane A195 (U2 evolution 6): the screen in the front end's look (FJamMenuLook): white type, verbs in the dim caption face, the cells and the two
	// buttons in the item look (the orange focus fill, the white bar), the listening veil in the style table's scrim. What a cell does is A183's.
	UOverlay* Root = T->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("ControlsRoot"));
	T->RootWidget = Root;
	UVerticalBox* Box = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ControlsRows"));
	S::AddO(Root, Box);
	AddV(Box, L::MakeText(T, TEXT("REBIND ANY KEY"), L::Font(L::Title - 8, FName("Black"), 120), L::Text()), FMargin(0));
	UTextBlock* How = L::MakeText(T, TEXT("Click a key or a pad button, then press the new one. Esc cancels. Saved at once."), L::Font(L::Body - 2, FName("Regular")), L::TextDim());
	How->SetWrapTextAt(744.f);   // lane A195: the pause card's inner width (the menu's panel is wider)
	AddV(Box, How, FMargin(0, 2, 0, L::Unit));
	const float VerbW = 250.f, KeysW = 270.f, PadW = 180.f;   // lane A191: 724 with the gaps: inside the pause card's 744 as well as the menu card
	auto Fixed = [&](UHorizontalBox* Row, UWidget* W, float Width)
	{
		USizeBox* Size = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		Size->SetWidthOverride(Width);
		Size->AddChild(W);
		AddH(Row, Size, FMargin(0, 0, 8, 0), false, VAlign_Center);
	};
	const FSlateFontInfo HeadFont = L::Font(L::Caption, FName("Bold"), L::TrackLabel);
	{
		UHorizontalBox* Head = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		Fixed(Head, L::MakeText(T, TEXT(""), HeadFont, L::TextDim()), VerbW);
		Fixed(Head, L::MakeText(T, TEXT("MOUSE AND KEYS"), HeadFont, L::TextDim()), KeysW);
		Fixed(Head, L::MakeText(T, TEXT("PAD"), HeadFont, L::TextDim()), PadW);
		AddV(Box, Head, FMargin(0, 0, 0, 2));
	}
	CellButtons.Init(nullptr, FJamBindings::NumVerbs * 2);
	CellTexts.Init(nullptr, FJamBindings::NumVerbs * 2);
	const FMargin CellPad(12, 3);   // a row a line high: fifteen rows and the buttons inside the menu card and the pause card
	for (const EJamVerb V : GControlRowsOrder)
	{
		UHorizontalBox* Row = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		Fixed(Row, L::MakeText(T, RowLabel(V), L::Font(L::Body - 3, FName("Bold"), 20), L::Text()), VerbW);
		for (int32 Pad = 0; Pad < 2; ++Pad)
		{
			const int32 Cell = static_cast<int32>(V) * 2 + Pad;
			UButton* B = T->ConstructWidget<UButton>(UButton::StaticClass());
			UTextBlock* Txt = L::MakeText(T, TEXT(""), L::Font(L::Caption + 1, FName("Bold"), 20), L::Text());
			Txt->SetJustification(ETextJustify::Center);
			B->AddChild(Txt);
			B->SetIsEnabled(!(Pad == 1 && IsSkate(V)));   // the left stick skates: no pad cell for the four directions
			Painter->Add(B, Txt, nullptr, CellPad);
			CellButtons[Cell] = B;
			CellTexts[Cell] = Txt;
			// UButton's click carries no payload: the tick tells which cell was clicked (OnCellClicked).
			Fixed(Row, B, Pad ? PadW : KeysW);
		}
		AddV(Box, Row, FMargin(0, 1));
	}
	StatusText = L::MakeText(T, TEXT(""), L::Font(L::Body, FName("Bold")), L::Focus());
	StatusText->SetWrapTextAt(700.f);
	AddV(Box, StatusText, FMargin(0, 6, 0, 0));
	UHorizontalBox* Buttons = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	UTextBlock* RT = nullptr;
	UWidget* RBar = nullptr;
	ResetButton = L::MakeItem(T, TEXT("RESET TO DEFAULTS"), L::ItemSmall, RT, RBar);
	ResetText = RT;
	Painter->Add(ResetButton, RT, RBar);
	ResetButton->OnClicked.AddDynamic(this, &UJamControlsWidget::OnResetClicked);
	AddH(Buttons, ResetButton, FMargin(0, 0, L::Gap, 0));
	UTextBlock* BT = nullptr;
	UWidget* BBar = nullptr;
	BackButton = L::MakeItem(T, TEXT("BACK"), L::ItemSmall, BT, BBar);
	BackText = BT;
	Painter->Add(BackButton, BT, BBar);
	BackButton->OnClicked.AddDynamic(this, &UJamControlsWidget::OnBackClicked);
	AddH(Buttons, BackButton, FMargin(0));
	AddV(Box, Buttons, FMargin(0, L::Unit, 0, 0), HAlign_Left);

	// The veil while a cell listens: it covers the rows so the next mouse press reaches this widget, not a button under the cursor.
	ListenVeil = S::Plate(T, L::Scrim(), FMargin(0));
	ListenVeil->SetVisibility(ESlateVisibility::Collapsed);
	UTextBlock* Press = L::MakeText(T, TEXT("PRESS THE NEW KEY  -  ESC CANCELS"), L::Font(L::Item, FName("Black"), 120), L::Text());
	Press->SetJustification(ETextJustify::Center);
	ListenVeil->SetHorizontalAlignment(HAlign_Center);
	ListenVeil->SetVerticalAlignment(VAlign_Center);
	ListenVeil->SetContent(Press);
	S::AddO(Root, ListenVeil);
	RefreshCells();
}

void UJamControlsWidget::Refresh()
{
	if (ListenVeil) ListenVeil->SetVisibility(ESlateVisibility::Collapsed);
	ListenVerb = -1;
	StatusLine.Reset();
	bResetArmed = false;   // lane A199
	if (ResetText) ResetText->SetText(FText::FromString(TEXT("RESET TO DEFAULTS")));
	RefreshCells();
}

FString UJamControlsWidget::CellText(EJamVerb Verb, bool bPad) const
{
	if (bPad && IsSkate(Verb)) return TEXT("LEFT STICK");
	const FString T = UJamInputMap::VerbKeysText(Verb, bPad);
	return T.IsEmpty() ? FString(TEXT("-")) : T;
}

void UJamControlsWidget::RefreshCells()
{
	for (int32 I = 0; I < CellTexts.Num(); ++I)
	{
		if (!CellTexts[I]) continue;
		const EJamVerb V = static_cast<EJamVerb>(I / 2);
		const bool bPad = (I % 2) == 1;
		const bool bHere = ListenVerb == I / 2 && bListenPad == bPad;
		CellTexts[I]->SetText(FText::FromString(bHere ? FString(TEXT("...")) : CellText(V, bPad)));
	}
	if (StatusText) StatusText->SetText(FText::FromString(StatusLine));
}

UWidget* UJamControlsWidget::GetFirstFocusable() const
{
	return CellButtons.IsValidIndex(0) ? CellButtons[0].Get() : nullptr;
}

TArray<UWidget*> UJamControlsWidget::GetFocusables() const
{
	TArray<UWidget*> Out;
	for (const EJamVerb V : GControlRowsOrder)
	{
		for (int32 Pad = 0; Pad < 2; ++Pad)
		{
			const int32 Cell = static_cast<int32>(V) * 2 + Pad;
			if (CellButtons.IsValidIndex(Cell) && CellButtons[Cell] && CellButtons[Cell]->GetIsEnabled()) Out.Add(CellButtons[Cell]);
		}
	}
	Out.Add(ResetButton.Get());
	Out.Add(BackButton.Get());
	return Out;
}

void UJamControlsWidget::BeginListen(EJamVerb Verb, bool bPad)
{
	if (Verb >= EJamVerb::Count || (bPad && IsSkate(Verb))) return;
	ListenVerb = static_cast<int32>(Verb);
	bListenPad = bPad;
	// Lane A199: no grace. A cell starts listening on its click's release (NativeTick) or on a key's press (NativeOnKeyDown), so the next mouse down is a
	// new press; the grace of two ticks ate a fast player's press. The repeat of the key that opened it is ignored instead (TakeListenPress).
	ListenGraceTicks = 0;
	if (bResetArmed) { bResetArmed = false; if (ResetText) ResetText->SetText(FText::FromString(TEXT("RESET TO DEFAULTS"))); }
	StatusLine = FString::Printf(TEXT("%s: press the new %s (Esc cancels)"), *RowLabel(Verb), bPad ? TEXT("pad button") : TEXT("key or mouse button"));
	if (ListenVeil) ListenVeil->SetVisibility(ESlateVisibility::Visible);
	SetKeyboardFocus();
	RefreshCells();
}

bool UJamControlsWidget::HandleListenedKey(const FKey& InKey)
{
	if (ListenVerb < 0) return false;
	FKey Key = InKey;
	// The digital trigger events stand for the trigger axes the game binds.
	if (Key == EKeys::Gamepad_RightTrigger) Key = EKeys::Gamepad_RightTriggerAxis;
	if (Key == EKeys::Gamepad_LeftTrigger) Key = EKeys::Gamepad_LeftTriggerAxis;
	const EJamVerb V = static_cast<EJamVerb>(ListenVerb);
	auto Stop = [&](const FString& Line)
	{
		ListenVerb = -1;
		StatusLine = Line;
		if (ListenVeil) ListenVeil->SetVisibility(ESlateVisibility::Collapsed);
		RefreshCells();
		return true;
	};
	if (Key == EKeys::Escape || Key == EKeys::Gamepad_Special_Right) return Stop(TEXT("Cancelled: nothing changed."));
	if (Key.IsGamepadKey() != bListenPad)
	{
		StatusLine = bListenPad ? TEXT("That is a key: press a pad button (Esc cancels).") : TEXT("That is a pad button: press a key or a mouse button (Esc cancels).");
		RefreshCells();
		return false;
	}
	if (FJamBindings::IsReserved(Key)) { StatusLine = FString::Printf(TEXT("%s is the game's own (pause, the aim, the signals): pick another."), *UJamInputMap::KeyName(Key)); RefreshCells(); return false; }
	FJamBindings B = UJamInputMap::Get()->GetBindings();
	EJamVerb Swapped = EJamVerb::Count;
	if (!B.Rebind(V, Key, &Swapped))
	{
		// Lane A199: Rebind refuses a move that would leave a verb with no key and no pad button (A198's FAULT 2), as well as a key already first on the row.
		const TArray<FKey>& L = bListenPad ? B[V].Pads : B[V].Keys;
		if (L.Num() > 0 && L[0] == Key) return Stop(FString::Printf(TEXT("%s is already %s's."), *UJamInputMap::KeyName(Key), *RowLabel(V)));
		return Stop(FString::Printf(TEXT("Not moved: %s on %s would leave another control with no key. Free it first."), *UJamInputMap::KeyName(Key), *RowLabel(V)));
	}
	SaveAndApply(B);
	FString Line = FString::Printf(TEXT("%s is now on %s."), *RowLabel(V), *UJamInputMap::KeyName(Key));
	if (Swapped != EJamVerb::Count)
	{
		const FJamVerbBinding& Was = UJamInputMap::Get()->GetBindings()[Swapped];
		const TArray<FKey>& L = bListenPad ? Was.Pads : Was.Keys;
		Line += FString::Printf(TEXT(" It was %s's: %s moved to %s."), *RowLabel(Swapped), *RowLabel(Swapped), L.Num() ? *UJamInputMap::KeyName(L[0]) : TEXT("no key (bind it again)"));
	}
	return Stop(Line);
}

void UJamControlsWidget::SaveAndApply(const FJamBindings& B)
{
	UJamInputMap::Get()->ApplyBindings(B);
	FJamSettings St = FJamSettings::Load();
	St.Controls = UJamInputMap::Get()->GetBindings().ToSaved();
	St.Save();
}

void UJamControlsWidget::RequestReset()
{
	// Lane A199 (item 4): a reset asks first. The first press arms it for ResetConfirmSeconds; the second resets; anything else (a listen, the time) disarms.
	if (IsListening()) HandleListenedKey(EKeys::Escape);
	if (!bResetArmed)
	{
		bResetArmed = true;
		ResetArmedLeft = ResetConfirmSeconds;
		StatusLine = TEXT("Every control back on its default? Press CONFIRM RESET to do it.");
		if (ResetText) ResetText->SetText(FText::FromString(TEXT("CONFIRM RESET")));
		RefreshCells();
		return;
	}
	ResetToDefaults();
}

bool UJamControlsWidget::TakeListenPress(const FKey& Key, bool bRepeat)
{
	if (!IsListening()) return false;
	if (bRepeat) return true;   // the held key that opened the listen (Enter, pad A) repeats: never bound by its own repeat
	HandleListenedKey(Key);
	return true;
}

void UJamControlsWidget::ResetToDefaults()
{
	bResetArmed = false;   // lane A199
	if (ResetText) ResetText->SetText(FText::FromString(TEXT("RESET TO DEFAULTS")));
	ListenVerb = -1;
	if (ListenVeil) ListenVeil->SetVisibility(ESlateVisibility::Collapsed);
	SaveAndApply(FJamBindings::Defaults());
	StatusLine = TEXT("Every control is back on its default.");
	RefreshCells();
}

void UJamControlsWidget::OnResetClicked() { RequestReset(); }   // lane A199: asks first

void UJamControlsWidget::OnBackClicked()
{
	if (IsListening()) { HandleListenedKey(EKeys::Escape); return; }
	OnBack.Broadcast();
}

void UJamControlsWidget::OnCellClicked(int32 Cell)
{
	BeginListen(static_cast<EJamVerb>(Cell / 2), (Cell % 2) == 1);
}

void UJamControlsWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (ListenGraceTicks > 0) --ListenGraceTicks;
	if (bResetArmed && (ResetArmedLeft -= InDeltaTime) <= 0.f)   // lane A199: the confirm lapses
	{
		bResetArmed = false;
		if (ResetText) ResetText->SetText(FText::FromString(TEXT("RESET TO DEFAULTS")));
		StatusLine = TEXT("Reset not confirmed: nothing changed.");
		RefreshCells();
	}
	// The cells' clicks: a UButton's OnClicked carries no index, so the tick asks which cell went from pressed to released (the press and release
	// both inside the button: a click).
	CellWasPressed.SetNum(CellButtons.Num());
	for (int32 I = 0; I < CellButtons.Num(); ++I)
	{
		UButton* B = CellButtons[I];
		if (!B) continue;
		const bool bNow = B->IsPressed();
		bool& bWas = CellWasPressed[I];
		if (bWas && !bNow && B->IsHovered() && !IsListening()) OnCellClicked(I);
		bWas = bNow;
	}
	if (Painter) Painter->Paint(GetOwningPlayer(), !IsListening());   // lane A195: the item look; the hovered item takes the focus (not while a cell listens)
}

FReply UJamControlsWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (TakeListenPress(Key, InKeyEvent.IsRepeat())) return FReply::Handled();   // lane A199: a repeat never binds
	if (InKeyEvent.IsRepeat() && (Key == EKeys::Escape || Key == EKeys::Gamepad_FaceButton_Right)) return FReply::Handled();   // the Esc that cancelled, still held, does not leave
	if (Key == EKeys::Escape || Key == EKeys::Gamepad_FaceButton_Right) { OnBack.Broadcast(); return FReply::Handled(); }
	// The pad's A on a focused cell starts it listening.
	if (Key == EKeys::Gamepad_FaceButton_Bottom || Key == EKeys::Enter)
	{
		for (int32 I = 0; I < CellButtons.Num(); ++I) if (CellButtons[I] && CellButtons[I]->HasAnyUserFocus()) { OnCellClicked(I); return FReply::Handled(); }
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UJamControlsWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (TakeListenPress(InMouseEvent.GetEffectingButton(), false)) return FReply::Handled();   // lane A199: every mouse down while listening is a new press
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UJamControlsWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Lane A199: Slate sends a quick second click as a double-click, not a mouse down: the click on the cell and the press of the new button inside the
	// double-click time were eaten. The second click's button is the press.
	if (TakeListenPress(InMouseEvent.GetEffectingButton(), false)) return FReply::Handled();
	return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
}

FReply UJamControlsWidget::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (IsListening()) { HandleListenedKey(InMouseEvent.GetWheelDelta() > 0.f ? EKeys::MouseScrollUp : EKeys::MouseScrollDown); return FReply::Handled(); }
	return Super::NativeOnMouseWheel(InGeometry, InMouseEvent);
}

FReply UJamControlsWidget::NativeOnAnalogValueChanged(const FGeometry& InGeometry, const FAnalogInputEvent& InAnalogEvent)
{
	const FKey Key = InAnalogEvent.GetKey();
	if (IsListening() && bListenPad && (Key == EKeys::Gamepad_RightTriggerAxis || Key == EKeys::Gamepad_LeftTriggerAxis) && InAnalogEvent.GetAnalogValue() > 0.5f)
	{
		HandleListenedKey(Key);
		return FReply::Handled();
	}
	return Super::NativeOnAnalogValueChanged(InGeometry, InAnalogEvent);
}
