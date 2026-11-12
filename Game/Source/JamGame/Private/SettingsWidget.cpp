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

const TCHAR* FJamSettings::Section = TEXT("Jam.Settings");

FJamSettings FJamSettings::Load()
{
	FJamSettings S;
	if (!GConfig) return S;
	FString D;
	if (GConfig->GetString(Section, TEXT("Difficulty"), D, GGameUserSettingsIni)) ParseDifficulty(D, S.Difficulty);
	GConfig->GetString(Section, TEXT("LookTier"), S.LookTierOverride, GGameUserSettingsIni);
	if (!S.LookTierOverride.IsEmpty() && !ULookTierSwitcher::GetKnownTiers().Contains(S.LookTierOverride)) S.LookTierOverride.Empty();
	float V = 1.f;
	if (GConfig->GetFloat(Section, TEXT("Volume"), V, GGameUserSettingsIni)) S.Volume = FMath::Clamp(V, 0.f, 1.f);
	int32 B = -1;
	if (GConfig->GetInt(Section, TEXT("Barks"), B, GGameUserSettingsIni)) S.Barks = FMath::Clamp(B, -1, 1);
	GConfig->GetString(Section, TEXT("BarkIntensity"), S.BarkIntensity, GGameUserSettingsIni);
	if (S.BarkIntensity != TEXT("quiet") && S.BarkIntensity != TEXT("normal") && S.BarkIntensity != TEXT("loud")) S.BarkIntensity.Empty();
	return S;
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

bool FJamSettings::ParseDifficulty(const FString& S, EJamMenuDifficulty& Out)
{
	if (S == TEXT("rookie")) { Out = EJamMenuDifficulty::Rookie; return true; }
	if (S == TEXT("pro")) { Out = EJamMenuDifficulty::Pro; return true; }
	if (S == TEXT("all_star")) { Out = EJamMenuDifficulty::AllStar; return true; }
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

	auto Label = [&](const FString& S) { UTextBlock* L = Text(T, S, 16, Dim()); return L; };

	// API key: masked, stored by the Designer in Saved/Config, never shown back.
	AddV(Root, Label(TEXT("API KEY  (typed here, kept on this machine only; the Designer directs live cuts with it)")), FMargin(0, 10, 0, 2));
	UHorizontalBox* KeyRow = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	KeyBox = T->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("KeyBox"));
	KeyBox->SetIsPassword(true);
	KeyBox->SetHintText(FText::FromString(TEXT("paste a key and press Enter")));
	KeyBox->SetMinDesiredWidth(420.f);
	KeyBox->OnTextCommitted.AddDynamic(this, &UJamSettingsWidget::OnKeyCommitted);
	AddH(KeyRow, KeyBox, FMargin(0, 0, 8, 0), true);
	KeyState = Text(T, TEXT(""), 16, Yellow());
	AddH(KeyRow, KeyState, FMargin(8, 0));
	UTextBlock* Tmp = nullptr;
	ClearKeyButton = Button(T, TEXT("CLEAR"), 16, Tmp);
	ClearKeyText = Tmp;
	ClearKeyButton->OnClicked.AddDynamic(this, &UJamSettingsWidget::OnClearKey);
	AddH(KeyRow, ClearKeyButton, FMargin(8, 0, 0, 0));
	AddV(Root, KeyRow, FMargin(0, 0, 0, 10));

	// AI difficulty (lane A3's enum; a placeholder that stores the choice until the difficulty layer reads it).
	AddV(Root, Label(TEXT("AI DIFFICULTY  (rookie | pro | all_star; read by the difficulty layer when it lands)")), FMargin(0, 10, 0, 2));
	DifficultyButton = Button(T, TEXT(""), 22, Tmp);
	DifficultyText = Tmp;
	DifficultyButton->OnClicked.AddDynamic(this, &UJamSettingsWidget::OnDifficultyClicked);
	AddV(Root, DifficultyButton, FMargin(0, 0, 0, 10), HAlign_Left);

	// Look tier override.
	AddV(Root, Label(TEXT("LOOK  (the cut's own tier, or force one light rig)")), FMargin(0, 10, 0, 2));
	TierButton = Button(T, TEXT(""), 22, Tmp);
	TierText = Tmp;
	TierButton->OnClicked.AddDynamic(this, &UJamSettingsWidget::OnTierClicked);
	AddV(Root, TierButton, FMargin(0, 0, 0, 10), HAlign_Left);

	// Volume.
	AddV(Root, Label(TEXT("VOLUME")), FMargin(0, 10, 0, 2));
	UHorizontalBox* VolRow = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	VolumeSlider = T->ConstructWidget<USlider>(USlider::StaticClass(), TEXT("VolumeSlider"));
	VolumeSlider->SetStepSize(0.05f);
	VolumeSlider->RequiresControllerLock = false;
	VolumeSlider->IsFocusable = true;
	VolumeSlider->SetSliderBarColor(Dim());
	VolumeSlider->SetSliderHandleColor(Yellow());
	VolumeSlider->OnValueChanged.AddDynamic(this, &UJamSettingsWidget::OnVolumeChanged);
	USizeBox* VolBox = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	VolBox->SetWidthOverride(360.f);
	VolBox->AddChild(VolumeSlider);
	AddH(VolRow, VolBox, FMargin(0, 0, 12, 0));
	VolumeText = Text(T, TEXT(""), 20, Chalk());
	AddH(VolRow, VolumeText);
	AddV(Root, VolRow, FMargin(0, 0, 0, 10));

	// Voice signals (lane A11's recognizer); keyboard signals 1-4 stay on regardless.
	AddV(Root, Label(TEXT("VOICE SIGNALS  (say a signal instead of pressing 1-4; the keys always work)")), FMargin(0, 10, 0, 2));
	UHorizontalBox* VoiceRow = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	VoiceButton = Button(T, TEXT(""), 22, Tmp);
	VoiceText = Tmp;
	VoiceButton->OnClicked.AddDynamic(this, &UJamSettingsWidget::OnVoiceClicked);
	AddH(VoiceRow, VoiceButton, FMargin(0, 0, 12, 0));
	VoiceStatusText = Text(T, TEXT(""), 16, Dim(), FName("Regular"));
	AddH(VoiceRow, VoiceStatusText);
	AddV(Root, VoiceRow, FMargin(0, 0, 0, 10));

	// Barks (lane A12): off, the cut's own, on; the intensity override rides along.
	AddV(Root, Label(TEXT("BARKS  (what the other skaters shout: off | cut's own | on, quiet | normal | loud)")), FMargin(0, 10, 0, 2));
	BarksButton = Button(T, TEXT(""), 22, Tmp);
	BarksText = Tmp;
	BarksButton->OnClicked.AddDynamic(this, &UJamSettingsWidget::OnBarksClicked);
	AddV(Root, BarksButton, FMargin(0, 0, 0, 10), HAlign_Left);

	// Controls summary (lane A5's mapping).
	AddV(Root, Label(TEXT("CONTROLS")), FMargin(0, 14, 0, 2));
	UTextBlock* Controls = Text(T,
		TEXT("Skate   WASD / left stick        Shoot   Space / A        Check   Shift / B  (Raw Cut)\n")
		TEXT("Restart   R / View        Pause and Director's Call   Esc / Start        Menus   stick or arrows, A / Enter, B / Esc back"),
		16, Chalk(), FName("Regular"));
	Controls->SetAutoWrapText(true);
	AddV(Root, Controls, FMargin(0, 0, 0, 6));

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
	KeyState->SetColorAndOpacity(FSlateColor(bHasKey ? Yellow() : Dim()));
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
	if (VolumeSlider) VolumeSlider->SetSliderHandleColor(VolumeSlider->HasAnyUserFocus() || VolumeSlider->IsHovered() ? Yellow() : Chalk());
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
