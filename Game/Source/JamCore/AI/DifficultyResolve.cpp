// Lane A3: see DifficultyResolve.h.
#include "DifficultyResolve.h"

#include "JamCartridge.h"
#include "JamJson.h"
#include "JamSimAdapter.h"

namespace JamAI
{
	bool ResolveDifficulty(const UJamCartridge& Cart, FJamDifficultyResolution& Out, const TCHAR* DifficultyOverride)
	{
		Out = FJamDifficultyResolution();
		TSharedPtr<FJamJson> Ai, Values;
		FString ParseErr;
		if (TSharedPtr<FJamJson> Root = FJamJson::Parse(Cart.RawJson, ParseErr))
		{
			if (TSharedPtr<FJamJson> Game = Root->Get(TEXT("game")); Game && Game->IsObject())
			{
				Ai = Game->Get(TEXT("ai"));
			}
			Values = Root->Get(TEXT("values"));
		}
		if (Ai && Ai->IsObject())
		{
			if (TSharedPtr<FJamJson> L = Ai->Get(TEXT("difficulty")); L && L->IsString())
			{
				Out.bLevelWritten = ParseDifficulty(TCHAR_TO_UTF8(*L->String), Out.Level);
			}
		}
		if (DifficultyOverride && *DifficultyOverride)
		{
			if (!ParseDifficulty(TCHAR_TO_UTF8(DifficultyOverride), Out.Level))
			{
				return false;
			}
		}
		ApplyDifficultyProfile(GetDifficultyProfile(Out.Level), Out.Params);
		// 2. explicit game.ai numbers (the loaded values, which the loader read from exactly these keys)
		const FJamAiConfig& G = Cart.Game.Ai;
		struct FKey { const TCHAR* Name; double* Dst; double Src; };
		const FKey Keys[] = {
			{ TEXT("yield_margin_m"), &Out.Params.YieldMarginM, G.YieldMarginM }, { TEXT("yield_radius_m"), &Out.Params.YieldRadiusM, G.YieldRadiusM },
			{ TEXT("hold_radius_m"), &Out.Params.HoldRadiusM, G.HoldRadiusM }, { TEXT("speed_scale"), &Out.Params.SpeedScale, G.SpeedScale },
			{ TEXT("red_skill"), &Out.Params.RedSkill, G.RedSkill },
		};
		for (const FKey& K : Keys)
		{
			if (Ai && Ai->IsObject() && Ai->Has(K.Name))
			{
				*K.Dst = K.Src;
				Out.ExplicitKeys.Add(FString(TEXT("game.ai.")) + K.Name);
			}
		}
		// 3. Designer values (the loader already folded them into Cart.Game.Ai; a value outranks the game field)
		if (Values && Values->IsObject())
		{
			if (Values->Has(TEXT("ai_speed_scale"))) { Out.Params.SpeedScale = G.SpeedScale; Out.ExplicitKeys.Add(TEXT("values.ai_speed_scale")); }
			if (Values->Has(TEXT("ai_red_skill"))) { Out.Params.RedSkill = G.RedSkill; Out.ExplicitKeys.Add(TEXT("values.ai_red_skill")); }
		}
		// sim facts the skills read
		Out.Params.InterceptSpeed = Cart.Game.Skater.Speed;
		Out.Params.AimConeDeg = Cart.Game.Shot.bHasAimCone ? Cart.Game.Shot.AimConeDeg : 0.0;
		return true;
	}

	FJamMatchConfig ToMatchConfigResolved(const UJamCartridge& Cart, const TCHAR* DifficultyOverride)
	{
		FJamMatchConfig C = ToMatchConfig(Cart.Game);
		FJamDifficultyResolution R;
		ResolveDifficulty(Cart, R, DifficultyOverride);
		C.AI = R.Params;
		return C;
	}
}
