// Lane A2: see JamBrain.h.
#include "JamBrain.h"

void UJamBrain::ApplyParams()
{
	Core.Params.YieldMarginM = YieldMarginM;
	Core.Params.YieldRadiusM = YieldRadiusM;
	Core.Params.HoldRadiusM = HoldRadiusM;
	Core.Params.SpeedScale = SpeedScale;
	Core.Params.RedSkill = RedSkill;
}

void UJamBrain::ResetBrain()
{
	ApplyParams();
	Core.Reset();
}

void UJamBrain::Think(const JamAI::FJamAIWorld& W, JamAI::FJamAIInput* Out)
{
	Core.Think(W, Out);
}

JamAI::FJamAIInput UJamPlayerBot::Think(const JamAI::FJamAIWorld& W) const
{
	JamAI::FJamPlayerBot Bot;
	if (!bWinnerVariant)
	{
		Bot.ShootRangeM = 22.0;
		Bot.bPeelToMiddle = false;
	}
	return Bot.Think(W);
}
