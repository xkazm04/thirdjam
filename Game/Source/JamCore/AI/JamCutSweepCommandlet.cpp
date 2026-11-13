// Lane A37: see JamCutSweepCommandlet.h.
#include "JamCutSweepCommandlet.h"

#include "JamBotMatch.h"
#include "JamSimAdapter.h"
#include "JamCartridge.h"
#include "JamEffectSystem.h"
#include "JamEventBus.h"
#include "JamRuleSystem.h"
#include "JamZones.h"
#include "DifficultyResolve.h"
#include "Misc/Parse.h"

DEFINE_LOG_CATEGORY_STATIC(LogJamCutSweep, Log, All);

namespace JamCutSweepLocal
{
	// Lane A41: the headless match with the game mode's systems is FJamSimAdapter built from the cartridge (JamSimAdapter.h, FJamSystems:
	// rule hook before Init, zone source before Init, effects after; per frame score mirror, sim step, zone update, effect step; the
	// brain's view with slow-ice circles and no_shoot). This class only counts what the sweep's JSON line reports.
	class FJamCutSim : public JamAI::FJamSimAdapter
	{
	public:
		explicit FJamCutSim(const UJamCartridge& InCart)
			: FJamSimAdapter(InCart), Cart(&InCart)
		{
		}

		virtual void Reset(const JamAI::FJamMatchConfig& C, uint64_t Seed) override
		{
			FJamSimAdapter::Reset(C, Seed);
			Count = FCounts();
			CountSimEvents();   // the opening faceoff / TAKE 1 events
		}

		virtual JamAI::FJamAIStepEvents Step(const JamAI::FJamAIInput* In) override
		{
			const JamAI::FJamAIStepEvents Ev = FJamSimAdapter::Step(In);
			CountSimEvents();
			const int32 Human = GetSim()->GetHumanIndex();
			if (const UJamEffectSystem* Fx = GetSystems().Effects.Get())
			{
				for (const FJamSimEvent& E : Fx->GetFrameEvents())
				{
					if (E.Name == JamEvents::PickupTake && E.Who == Human) { ++Count.HumanPickups; }
				}
			}
			return Ev;
		}

		struct FCounts
		{
			int32 Periods = 0, Announces = 0, ShotsBlocked = 0, Checks = 0, Knockdowns = 0, Fouls = 0, HumanPickups = 0;
		};

		// The "sweep" object of the JSON line.
		FString SweepJson(const FString& Level) const
		{
			const UJamSim* S = GetSim();
			const JamAI::FJamSystems& Sys = GetSystems();
			const FJamRulesConfig& R = Cart->Game.Rules;
			const bool bOver = S->GetPhase() == EJamSimPhase::Over;
			const bool bFirstTo = R.Mode == FName(TEXT("first_to"));
			const bool bTimer = R.Mode == FName(TEXT("timer"));
			const int32 Top = FMath::Max(S->GetScore(0), S->GetScore(1));
			FString Cause = TEXT("none");
			if (bOver)
			{
				if (Sys.EndMatchActions > 0) { Cause = TEXT("rule"); }
				else if (bFirstTo && R.ScoreToWin > 0 && Top >= R.ScoreToWin) { Cause = TEXT("score"); }
				else if (bTimer && S->GetTimerFrames() <= 0) { Cause = TEXT("timer"); }
				else { Cause = TEXT("other"); }
			}
			FString Errs;
			for (const FString& E : Sys.EffectErrors) { Errs += (Errs.IsEmpty() ? TEXT("") : TEXT(" | ")) + E.ReplaceCharWithEscapedChar(); }
			const UJamRuleSystem* Rules = Sys.Rules.Get();
			const UJamZoneSystem* Zones = Sys.Zones.Get();
			const UJamEffectSystem* Effects = Sys.Effects.Get();
			return FString::Printf(TEXT("{\"level\":\"%s\",\"rules_mode\":\"%s\",\"score_to_win\":%d,\"timer_frames\":%d,\"hooks\":%d,\"rule_fires\":%d,\"end_match_actions\":%d,\"end_period_actions\":%d,")
				TEXT("\"pickups_spawned\":%d,\"pickups_taken\":%d,\"human_pickups\":%d,\"effects_started\":%d,\"zones\":%d,\"zone_enters\":%d,\"periods\":%d,\"period_events\":%d,")
				TEXT("\"announces\":%d,\"shots_blocked\":%d,\"checks\":%d,\"knockdowns\":%d,\"fouls\":%d,\"rink_revision\":%d,\"rink_m\":[%g,%g],\"end_cause\":\"%s\",\"effect_errors\":\"%s\"}"),
				*Level, *R.Mode.ToString(), R.ScoreToWin, R.TimerFrames, Rules ? Rules->GetHookCount() : 0, Rules ? Rules->GetTotalFires() : 0, Sys.EndMatchActions, Sys.EndPeriodActions,
				Effects ? Effects->GetPickupsSpawned() : 0, Effects ? Effects->GetPickupsTaken() : 0, Count.HumanPickups, Effects ? Effects->GetEffectsStarted() : 0,
				Zones ? Zones->GetZoneCount() : 0, Zones ? Zones->GetEnterCount() : 0,
				S->GetPeriod(), Count.Periods, Count.Announces, Count.ShotsBlocked, Count.Checks, Count.Knockdowns, Count.Fouls, S->GetRinkRevision(),
				S->GetConfig().Rink.LengthM, S->GetConfig().Rink.WidthM, *Cause, *Errs);
		}

	private:
		void CountSimEvents()
		{
			for (const FJamSimEvent& E : GetSim()->GetFrameEvents())
			{
				if (E.Name == JamEvents::Period) { ++Count.Periods; }
				else if (E.Name == JamEvents::Announce) { ++Count.Announces; }
				else if (E.Name == JamEvents::ShotBlocked) { ++Count.ShotsBlocked; }
				else if (E.Name == JamEvents::Check) { ++Count.Checks; }
				else if (E.Name == JamEvents::Knockdown) { ++Count.Knockdowns; }
				else if (E.Name == JamEvents::Foul) { ++Count.Fouls; }
			}
		}

		const UJamCartridge* Cart = nullptr;
		FCounts Count;
	};
}

UJamCutSweepCommandlet::UJamCutSweepCommandlet()
{
	IsClient = false;
	IsServer = false;
	LogToConsole = true;
	ShowErrorCount = true;
}

int32 UJamCutSweepCommandlet::Main(const FString& Params)
{
	FString Cartridge = TEXT("hockey.regular"), ModeText = TEXT("playerbot"), Seeds = TEXT("1-5"), DifficultyArg;
	int64 MaxFrames = 36000;
	FParse::Value(*Params, TEXT("-cartridge="), Cartridge, false);
	FParse::Value(*Params, TEXT("-mode="), ModeText, false);
	FParse::Value(*Params, TEXT("-seeds="), Seeds, false);
	FParse::Value(*Params, TEXT("-maxframes="), MaxFrames);
	FParse::Value(*Params, TEXT("-difficulty="), DifficultyArg, false);

	JamAI::EJamPlayerMode Mode = JamAI::EJamPlayerMode::PlayerBot;
	if (!JamAI::ParseMode(TCHAR_TO_UTF8(*ModeText), Mode))
	{
		UE_LOG(LogJamCutSweep, Error, TEXT("FAIL cutsweep bad -mode=%s"), *ModeText);
		return 2;
	}
	const FString Path = UJamCartridge::ResolveCartridgePath(Cartridge);
	const FString Label = Cartridge.Replace(TEXT("\\"), TEXT("/")).Replace(TEXT("\""), TEXT("'"));
	TArray<FString> Errors;
	UJamCartridge* Cart = UJamCartridge::LoadFromFile(Path, Errors);
	if (!Cart)
	{
		const FString Joined = FString::Join(Errors, TEXT(" | ")).ReplaceCharWithEscapedChar();
		UE_LOG(LogJamCutSweep, Display, TEXT("{\"tool\":\"thirdjam.tournament/1\",\"cartridge\":\"%s\",\"load_error\":\"%s\"}"), *Label, *Joined);
		return 0;
	}
	Cart->AddToRoot();

	JamAI::FJamDifficultyResolution Res;
	JamAI::ResolveDifficulty(*Cart, Res, DifficultyArg.IsEmpty() ? nullptr : *DifficultyArg);
	const JamAI::FJamMatchConfig Cfg = JamAI::ToMatchConfigResolved(*Cart, DifficultyArg.IsEmpty() ? nullptr : *DifficultyArg);
	const FString Level = UTF8_TO_TCHAR(JamAI::DifficultyName(Res.Level));

	FString Lo, Hi;
	if (!Seeds.Split(TEXT("-"), &Lo, &Hi)) { Lo = Hi = Seeds; }
	const int32 SeedLo = FCString::Atoi(*Lo);
	const int32 SeedHi = FCString::Atoi(*Hi);
	static char Line[8192];
	const FTCHARToUTF8 CartUtf8(*Label);
	for (int32 Seed = SeedLo; Seed <= SeedHi; ++Seed)
	{
		JamCutSweepLocal::FJamCutSim Sim(*Cart);
		const JamAI::FJamMatchStats St = JamAI::RunMatch(Sim, Cfg, static_cast<uint64_t>(Seed), Mode, MaxFrames, 600);
		JamAI::FormatStatsJson(St, CartUtf8.Get(), JamAI::ModeName(Mode), Line, sizeof(Line));
		FString Out = UTF8_TO_TCHAR(Line);
		Out.LeftChopInline(1);
		Out += FString::Printf(TEXT(",\"sim\":\"jam\",\"difficulty\":\"%s\",\"title\":\"%s\",\"sweep\":%s}"), *Level, *Cart->Title.ReplaceCharWithEscapedChar(), *Sim.SweepJson(Level));
		UE_LOG(LogJamCutSweep, Display, TEXT("%s"), *Out);
	}
	Cart->RemoveFromRoot();
	return 0;
}
