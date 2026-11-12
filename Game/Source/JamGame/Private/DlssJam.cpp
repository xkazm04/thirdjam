// Lane A11: see DlssJam.h and docs/experiments/DLSS.md.
#include "DlssJam.h"

#include "HAL/IConsoleManager.h"
#include "Misc/ConfigCacheIni.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "UnrealClient.h"
#include "Misc/CoreDelegates.h"
#include "Modules/ModuleManager.h"

#if JAM_WITH_DLSS
#include "DLSSLibrary.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogJamDlss, Log, All);

namespace JamDlss
{
	static const TCHAR* Section = TEXT("/Script/JamGame.JamDlssSettings");
	static const TCHAR* Key = TEXT("Mode");
	static EJamDlssMode Effective = EJamDlssMode::Off;

	static void SetScreenPercentage(float Percent)
	{
		if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage")))
		{
			CVar->Set(Percent, ECVF_SetByGameSetting);
		}
	}

	static FVector2D ViewportSize()
	{
		if (GEngine && GEngine->GameViewport && GEngine->GameViewport->Viewport)
		{
			const FIntPoint S = GEngine->GameViewport->Viewport->GetSizeXY();
			if (S.X > 0 && S.Y > 0) return FVector2D(S.X, S.Y);
		}
		return FVector2D(1920.0, 1080.0);
	}
}

bool UJamDlssSettings::IsSupported()
{
#if JAM_WITH_DLSS
	if (!FModuleManager::Get().IsModuleLoaded(TEXT("DLSS"))) return false; // before PostEngineInit, or plugin not loaded
	return UDLSSLibrary::IsDLSSSupported();
#else
	return false;
#endif
}

FString UJamDlssSettings::GetUnsupportedReason()
{
#if JAM_WITH_DLSS
	if (!FModuleManager::Get().IsModuleLoaded(TEXT("DLSS"))) return TEXT("DLSS not loaded yet");
	switch (UDLSSLibrary::QueryDLSSSupport())
	{
	case UDLSSSupport::Supported: return FString();
	case UDLSSSupport::NotSupportedIncompatibleHardware: return TEXT("needs an NVIDIA RTX GPU");
	case UDLSSSupport::NotSupportedDriverOutOfDate: return TEXT("NVIDIA driver too old");
	case UDLSSSupport::NotSupportedOperatingSystemOutOfDate: return TEXT("Windows too old");
	case UDLSSSupport::NotSupportedIncompatibleAPICaptureToolActive: return TEXT("a capture tool is active");
	default: return TEXT("not available on this machine");
	}
#else
	return TEXT("not in this build");
#endif
}

FString UJamDlssSettings::ModeToString(EJamDlssMode Mode)
{
	switch (Mode)
	{
	case EJamDlssMode::Auto: return TEXT("auto");
	case EJamDlssMode::Quality: return TEXT("quality");
	case EJamDlssMode::Balanced: return TEXT("balanced");
	case EJamDlssMode::Performance: return TEXT("performance");
	case EJamDlssMode::DLAA: return TEXT("dlaa");
	default: return TEXT("off");
	}
}

bool UJamDlssSettings::ModeFromString(const FString& In, EJamDlssMode& OutMode)
{
	const FString S = In.TrimStartAndEnd().ToLower();
	if (S == TEXT("auto")) { OutMode = EJamDlssMode::Auto; return true; }
	if (S == TEXT("quality")) { OutMode = EJamDlssMode::Quality; return true; }
	if (S == TEXT("balanced")) { OutMode = EJamDlssMode::Balanced; return true; }
	if (S == TEXT("performance")) { OutMode = EJamDlssMode::Performance; return true; }
	if (S == TEXT("dlaa")) { OutMode = EJamDlssMode::DLAA; return true; }
	if (S == TEXT("off")) { OutMode = EJamDlssMode::Off; return true; }
	return false;
}

EJamDlssMode UJamDlssSettings::GetMode()
{
	FString S;
	EJamDlssMode M = EJamDlssMode::Auto;
	if (GConfig && GConfig->GetString(JamDlss::Section, JamDlss::Key, S, GGameUserSettingsIni))
	{
		ModeFromString(S, M);
	}
	return M;
}

EJamDlssMode UJamDlssSettings::GetEffectiveMode()
{
	return JamDlss::Effective;
}

TArray<EJamDlssMode> UJamDlssSettings::GetAvailableModes()
{
	if (!IsSupported()) return { EJamDlssMode::Off };
	return { EJamDlssMode::Auto, EJamDlssMode::Quality, EJamDlssMode::Balanced, EJamDlssMode::Performance, EJamDlssMode::DLAA, EJamDlssMode::Off };
}

void UJamDlssSettings::SetMode(EJamDlssMode Mode)
{
	if (GConfig)
	{
		GConfig->SetString(JamDlss::Section, JamDlss::Key, *ModeToString(Mode), GGameUserSettingsIni);
		GConfig->Flush(false, GGameUserSettingsIni);
	}
	Apply(Mode);
}

void UJamDlssSettings::ApplySaved()
{
	Apply(GetMode());
}

void UJamDlssSettings::Apply(EJamDlssMode Mode)
{
#if JAM_WITH_DLSS
	if (!IsSupported() || Mode == EJamDlssMode::Off)
	{
		if (IsSupported()) UDLSSLibrary::EnableDLSS(false);
		JamDlss::SetScreenPercentage(100.f);
		JamDlss::Effective = EJamDlssMode::Off;
		UE_LOG(LogJamDlss, Log, TEXT("DLSS off (requested %s%s%s); engine upscaler/AA stays as configured"), *ModeToString(Mode),
			IsSupported() ? TEXT("") : TEXT(", unsupported: "), *GetUnsupportedReason());
		return;
	}

	UDLSSMode PluginMode = UDLSSMode::Auto;
	switch (Mode)
	{
	case EJamDlssMode::Quality: PluginMode = UDLSSMode::Quality; break;
	case EJamDlssMode::Balanced: PluginMode = UDLSSMode::Balanced; break;
	case EJamDlssMode::Performance: PluginMode = UDLSSMode::Performance; break;
	case EJamDlssMode::DLAA: PluginMode = UDLSSMode::DLAA; break;
	default: PluginMode = UDLSSMode::Auto; break;
	}

	bool bModeSupported = false, bFixed = false;
	float Optimal = 100.f, MinPct = 100.f, MaxPct = 100.f, Sharpness = 0.f;
	UDLSSLibrary::GetDLSSModeInformation(PluginMode, JamDlss::ViewportSize(), bModeSupported, Optimal, bFixed, MinPct, MaxPct, Sharpness);
	if (!bModeSupported)
	{
		// Fall back to the plugin's own recommendation for this resolution.
		UDLSSLibrary::GetDLSSModeInformation(UDLSSMode::Auto, JamDlss::ViewportSize(), bModeSupported, Optimal, bFixed, MinPct, MaxPct, Sharpness);
	}
	if (PluginMode == UDLSSMode::DLAA) Optimal = 100.f;
	if (Optimal <= 0.f) Optimal = 100.f;

	JamDlss::SetScreenPercentage(Optimal);
	UDLSSLibrary::EnableDLSS(true);
	JamDlss::Effective = Mode;
	UE_LOG(LogJamDlss, Log, TEXT("DLSS on: mode %s, screen percentage %.1f"), *ModeToString(Mode), Optimal);
#else
	JamDlss::Effective = EJamDlssMode::Off;
	UE_LOG(LogJamDlss, Log, TEXT("DLSS not in this build; mode %s ignored"), *ModeToString(Mode));
#endif
}

void UJamDlssSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
#if JAM_WITH_DLSS
	// In a packaged game the game instance exists before PostEngineInit, when the DLSS module (LoadingPhase PostEngineInit)
	// is not loaded yet (OnPostEngineInit fires before PostEngineInit modules load); apply when the engine loop init completes.
	if (!FModuleManager::Get().IsModuleLoaded(TEXT("DLSS")))
	{
		FCoreDelegates::OnFEngineLoopInitComplete.AddLambda([]() { UJamDlssSettings::ApplySaved(); });
		return;
	}
#endif
	UJamDlssSettings::ApplySaved();
}

// Console: jam.dlss [auto|quality|balanced|performance|dlaa|off]  (no argument prints the state)
static FAutoConsoleCommand GJamDlssCommand(
	TEXT("jam.dlss"),
	TEXT("jam.dlss [auto|quality|balanced|performance|dlaa|off]: set and save the DLSS mode; no argument prints support and mode"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
	{
		EJamDlssMode M;
		if (Args.Num() > 0 && UJamDlssSettings::ModeFromString(Args[0], M))
		{
			UJamDlssSettings::SetMode(M);
		}
		UE_LOG(LogJamDlss, Display, TEXT("jam.dlss supported=%d reason='%s' saved=%s effective=%s"),
			UJamDlssSettings::IsSupported() ? 1 : 0, *UJamDlssSettings::GetUnsupportedReason(),
			*UJamDlssSettings::ModeToString(UJamDlssSettings::GetMode()), *UJamDlssSettings::ModeToString(UJamDlssSettings::GetEffectiveMode()));
	}));
