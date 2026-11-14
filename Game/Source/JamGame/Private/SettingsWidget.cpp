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
	const FString Base = Barks < 0 ? TEXT("CUT'S OWN") : TEXT("ON");
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
	if (Tier.IsEmpty()) return TEXT("CUT'S OWN");
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

	// The panel sits on the 880 card inside 32 units of padding a side: two columns of 394 with 28 between them.
	const float ColumnTextW = 386.f;

	Root = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("SettingsRoot"));
	T->RootWidget = Root;

	// A heading with its kit icon and a caption under it.
	auto Heading = [&](UVerticalBox* Col, const FString& Icon, const FString& Title, const FString& Caption)
	{
		UHorizontalBox* H = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		if (!Icon.IsEmpty()) AddH(H, S::Icon(T, Icon, 22.f), FMargin(0, 0, 8, 0));
		AddH(H, S::Text(T, Title, S::Font(S::Body, FName("Bold"), S::TrackLabel), S::Yellow()), FMargin(0));
		AddV(Col, H, FMargin(0, 10, 0, 0));
		UTextBlock* C = S::Text(T, Caption, S::CaptionFont(), S::Muted());
		C->SetWrapTextAt(ColumnTextW);   // lane A144: a fixed wrap width, so the screen's measure does not wait for a layout pass
		AddV(Col, C, FMargin(0, 0, 0, 4));
	};

	UHorizontalBox* Columns = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	UVerticalBox* Left = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("SettingsLeft"));
	UVerticalBox* Right = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("SettingsRight"));
	AddH(Columns, Left, FMargin(0, 0, 14, 0), true, VAlign_Top);
	AddH(Columns, Right, FMargin(14, 0, 0, 0), true, VAlign_Top);
	AddV(Root, Columns, FMargin(0));
	UTextBlock* Tmp = nullptr;

	// Left column: the API key, the AI difficulty, the look tier.
	Heading(Left, TEXT("cut-directors"), TEXT("API KEY"), TEXT("Typed here, kept on this machine only; the Designer directs live cuts with it."));
	UHorizontalBox* KeyRow = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	KeyBox = T->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("KeyBox"));
	KeyBox->SetIsPassword(true);
	KeyBox->SetHintText(FText::FromString(TEXT("paste key, Enter")));
	KeyBox->SetMinDesiredWidth(220.f);
	KeyBox->WidgetStyle.SetFont(S::BodyFont());
	KeyBox->WidgetStyle.SetPadding(FMargin(10, 7));
	// Lane A144: chalk on dark (the default light box hid its grey hint).
	KeyBox->WidgetStyle.SetBackgroundColor(FLinearColor(0.16f, 0.17f, 0.20f, 1.f));
	KeyBox->WidgetStyle.SetForegroundColor(S::Chalk());
	KeyBox->WidgetStyle.SetFocusedForegroundColor(S::Chalk());
	KeyBox->OnTextCommitted.AddDynamic(this, &UJamSettingsWidget::OnKeyCommitted);
	AddH(KeyRow, KeyBox, FMargin(0, 0, 8, 0), true);
	ClearKeyButton = Button(T, TEXT("CLEAR"), S::Caption + 1, Tmp);
	ClearKeyText = Tmp;
	ClearKeyButton->OnClicked.AddDynamic(this, &UJamSettingsWidget::OnClearKey);
	AddH(KeyRow, ClearKeyButton, FMargin(0));
	AddV(Left, KeyRow, FMargin(0, 0, 0, 2));
	KeyState = S::Text(T, TEXT(""), S::CaptionFont(), S::Yellow());
	AddV(Left, KeyState, FMargin(0, 0, 0, 6));

	Heading(Left, TEXT("signal-defend"), TEXT("AI DIFFICULTY"), TEXT("For PLAY NOW and the cuts. The season and the modes set their own."));
	DifficultyButton = Button(T, TEXT(""), S::Body + 2, Tmp);
	DifficultyText = Tmp;
	DifficultyButton->OnClicked.AddDynamic(this, &UJamSettingsWidget::OnDifficultyClicked);
	AddV(Left, DifficultyButton, FMargin(0, 0, 0, 6), HAlign_Left);

	Heading(Left, TEXT("upscale"), TEXT("LOOK"), TEXT("The cut's own look, or one look for every cut."));
	TierButton = Button(T, TEXT(""), S::Body + 2, Tmp);
	TierText = Tmp;
	TierButton->OnClicked.AddDynamic(this, &UJamSettingsWidget::OnTierClicked);
	AddV(Left, TierButton, FMargin(0, 0, 0, 6), HAlign_Left);

	// Right column: the volume, the voice signals, the barks.
	Heading(Right, TEXT("voice"), TEXT("VOLUME"), TEXT("Everything the game plays."));
	UHorizontalBox* VolRow = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	VolumeSlider = T->ConstructWidget<USlider>(USlider::StaticClass(), TEXT("VolumeSlider"));
	VolumeSlider->SetStepSize(0.05f);
	VolumeSlider->RequiresControllerLock = false;
	VolumeSlider->IsFocusable = true;
	VolumeSlider->SetWidgetStyle(S::SliderStyle());
	VolumeSlider->OnValueChanged.AddDynamic(this, &UJamSettingsWidget::OnVolumeChanged);
	USizeBox* VolBox = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	VolBox->SetWidthOverride(260.f);
	VolBox->AddChild(VolumeSlider);
	AddH(VolRow, VolBox, FMargin(0, 0, 12, 0));
	VolumeText = S::Text(T, TEXT(""), S::Font(S::Body + 2, FName("Bold")), S::Chalk());
	AddH(VolRow, VolumeText);
	AddV(Right, VolRow, FMargin(0, 4, 0, 6));

	// Voice signals (lane A11's recognizer); keyboard signals 1-4 stay on regardless.
	Heading(Right, TEXT("signal-pass"), TEXT("VOICE SIGNALS"), TEXT("Say a signal instead of pressing 1-4; the keys always work."));
	UHorizontalBox* VoiceRow = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	VoiceButton = Button(T, TEXT(""), S::Body + 2, Tmp);
	VoiceText = Tmp;
	VoiceButton->OnClicked.AddDynamic(this, &UJamSettingsWidget::OnVoiceClicked);
	AddH(VoiceRow, VoiceButton, FMargin(0, 0, 12, 0));
	VoiceStatusText = S::Text(T, TEXT(""), S::CaptionFont(), S::Muted());
	VoiceStatusText->SetWrapTextAt(200.f);
	AddH(VoiceRow, VoiceStatusText, FMargin(0), true);
	AddV(Right, VoiceRow, FMargin(0, 0, 0, 6));

	// Barks (lane A12): off, the cut's own, on; the intensity override rides along.
	Heading(Right, TEXT("barks"), TEXT("BARKS"), TEXT("What the other skaters shout: off, the cut's own, or on (quiet, normal, loud)."));
	BarksButton = Button(T, TEXT(""), S::Body + 2, Tmp);
	BarksText = Tmp;
	BarksButton->OnClicked.AddDynamic(this, &UJamSettingsWidget::OnBarksClicked);
	AddV(Right, BarksButton, FMargin(0, 0, 0, 6), HAlign_Left);

	// Lane A144 (track U2): the controls have a screen of their own, one press from the first screen (UJamMenuFlow::ControlRows, held against the
	// bindings by Jam.Menu.ControlsComplete); the run-on paragraph that stood here split verb from key at every line end.
	Heading(Root, TEXT("settings"), TEXT("CONTROLS"), TEXT("The full list: BACK, then CONTROLS on the first screen. In a match: Esc."));

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
	KeyState->SetText(FText::FromString(bHasKey ? TEXT("key: set") : TEXT("key: none (cached cuts only)")));
	KeyState->SetColorAndOpacity(FSlateColor(bHasKey ? S::Yellow() : S::Muted()));
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
	Out.Add(KeyBox.Get());
	Out.Add(ClearKeyButton.Get());
	Out.Add(DifficultyButton.Get());
	Out.Add(TierButton.Get());
	Out.Add(VolumeSlider.Get());
	Out.Add(VoiceButton.Get());
	Out.Add(BarksButton.Get());
	return Out;
}

void UJamSettingsWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	Tint(ClearKeyButton, ClearKeyText);
	Tint(DifficultyButton, DifficultyText);
	Tint(TierButton, TierText);
	Tint(VoiceButton, VoiceText);
	Tint(BarksButton, BarksText);
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

	UOverlay* Root = T->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("ControlsRoot"));
	T->RootWidget = Root;
	UVerticalBox* Box = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ControlsRows"));
	S::AddO(Root, Box);
	AddV(Box, S::Section(T, TEXT("CONTROLS")), FMargin(0, 0, 0, 4));
	UTextBlock* How = S::Text(T, TEXT("Click a key or a pad button, then press the new one. Esc cancels. Saved at once."), S::CaptionFont(), S::Muted());
	How->SetWrapTextAt(820.f);
	AddV(Box, How, FMargin(0, 0, 0, 6));
	const float VerbW = 290.f, KeysW = 300.f, PadW = 200.f;
	auto Fixed = [&](UHorizontalBox* Row, UWidget* W, float Width)
	{
		USizeBox* Size = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		Size->SetWidthOverride(Width);
		Size->AddChild(W);
		AddH(Row, Size, FMargin(0, 0, 8, 0), false, VAlign_Center);
	};
	{
		UHorizontalBox* Head = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		Fixed(Head, S::Text(T, TEXT(""), S::CaptionFont(), S::Muted()), VerbW);
		Fixed(Head, S::Text(T, TEXT("MOUSE AND KEYS"), S::CaptionFont(), S::Muted()), KeysW);
		Fixed(Head, S::Text(T, TEXT("PAD"), S::CaptionFont(), S::Muted()), PadW);
		AddV(Box, Head, FMargin(0, 0, 0, 2));
	}
	CellButtons.Init(nullptr, FJamBindings::NumVerbs * 2);
	CellTexts.Init(nullptr, FJamBindings::NumVerbs * 2);
	for (const EJamVerb V : GControlRowsOrder)
	{
		UHorizontalBox* Row = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		Fixed(Row, S::Text(T, RowLabel(V), S::Font(S::Body - 2, FName("Black")), S::Yellow()), VerbW);
		for (int32 Pad = 0; Pad < 2; ++Pad)
		{
			const int32 Cell = static_cast<int32>(V) * 2 + Pad;
			UTextBlock* Txt = nullptr;
			UButton* B = Button(T, TEXT(""), S::Caption + 1, Txt);
			FButtonStyle Tight = S::ButtonStyle();   // a row a line high: fifteen rows and the buttons inside the menu card
			Tight.SetNormalPadding(FMargin(12, 2));
			Tight.SetPressedPadding(FMargin(12, 3, 12, 1));
			B->SetStyle(Tight);
			B->SetIsEnabled(!(Pad == 1 && IsSkate(V)));   // the left stick skates: no pad cell for the four directions
			CellButtons[Cell] = B;
			CellTexts[Cell] = Txt;
			// UButton's click carries no payload: the tick tells which cell was clicked (OnCellClicked).
			Fixed(Row, B, Pad ? PadW : KeysW);
		}
		AddV(Box, Row, FMargin(0, 1));
	}
	StatusText = S::Text(T, TEXT(""), S::BodyFont(), S::Yellow());
	StatusText->SetWrapTextAt(820.f);
	AddV(Box, StatusText, FMargin(0, 6, 0, 0));
	UHorizontalBox* Buttons = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	UTextBlock* RT = nullptr;
	ResetButton = Button(T, TEXT("RESET TO DEFAULTS"), S::Body, RT);
	ResetText = RT;
	ResetButton->OnClicked.AddDynamic(this, &UJamControlsWidget::OnResetClicked);
	AddH(Buttons, ResetButton, FMargin(0, 0, 12, 0));
	UTextBlock* BT = nullptr;
	BackButton = Button(T, TEXT("BACK"), S::Body, BT);
	BackText = BT;
	BackButton->OnClicked.AddDynamic(this, &UJamControlsWidget::OnBackClicked);
	AddH(Buttons, BackButton, FMargin(0));
	AddV(Box, Buttons, FMargin(0, 8, 0, 0), HAlign_Left);

	// The veil while a cell listens: it covers the rows so the next mouse press reaches this widget, not a button under the cursor.
	ListenVeil = S::Plate(T, FLinearColor(0.f, 0.f, 0.f, 0.55f), FMargin(0));
	ListenVeil->SetVisibility(ESlateVisibility::Collapsed);
	UTextBlock* Press = S::Text(T, TEXT("PRESS THE NEW KEY  -  ESC CANCELS"), S::Font(S::Body + 4, FName("Black"), 30), S::Yellow());
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
	ListenGraceTicks = 2;
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
	if (!B.Rebind(V, Key, &Swapped)) return Stop(FString::Printf(TEXT("%s is already %s's."), *UJamInputMap::KeyName(Key), *RowLabel(V)));
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

void UJamControlsWidget::ResetToDefaults()
{
	ListenVerb = -1;
	if (ListenVeil) ListenVeil->SetVisibility(ESlateVisibility::Collapsed);
	SaveAndApply(FJamBindings::Defaults());
	StatusLine = TEXT("Every control is back on its default.");
	RefreshCells();
}

void UJamControlsWidget::OnResetClicked() { ResetToDefaults(); }

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
		S::Tint(B, CellTexts[I]);
	}
	S::Tint(ResetButton, ResetText);
	S::Tint(BackButton, BackText);
}

FReply UJamControlsWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (IsListening())
	{
		HandleListenedKey(Key);
		return FReply::Handled();
	}
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
	if (IsListening() && ListenGraceTicks <= 0) { HandleListenedKey(InMouseEvent.GetEffectingButton()); return FReply::Handled(); }
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
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
