// Native tournament driver (lane A2): the same engine-free sources as JamCore (AI/JamAICore, AI/JamBotMatch,
// AI/JamBotRefSim) compiled with clang++ so tournaments run in seconds without the editor. tools/tournament/run.mjs
// builds and drives it; the Unreal path is the commandlet -run=JamTournament with the same arguments and output.
//
//   jam_tournament --cfg "<k=v;...>" --cartridge <name> --seeds 1-20 --mode playerbot|playerbot_ai|brain|idle --max-frames 36000
// Prints one JSON line per match (thirdjam.tournament/1). Exit 0 ran, 2 bad arguments.
#include "JamBotMatch.h"
#include "JamBotRefSim.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

static void TraceToStdout(void*, const JamAI::FJamAIWorld& W, const JamAI::FJamAIInput* In, const JamAI::FJamBrain& Brain)
{
	static char Buf[4096];
	JamAI::FormatTrace(W, In, Brain, Buf, sizeof(Buf));
	std::puts(Buf);
}

int main(int Argc, char** Argv)
{
	const char* CfgText = "";
	const char* Cartridge = "hockey.regular";
	const char* ModeText = "playerbot";
	const char* Preset = "regular";
	int SeedLo = 1, SeedHi = 1;
	long long MaxFrames = 36000;
	int HashEvery = 600;
	int TraceEvery = 0;
	for (int i = 1; i + 1 < Argc; i += 2)
	{
		const char* K = Argv[i];
		const char* V = Argv[i + 1];
		if (std::strcmp(K, "--cfg") == 0) { CfgText = V; }
		else if (std::strcmp(K, "--cartridge") == 0) { Cartridge = V; }
		else if (std::strcmp(K, "--mode") == 0) { ModeText = V; }
		else if (std::strcmp(K, "--preset") == 0) { Preset = V; }
		else if (std::strcmp(K, "--max-frames") == 0) { MaxFrames = std::atoll(V); }
		else if (std::strcmp(K, "--hash-every") == 0) { HashEvery = std::atoi(V); }
		else if (std::strcmp(K, "--trace") == 0) { TraceEvery = std::atoi(V); }
		else if (std::strcmp(K, "--seeds") == 0)
		{
			const char* Dash = std::strchr(V, '-');
			SeedLo = std::atoi(V);
			SeedHi = Dash ? std::atoi(Dash + 1) : SeedLo;
		}
		else
		{
			std::fprintf(stderr, "unknown argument %s\n", K);
			return 2;
		}
	}
	JamAI::FJamMatchConfig Cfg;
	if (std::strcmp(Preset, "small") == 0) { Cfg.SetSmall(); } else { Cfg.SetRegular(); }
	char Err[512] = {};
	if (!Cfg.Parse(CfgText, Err, sizeof(Err)))
	{
		std::fprintf(stderr, "bad --cfg: %s\n", Err);
		return 2;
	}
	JamAI::EJamPlayerMode Mode;
	if (!JamAI::ParseMode(ModeText, Mode))
	{
		std::fprintf(stderr, "bad --mode %s\n", ModeText);
		return 2;
	}
	static char Line[8192];
	for (int Seed = SeedLo; Seed <= SeedHi; ++Seed)
	{
		JamAI::FRefSim Sim;
		const JamAI::FJamMatchStats St = JamAI::RunMatch(Sim, Cfg, static_cast<uint64_t>(Seed), Mode, MaxFrames, HashEvery, TraceEvery > 0 ? &TraceToStdout : nullptr, nullptr, TraceEvery);
		JamAI::FormatStatsJson(St, Cartridge, JamAI::ModeName(Mode), Line, sizeof(Line));
		std::printf("%s\n", Line);
		std::fflush(stdout);
	}
	return 0;
}
