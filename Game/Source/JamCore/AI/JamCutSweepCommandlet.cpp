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
#include "UObject/StrongObjectPtr.h"

DEFINE_LOG_CATEGORY_STATIC(LogJamCutSweep, Log, All);

namespace JamCutSweepLocal
{
	// The headless match as the game mode builds it (HokejGameMode::BuildMatch / StepOnce): rule hook set before Init so the
	// opening faceoff reaches it, zone source before Init, effect system bound after Init; per frame: score mirror, sim step,
	// zone update, effect step. The brain's world view gains the slow-ice circles and the no_shoot flags the game mode adds.
	class FJamCutSim : public JamAI::FJamSimAdapter, public IJamRuleHook
	{
	public:
		explicit FJamCutSim(const UJamCartridge& InCart)
			: FJamSimAdapter(InCart.Game), Cart(&InCart)
		{
		}

		virtual void Reset(const JamAI::FJamMatchConfig& C, uint64_t Seed) override
		{
			Rules.Reset(NewObject<UJamRuleSystem>());
			Rules->Init(Cart->Game);
			GetSim()->SetRuleHook(this);
			Zones.Reset(NewObject<UJamZoneSystem>());
			Zones->Init(Cart->Game);
			GetSim()->SetZoneSource(Zones.Get());
			FJamSimAdapter::Reset(C, Seed);
			Effects.Reset(NewObject<UJamEffectSystem>());
			EffectErrors.Reset();
			Effects->Init(GetSim(), Cart->RawJson, EffectErrors);
			Count = FCounts();
			CountSimEvents();   // the opening faceoff / TAKE 1 events
		}

		virtual void View(JamAI::FJamAIWorld& W) const override
		{
			FJamSimAdapter::View(W);
			if (!Zones.IsValid()) { return; }
			W.NumSlowIce = 0;
			for (const FJamZoneConfig& Z : Zones->GetZones())
			{
				if (Z.Kind != FName(TEXT("slow_ice")) || W.NumSlowIce >= JamAI::FJamAIWorld::kMaxSlowIce) { continue; }
				const bool bRect = Z.Shape == FName(TEXT("rect"));
				W.SlowIceCentre[W.NumSlowIce] = JamAI::FVec2(Z.X, Z.Z);
				W.SlowIceRadius[W.NumSlowIce] = bRect ? 0.5 * FMath::Sqrt(Z.W * Z.W + Z.D * Z.D) : Z.R;
				++W.NumSlowIce;
			}
			for (int32 i = 0; i < W.NumSkaters; ++i)
			{
				FName ZoneId;
				W.Skaters[i].bNoShoot = Zones->BlocksShot(FVector2D(W.Skaters[i].Pos.X, W.Skaters[i].Pos.Y), ZoneId);
			}
		}

		virtual JamAI::FJamAIStepEvents Step(const JamAI::FJamAIInput* In) override
		{
			UJamSim* S = GetSim();
			Rules->SetScore(0, S->GetScore(0));
			Rules->SetScore(1, S->GetScore(1));
			const JamAI::FJamAIStepEvents Ev = FJamSimAdapter::Step(In);
			Zones->Update(S->GetFrame(), S->GetSkaters(), S->GetPuck());
			Effects->Step();
			CountSimEvents();
			const int32 Human = S->GetHumanIndex();
			for (const FJamSimEvent& E : Effects->GetFrameEvents())
			{
				if (E.Name == JamEvents::PickupTake && E.Who == Human) { ++Count.HumanPickups; }
			}
			return Ev;
		}

		// IJamRuleHook: forward to the rule system and count the actions it emits.
		virtual void OnRuleEvent(const FJamRuleEventIn& In, TArray<FJamRuleActionOut>& Out) override
		{
			const int32 Before = Out.Num();
			Rules->OnRuleEvent(In, Out);
			for (int32 i = Before; i < Out.Num(); ++i)
			{
				if (Out[i].Action == EJamRuleAction::EndMatch) { ++Count.EndMatchActions; }
				else if (Out[i].Action == EJamRuleAction::EndPeriod) { ++Count.EndPeriodActions; }
			}
		}

		struct FCounts
		{
			int32 Periods = 0, Announces = 0, ShotsBlocked = 0, Checks = 0, Knockdowns = 0, Fouls = 0, HumanPickups = 0, EndMatchActions = 0, EndPeriodActions = 0;
		};

		// The "sweep" object of the JSON line.
		FString SweepJson(const FString& Level) const
		{
			const UJamSim* S = GetSim();
			const FJamRulesConfig& R = Cart->Game.Rules;
			const bool bOver = S->GetPhase() == EJamSimPhase::Over;
			const bool bFirstTo = R.Mode == FName(TEXT("first_to"));
			const bool bTimer = R.Mode == FName(TEXT("timer"));
			const int32 Top = FMath::Max(S->GetScore(0), S->GetScore(1));
			FString Cause = TEXT("none");
			if (bOver)
			{
				if (Count.EndMatchActions > 0) { Cause = TEXT("rule"); }
				else if (bFirstTo && R.ScoreToWin > 0 && Top >= R.ScoreToWin) { Cause = TEXT("score"); }
				else if (bTimer && S->GetTimerFrames() <= 0) { Cause = TEXT("timer"); }
				else { Cause = TEXT("other"); }
			}
			FString Errs;
			for (const FString& E : EffectErrors) { Errs += (Errs.IsEmpty() ? TEXT("") : TEXT(" | ")) + E.ReplaceCharWithEscapedChar(); }
			return FString::Printf(TEXT("{\"level\":\"%s\",\"rules_mode\":\"%s\",\"score_to_win\":%d,\"timer_frames\":%d,\"hooks\":%d,\"rule_fires\":%d,\"end_match_actions\":%d,\"end_period_actions\":%d,")
				TEXT("\"pickups_spawned\":%d,\"pickups_taken\":%d,\"human_pickups\":%d,\"effects_started\":%d,\"zones\":%d,\"zone_enters\":%d,\"periods\":%d,\"period_events\":%d,")
				TEXT("\"announces\":%d,\"shots_blocked\":%d,\"checks\":%d,\"knockdowns\":%d,\"fouls\":%d,\"rink_revision\":%d,\"rink_m\":[%g,%g],\"end_cause\":\"%s\",\"effect_errors\":\"%s\"}"),
				*Level, *R.Mode.ToString(), R.ScoreToWin, R.TimerFrames, Rules->GetHookCount(), Rules->GetTotalFires(), Count.EndMatchActions, Count.EndPeriodActions,
				Effects->GetPickupsSpawned(), Effects->GetPickupsTaken(), Count.HumanPickups, Effects->GetEffectsStarted(), Zones->GetZoneCount(), Zones->GetEnterCount(),
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
		TStrongObjectPtr<UJamRuleSystem> Rules;
		TStrongObjectPtr<UJamZoneSystem> Zones;
		TStrongObjectPtr<UJamEffectSystem> Effects;
		TArray<FString> EffectErrors;
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
