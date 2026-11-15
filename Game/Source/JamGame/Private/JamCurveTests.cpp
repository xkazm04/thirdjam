// Lane A217 (track C0, the curve): Jam.Curve.PersonPath pins the ladder's instrument, the person's path (-run=JamHandCells: the game mode's match
// with jam.ScriptHuman 2 through the button resolver), on one seed of each of the six cells (rookie / pro / all_star x regular / large), 1800 frames:
// the end state hash of each against tools/perf/results/a217/curve-pins.txt (one "level/cartridge hash" a line, written from the commandlet's own run when the ladder is set), and
// the pro cell twice (the same hash: the instrument is deterministic). A change to the sim, the brain, a level or the hand moves a pin: re-pin it with
// the command in CURVE-13.md section 4 and say so in the landing.
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "JamHandCellsCommandlet.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCurvePersonPath, "Jam.Curve.PersonPath", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FJamCurvePersonPath::RunTest(const FString& Parameters)
{
	const FString PinPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("../tools/perf/results/a217/curve-pins.txt"));
	FString Text;
	if (!FFileHelper::LoadFileToString(Text, *PinPath)) { AddError(FString::Printf(TEXT("no pins at %s"), *PinPath)); return false; }
	TMap<FString, FString> Pins;
	TArray<FString> Lines;
	Text.ParseIntoArrayLines(Lines);
	for (const FString& L : Lines) { FString K, V; if (L.Split(TEXT(" "), &K, &V)) Pins.Add(K.TrimStartAndEnd(), V.TrimStartAndEnd()); }
	auto Play = [](const FString& Cart, const FString& Level) -> uint64
	{
		UJamHandCellsCommandlet* C = NewObject<UJamHandCellsCommandlet>();
		C->Main(FString::Printf(TEXT("-cartridge=%s -difficulty=%s -seeds=1-1 -maxframes=1800"), *Cart, *Level));
		return JamHandCells::LastHash();
	};
	const TCHAR* Levels[3] = { TEXT("rookie"), TEXT("pro"), TEXT("all_star") };
	const TCHAR* Carts[2] = { TEXT("hockey.regular"), TEXT("hockey.arena.large") };
	for (const TCHAR* L : Levels)
	{
		for (const TCHAR* Ct : Carts)
		{
			const FString Key = FString::Printf(TEXT("%s/%s"), L, Ct);
			const FString* WantP = Pins.Find(Key);
			if (!WantP) { AddError(FString::Printf(TEXT("no pin for %s"), *Key)); continue; }
			const FString Want = *WantP;
			const FString Got = FString::Printf(TEXT("%016llx"), static_cast<unsigned long long>(Play(Ct, L)));
			TestEqual(FString::Printf(TEXT("%s seed 1, 1800 frames, the person's path"), *Key), Got, Want);
		}
	}
	const uint64 A = Play(TEXT("hockey.regular"), TEXT("pro")), B = Play(TEXT("hockey.regular"), TEXT("pro"));
	TestEqual(TEXT("pro regular twice: the same match"), A, B);
	return true;
}
#endif
