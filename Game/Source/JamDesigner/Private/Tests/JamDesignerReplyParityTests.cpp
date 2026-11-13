// Lane A141 (track S3, evolution 3): Jam.Designer.ReplyParity. Mutated Designer replies (tools/fuzz/gen-replies.mjs: duplicate keys, a byte order mark, truncated,
// a number outside its range, an unknown key, prose or a fence around the JSON, refusals good and bad, locked keys) run through FJamDesignerPrompt::ParseReply;
// Node's parseDesignerReply made the manifest. Both sides must agree on the kind (cartridge | refusal | invalid) and, for an invalid reply, on the sorted set of
// JSON pointers of the errors. A disagreement is a fault line "REPLYPARITY <case>". The gate runs the checked-in tools/fuzz/replies (56 cases);
// JAM_FUZZ_REPLIES names a bigger corpus (tools/fuzz/out/replies, 1,000 cases).
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformMisc.h"
#include "JamDesignerPrompt.h"
#include "JamCartridge.h"
#include "JamJson.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamReplyParity
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;

	FString Pointers(const TArray<FString>& Errors)
	{
		TArray<FString> Got;
		for (const FString& E : Errors)
		{
			FString P = E.StartsWith(TEXT("at ")) ? E.Mid(3) : TEXT("?");
			int32 Colon;
			if (P.FindChar(':', Colon)) { P = P.Left(Colon); }
			Got.AddUnique(P);
		}
		Got.Sort();
		return FString::Join(Got, TEXT(" "));
	}

	const TCHAR* KindName(EJamDesignerReplyKind K) { return K == EJamDesignerReplyKind::Cartridge ? TEXT("cartridge") : K == EJamDesignerReplyKind::Refusal ? TEXT("refusal") : TEXT("invalid"); }

	bool RunCorpus(FAutomationTestBase& T, const FString& Dir)
	{
		FString Text;
		if (!FFileHelper::LoadFileToString(Text, *FPaths::Combine(Dir, TEXT("manifest.json")))) { T.AddWarning(FString::Printf(TEXT("reply corpus %s not found; skipped"), *Dir)); return true; }
		FString Err;
		TSharedPtr<FJamJson> Root = FJamJson::Parse(Text, Err);
		if (!Root) { T.AddError(FString::Printf(TEXT("reply manifest: %s"), *Err)); return false; }
		FString BaseText;
		if (!FFileHelper::LoadFileToString(BaseText, *FPaths::Combine(UJamCartridge::GetCartridgeDir(), TEXT("hockey.regular.cartridge.json")))) { T.AddError(TEXT("hockey.regular unreadable")); return false; }
		int32 Faults = 0, Cases = 0;
		for (const TSharedPtr<FJamJson>& C : Root->Get(TEXT("cases"))->Array)
		{
			++Cases;
			const FString Name = C->Get(TEXT("name"))->String;
			const FString Reply = C->Get(TEXT("text"))->String;
			const FString WantKind = C->Get(TEXT("kind"))->String;
			TArray<FString> P;
			for (const TSharedPtr<FJamJson>& X : C->Get(TEXT("pointers"))->Array) { P.Add(X->String); }
			P.Sort();
			const FString WantPtrs = FString::Join(P, TEXT(" "));
			const FJamDesignerParsedReply R = FJamDesignerPrompt::ParseReply(Reply, BaseText);
			const FString GotKind = KindName(R.Kind);
			const FString GotPtrs = Pointers(R.Errors);
			if (GotKind != WantKind || GotPtrs != WantPtrs)
			{
				++Faults;
				T.AddError(FString::Printf(TEXT("REPLYPARITY %s: node %s [%s], c++ %s [%s] first: %s"), *Name, *WantKind, *WantPtrs, *GotKind, *GotPtrs, R.Errors.Num() ? *R.Errors[0].Left(200) : TEXT("-")));
			}
		}
		T.AddInfo(FString::Printf(TEXT("reply parity: %d cases, %d faults"), Cases, Faults));
		return Faults == 0 && Cases > 0;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamDesignerReplyParity, "Jam.Designer.ReplyParity", JamReplyParity::Flags)
bool FJamDesignerReplyParity::RunTest(const FString& Parameters)
{
	const FString Dir = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), TEXT(".."), TEXT("tools"), TEXT("fuzz"), TEXT("replies")));
	if (!IFileManager::Get().DirectoryExists(*Dir)) { AddWarning(TEXT("tools/fuzz/replies not found (source tree absent); skipped")); return true; }
	bool bOk = JamReplyParity::RunCorpus(*this, Dir);
	const FString Big = FPlatformMisc::GetEnvironmentVariable(TEXT("JAM_FUZZ_REPLIES"));
	if (!Big.IsEmpty()) { bOk = JamReplyParity::RunCorpus(*this, Big) && bOk; }
	return bOk;
}

#endif
