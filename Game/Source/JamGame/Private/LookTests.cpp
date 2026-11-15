// Lane A223 (track L1; B-to-A-0046): the numbers of the three looks the trailer shows, and the neutral value that gives back each picture of before.
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "HAL/IConsoleManager.h"
#include "LookTierSwitcher.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamLookTests
{
	const EAutomationTestFlags Flags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;

	struct FVarGuard
	{
		IConsoleVariable* Var = nullptr;
		FString Was;
		explicit FVarGuard(const TCHAR* Name) : Var(IConsoleManager::Get().FindConsoleVariable(Name)) { if (Var) Was = Var->GetString(); }
		~FVarGuard() { if (Var) Var->Set(*Was, ECVF_SetByCode); }
		void Set(float V) const { if (Var) Var->Set(V, ECVF_SetByCode); }
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamLookTrimDay, "Jam.Look.TrimDay", JamLookTests::Flags)
bool FJamLookTrimDay::RunTest(const FString&)
{
	JamLookTests::FVarGuard Day(TEXT("jam.look.TrimDay"));
	if (!TestNotNull(TEXT("jam.look.TrimDay exists"), Day.Var)) return false;
	Day.Set(10.f);
	TestEqual(TEXT("the day tier's trim is painted, not lit: Emissive 10"), ULookTierSwitcher::DefaultTrimEmissiveFor(TEXT("l1_day")), 10.f);
	// the other tiers are as they were
	TestEqual(TEXT("night 30"), ULookTierSwitcher::DefaultTrimEmissiveFor(TEXT("l2_night")), 30.f);
	TestEqual(TEXT("neon 30"), ULookTierSwitcher::DefaultTrimEmissiveFor(TEXT("l3_neon")), 30.f);
	TestEqual(TEXT("clay 12"), ULookTierSwitcher::DefaultTrimEmissiveFor(TEXT("p3_clay")), 12.f);
	TestEqual(TEXT("broadcast 30"), ULookTierSwitcher::DefaultTrimEmissiveFor(TEXT("p4_broadcast")), 30.f);
	// the neutral value: the day tier of before the lane
	Day.Set(30.f);
	TestEqual(TEXT("jam.look.TrimDay 30 gives back the day tier of before"), ULookTierSwitcher::DefaultTrimEmissiveFor(TEXT("l1_day")), 30.f);
	TestEqual(TEXT("and moves no other tier"), ULookTierSwitcher::DefaultTrimEmissiveFor(TEXT("p3_clay")), 12.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamLookFogKey, "Jam.Look.FogKey", JamLookTests::Flags)
bool FJamLookFogKey::RunTest(const FString&)
{
	JamLookTests::FVarGuard Scale(TEXT("jam.look.FogScale"));
	JamLookTests::FVarGuard From(TEXT("jam.look.FogFrom"));
	if (!TestNotNull(TEXT("jam.look.FogScale exists"), Scale.Var) || !TestNotNull(TEXT("jam.look.FogFrom exists"), From.Var)) return false;
	Scale.Set(8.f); From.Set(0.03f);
	TestEqual(TEXT("Fog of War asks 0.04: a height fog of density 0.32"), ULookTierSwitcher::FogDensityFor(0.04f), 0.32f, 1e-4f);
	TestEqual(TEXT("a cut that asks none draws none"), ULookTierSwitcher::FogDensityFor(0.f), 0.f);
	TestEqual(TEXT("a cut that asks 0.02 (hockey.boost, the ladder's neon takes) keeps the picture it had"), ULookTierSwitcher::FogDensityFor(0.02f), 0.f);
	TestEqual(TEXT("the schema's top value stays inside the engine's clamp"), ULookTierSwitcher::FogDensityFor(0.1f), 0.8f, 1e-4f);
	// the neutral value: no fog anywhere, the game of before the lane
	Scale.Set(0.f);
	TestEqual(TEXT("jam.look.FogScale 0 draws no fog for Fog of War"), ULookTierSwitcher::FogDensityFor(0.04f), 0.f);
	TestEqual(TEXT("nor for the schema's top value"), ULookTierSwitcher::FogDensityFor(0.1f), 0.f);
	return true;
}

#endif
