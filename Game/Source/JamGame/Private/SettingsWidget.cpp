#include "SettingsWidget.h"
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
#include "Engine/GameInstance.h"
#include "Engine/World.h"

using namespace JamMenuStyle;
using S = FJamUiStyle;

// Lane A18 (visual layer): two columns (key, difficulty, look | volume, voice, barks) with kit icons, the controls under both;
// type and plates from FJamUiStyle. The settings themselves (load, save, apply) are lane A8's.

const TCHAR* FJamSettings::Section = TEXT("Jam.Settings");

FJamSettings FJamSettings::Load()
{
	FJamSettings St;
	if (!GConfig) return St;
	FString D;
	if (GConfig->GetString(Section, TEXT("Difficulty"), D, GGameUserSettingsIni)) ParseDifficulty(D, St.Difficulty);
	GConfig->GetString(Section, TEXT("LookTier"), St.LookTierOverride, GGameUserSettingsIni);
	if (!St.LookTierOverride.IsEmpty() && !ULookTierSwitcher::GetKnownTiers().Contains(St.LookTierOverride)) St.LookTierOverride.Empty();
	float V = 1.f;
	if (GConfig->GetFloat(Section, TEXT("Volume"), V, GGameUserSettingsIni)) St.Volume = FMath::Clamp(V, 0.f, 1.f);
	int32 B = -1;
	if (GConfig->GetInt(Section, TEXT("Barks"), B, GGameUserSettingsIni)) St.Barks = FMath::Clamp(B, -1, 1);
	GConfig->GetString(Section, TEXT("BarkIntensity"), St.BarkIntensity, GGameUserSettingsIni);
	if (St.BarkIntensity != TEXT("quiet") && St.BarkIntensity != TEXT("normal") && St.BarkIntensity != TEXT("loud")) St.BarkIntensity.Empty();
	return St;
}

void FJamSettings::Save() const
{
	if (!GConfig) return;
	GConfig->SetString(Section, TEXT("Difficulty"), *DifficultyString(Difficulty), GGameUserSettingsIni);
	GConfig->SetString(Section, TEXT("LookTier"), *LookTierOverride, GGameUserSettingsIni);
	GConfig->SetFloat(Section, TEXT("Volume"), Volume, GGameUserSettingsIni);
	GConfig->SetInt(Section, TEXT("Barks"), Barks, GGameUserSettingsIni);
	GConfig->SetString(Section, TEXT("BarkIntensity"), *BarkIntensity, GGameUserSettingsIni);
	GConfig->Flush(false, GGameUserSettingsIni);
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
		C->SetAutoWrapText(true);
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
	KeyBox->SetHintText(FText::FromString(TEXT("paste a key and press Enter")));
	KeyBox->SetMinDesiredWidth(220.f);
	KeyBox->WidgetStyle.SetFont(S::BodyFont());
	KeyBox->WidgetStyle.SetPadding(FMargin(10, 7));
	KeyBox->OnTextCommitted.AddDynamic(this, &UJamSettingsWidget::OnKeyCommitted);
	AddH(KeyRow, KeyBox, FMargin(0, 0, 8, 0), true);
	ClearKeyButton = Button(T, TEXT("CLEAR"), S::Caption + 1, Tmp);
	ClearKeyText = Tmp;
	ClearKeyButton->OnClicked.AddDynamic(this, &UJamSettingsWidget::OnClearKey);
	AddH(KeyRow, ClearKeyButton, FMargin(0));
	AddV(Left, KeyRow, FMargin(0, 0, 0, 2));
	KeyState = S::Text(T, TEXT(""), S::CaptionFont(), S::Yellow());
	AddV(Left, KeyState, FMargin(0, 0, 0, 6));

	Heading(Left, TEXT("signal-defend"), TEXT("AI DIFFICULTY"), TEXT("Rookie, pro or all-star opponents."));
	DifficultyButton = Button(T, TEXT(""), S::Body + 2, Tmp);
	DifficultyText = Tmp;
	DifficultyButton->OnClicked.AddDynamic(this, &UJamSettingsWidget::OnDifficultyClicked);
	AddV(Left, DifficultyButton, FMargin(0, 0, 0, 6), HAlign_Left);

	Heading(Left, TEXT("upscale"), TEXT("LOOK"), TEXT("The cut's own tier, or force one light rig."));
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
	VoiceStatusText->SetAutoWrapText(true);
	AddH(VoiceRow, VoiceStatusText, FMargin(0), true);
	AddV(Right, VoiceRow, FMargin(0, 0, 0, 6));

	// Barks (lane A12): off, the cut's own, on; the intensity override rides along.
	Heading(Right, TEXT("barks"), TEXT("BARKS"), TEXT("What the other skaters shout: off, the cut's own, or on (quiet, normal, loud)."));
	BarksButton = Button(T, TEXT(""), S::Body + 2, Tmp);
	BarksText = Tmp;
	BarksButton->OnClicked.AddDynamic(this, &UJamSettingsWidget::OnBarksClicked);
	AddV(Right, BarksButton, FMargin(0, 0, 0, 6), HAlign_Left);

	// Controls summary (lane A5's mapping) under both columns.
	Heading(Root, TEXT("settings"), TEXT("CONTROLS"), TEXT(""));
	UTextBlock* Controls = S::Text(T,
		TEXT("Skate   WASD / left stick        Shoot   Space / A        Check   Shift / B  (Raw Cut)\n")
		TEXT("Restart   R / View        Pause and Director's Call   Esc / Start        Menus   stick or arrows, A / Enter, B / Esc back"),
		S::BodyFont(), S::Chalk());
	Controls->SetAutoWrapText(true);
	AddV(Root, Controls, FMargin(0, -2, 0, 6));

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
	DifficultyText->SetText(FText::FromString(FJamSettings::DifficultyLabel(Settings.Difficulty)));
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
	Settings.Difficulty = static_cast<EJamMenuDifficulty>((static_cast<uint8>(Settings.Difficulty) + 1) % 3);
	Settings.Save();
	RefreshLabels();
}

void UJamSettingsWidget::OnTierClicked()
{
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
	Settings.Volume = FMath::Clamp(V, 0.f, 1.f);
	Settings.Apply();
	Settings.Save();
	VolumeText->SetText(FText::FromString(FString::Printf(TEXT("%d %%"), FMath::RoundToInt(Settings.Volume * 100.f))));
}
