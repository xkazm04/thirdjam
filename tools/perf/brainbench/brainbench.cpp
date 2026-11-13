// Lane A106 (track Q3): the brain's cost per frame at five a side, outside the engine. Runs bot matches on the reference sim (the tournament driver's
// sources), records every world the bots saw, then times FJamBrain::Think and ReadSpace over those worlds with a steady clock. Measuring only:
// no decision changes. Build and run: node tools/perf/brainbench/run.mjs
#include "JamBotMatch.h"
#include "JamBotRefSim.h"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>

static std::vector<JamAI::FJamAIWorld>* GWorlds = nullptr;
static void Capture(void*, const JamAI::FJamAIWorld& W, const JamAI::FJamAIInput*, const JamAI::FJamBrain&) { GWorlds->push_back(W); }

int main(int Argc, char** Argv)
{
	const char* CfgText = Argc > 1 ? Argv[1] : "";
	const int Seeds = Argc > 2 ? std::atoi(Argv[2]) : 3;
	const long long MaxFrames = Argc > 3 ? std::atoll(Argv[3]) : 7200;
	JamAI::FJamMatchConfig Cfg;
	Cfg.SetRegular();
	char Err[512] = {};
	if (!Cfg.Parse(CfgText, Err, sizeof(Err))) { std::fprintf(stderr, "bad cfg: %s\n", Err); return 2; }
	std::vector<JamAI::FJamAIWorld> Worlds;
	GWorlds = &Worlds;
	JamAI::EJamPlayerMode Mode; JamAI::ParseMode("brain", Mode);
	for (int s = 1; s <= Seeds; ++s) { JamAI::FRefSim Sim; JamAI::RunMatch(Sim, Cfg, static_cast<uint64_t>(s), Mode, MaxFrames, 600, &Capture, nullptr, 1); }
	if (Worlds.empty()) { std::fprintf(stderr, "no worlds\n"); return 2; }
	std::vector<double> Us; Us.reserve(Worlds.size());
	JamAI::FJamBrain Brain; Brain.Reset(); Brain.Params = Cfg.AI;
	JamAI::FJamAIInput Out[JamAI::kMaxSkaters];
	using Clk = std::chrono::steady_clock;
	for (int rep = 0; rep < 3; ++rep)
	{
		Us.clear();
		for (const JamAI::FJamAIWorld& W : Worlds)
		{
			const auto t0 = Clk::now();
			Brain.Think(W, Out);
			Us.push_back(std::chrono::duration<double, std::micro>(Clk::now() - t0).count());
		}
	}
	std::vector<double> Sorted = Us; std::sort(Sorted.begin(), Sorted.end());
	double Sum = 0; for (double u : Us) Sum += u;
	// the space read alone, once per world for the skater nearest the puck (the brain calls it at most once per carrier bot per frame)
	double SpaceSum = 0; volatile int Sink = 0;
	for (const JamAI::FJamAIWorld& W : Worlds)
	{
		int C = 0; double Bd = 1e18;
		for (int i = 0; i < W.NumSkaters; ++i) { const double D = W.Skaters[i].Pos.DistTo(W.PuckPos); if (D < Bd) { Bd = D; C = i; } }
		const auto t0 = Clk::now();
		const JamAI::FJamSpaceRead R = JamAI::ReadSpace(W, C, W.OppGoal(W.Skaters[C].Team) - W.Skaters[C].Pos, 3.0, -1);
		SpaceSum += std::chrono::duration<double, std::micro>(Clk::now() - t0).count();
		Sink += R.NumOpen;
	}
	std::printf("{\"schema\":\"a106.brainbench/1\",\"skaters\":%d,\"frames\":%zu,\"think_mean_us\":%.3f,\"think_p50_us\":%.3f,\"think_p99_us\":%.3f,\"think_max_us\":%.3f,\"space_read_mean_us\":%.4f}\n",
		Worlds[0].NumSkaters, Worlds.size(), Sum / Us.size(), Sorted[Sorted.size() / 2], Sorted[Sorted.size() * 99 / 100], Sorted.back(), SpaceSum / Worlds.size());
	return 0;
}
