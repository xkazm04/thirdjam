// Lane A2: IJamAISim over lane A1's UJamSim, so the bot tests and the tournament run on the real simulation.
// Also the reference for wiring the brain into the game mode: View() is how a world view is built from UJamSim,
// and Step() is how bot inputs become FJamInput.
#pragma once

#include "CoreMinimal.h"
#include "JamBotMatch.h"
#include "JamCartridge.h"
#include "JamRandom.h"
#include "JamSim.h"
#include "UObject/StrongObjectPtr.h"


namespace JamAI
{
	// FJamGameConfig (the loaded cartridge) -> the match config the runner reads (roster, spawns, game.ai).
	JAMCORE_API FJamMatchConfig ToMatchConfig(const FJamGameConfig& G);

	class JAMCORE_API FJamSimAdapter : public IJamAISim
	{
	public:
		explicit FJamSimAdapter(const FJamGameConfig& InGame);
		virtual ~FJamSimAdapter() override;

		virtual void Reset(const FJamMatchConfig& C, uint64_t Seed) override;
		virtual void View(FJamAIWorld& W) const override;
		virtual FJamAIStepEvents Step(const FJamAIInput* In) override;
		virtual bool IsOver() const override;
		virtual int Winner() const override;
		virtual int Score(int Team) const override;
		virtual uint64_t StateHash() const override;
		virtual double BoardDistance(const FVec2& P) const override;

		UJamSim* GetSim() const { return Sim.Get(); }

		// Fills W from any UJamSim (the game mode uses this to feed UJamBrain).
		static void BuildWorld(const UJamSim& S, FJamAIWorld& W);

	private:
		FJamGameConfig Game;
		TStrongObjectPtr<UJamSim> Sim;
		TArray<FJamInput> Inputs;
	};
}
