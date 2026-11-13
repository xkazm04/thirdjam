#include "JamCommentaryData.h"
#include "CommentaryData.inl"
#include "CommentaryAbilityData.inl"   // lane A84 (R-0041 point 3): A's ability lines, merged below
#include "CommentaryRoughData.inl"     // lane A90 (R-0041 point 4): A89's rough-voice lines, merged below
#include "JamJson.h"

DEFINE_LOG_CATEGORY(LogJamCommentary);

namespace JamCommentary
{
	const FName TeamBlue(TEXT("blue"));
	const FName TeamOrange(TEXT("orange"));
	const FName TeamAny(TEXT("any"));
	const FName Schema(TEXT("hokej.commentary/1"));

	FName SituationOf(FName Id)
	{
		FString S = Id.ToString();
		int32 Dot = INDEX_NONE;
		if (S.FindLastChar(TEXT('.'), Dot) && Dot > 0)
		{
			bool bDigits = Dot + 1 < S.Len();
			for (int32 i = Dot + 1; i < S.Len(); ++i) bDigits &= FChar::IsDigit(S[i]);
			if (bDigits) S.LeftInline(Dot);
		}
		return FName(*S);
	}

	FName EventOf(FName Id)
	{
		FString S = Id.ToString();
		int32 Dot = INDEX_NONE;
		if (S.FindChar(TEXT('.'), Dot)) S.LeftInline(Dot);
		return FName(*S);
	}

	bool IsStoppageEvent(FName Event)
	{
		static const FName Period(TEXT("period")), PeriodEnd(TEXT("period_end")), SeasonTake(TEXT("season_take")), Win(TEXT("win")), Lose(TEXT("lose"));
		return Event == Period || Event == PeriodEnd || Event == SeasonTake || Event == Win || Event == Lose;
	}
}

bool FJamCommentaryBank::Parse(const FString& Json, TArray<FString>& OutErrors)
{
	Lines.Reset();
	ByEvent.Reset();
	ById.Reset();
	Situations.Reset();
	FString Err;
	TSharedPtr<FJamJson> Root = FJamJson::Parse(Json, Err);
	if (!Root) { OutErrors.Add(Err); return false; }
	if (!Root->IsObject()) { OutErrors.Add(TEXT("at /: the manifest must be an object")); return false; }
	static const TCHAR* Known[] = { TEXT("schema"), TEXT("voice"), TEXT("player"), TEXT("sound_class"), TEXT("count"), TEXT("rules"), TEXT("lines") };
	for (const TPair<FString, TSharedPtr<FJamJson>>& KV : Root->Object)
	{
		bool bKnown = false;
		for (const TCHAR* K : Known) bKnown |= KV.Key == K;
		if (!bKnown) OutErrors.Add(FString::Printf(TEXT("at /%s: unknown key"), *KV.Key));
	}
	TSharedPtr<FJamJson> Schema = Root->Get(TEXT("schema"));
	if (!Schema || !Schema->IsString() || Schema->String != JamCommentary::Schema.ToString()) OutErrors.Add(TEXT("at /schema: must be \"hokej.commentary/1\""));
	TSharedPtr<FJamJson> Arr = Root->Get(TEXT("lines"));
	if (!Arr || !Arr->IsArray()) { OutErrors.Add(TEXT("at /lines: must be an array")); return false; }
	if (TSharedPtr<FJamJson> Count = Root->Get(TEXT("count")); Count && Count->IsNumber() && static_cast<int32>(Count->Number) != Arr->Array.Num())
	{
		OutErrors.Add(FString::Printf(TEXT("at /count: %d but there are %d lines"), static_cast<int32>(Count->Number), Arr->Array.Num()));
	}
	static const FName GoalPause(TEXT("goal.pause"));
	for (int32 i = 0; i < Arr->Array.Num(); ++i)
	{
		const FString Ptr = FString::Printf(TEXT("/lines/%d"), i);
		const TSharedPtr<FJamJson>& L = Arr->Array[i];
		if (!L || !L->IsObject()) { OutErrors.Add(FString::Printf(TEXT("at %s: must be an object"), *Ptr)); continue; }
		FJamCommentaryLine Line;
		bool bOk = true;
		bool bHas[10] = { false };
		static const TCHAR* Keys[10] = { TEXT("id"), TEXT("event"), TEXT("kind"), TEXT("team"), TEXT("priority"), TEXT("cooldown_s"), TEXT("when"), TEXT("text"), TEXT("sound"), TEXT("seconds") };
		for (const TPair<FString, TSharedPtr<FJamJson>>& KV : L->Object)
		{
			const TSharedPtr<FJamJson>& V = KV.Value;
			const bool bStr = V.IsValid() && V->IsString();
			const bool bNum = V.IsValid() && V->IsNumber();
			int32 K = INDEX_NONE;
			for (int32 k = 0; k < 10; ++k) if (KV.Key == Keys[k]) K = k;
			if (K == INDEX_NONE) { OutErrors.Add(FString::Printf(TEXT("at %s/%s: unknown key"), *Ptr, *KV.Key)); bOk = false; continue; }
			bHas[K] = true;
			switch (K)
			{
			case 0: if (bStr) Line.Id = FName(*V->String); else bOk = false; break;
			case 1: if (bStr) Line.Event = FName(*V->String); else bOk = false; break;
			case 2: if (bStr) Line.Kind = V->String.IsEmpty() ? NAME_None : FName(*V->String); else bOk = false; break;
			case 3: if (bStr) Line.Team = FName(*V->String); else bOk = false; break;
			case 4: if (V.IsValid() && V->IsInteger() && V->Number >= 1 && V->Number <= 5) Line.Priority = static_cast<int32>(V->Number); else bOk = false; break;
			case 5: if (bNum && V->Number >= 0) Line.CooldownFrames = FMath::RoundToInt32(V->Number * 60.0); else bOk = false; break;
			case 6: if (bStr) Line.When = V->String; else bOk = false; break;
			case 7: if (bStr) Line.Text = V->String; else bOk = false; break;
			case 8: if (bStr) Line.Sound = FSoftObjectPath(V->String); else bOk = false; break;
			case 9: if (bNum && V->Number > 0) Line.DurationFrames = FMath::Max(1, FMath::CeilToInt32(V->Number * 60.0)); else bOk = false; break;
			default: break;
			}
			if (!bOk) { OutErrors.Add(FString::Printf(TEXT("at %s/%s: wrong type or out of range"), *Ptr, *KV.Key)); break; }
		}
		for (int32 k = 0; bOk && k < 10; ++k) if (!bHas[k]) { OutErrors.Add(FString::Printf(TEXT("at %s: missing key \"%s\""), *Ptr, Keys[k])); bOk = false; }
		if (!bOk) continue;
		if (Line.Team != JamCommentary::TeamBlue && Line.Team != JamCommentary::TeamOrange && Line.Team != JamCommentary::TeamAny)
		{
			OutErrors.Add(FString::Printf(TEXT("at %s/team: must be blue, orange or any"), *Ptr)); continue;
		}
		if (JamCommentary::EventOf(Line.Id) != Line.Event) { OutErrors.Add(FString::Printf(TEXT("at %s/id: first segment is not the event"), *Ptr)); continue; }
		if (ById.Contains(Line.Id)) { OutErrors.Add(FString::Printf(TEXT("at %s/id: duplicate id %s"), *Ptr, *Line.Id.ToString())); continue; }
		Line.Situation = JamCommentary::SituationOf(Line.Id);
		Line.bStoppage = JamCommentary::IsStoppageEvent(Line.Event) || Line.Situation == GoalPause;
		ById.Add(Line.Id, Lines.Num());
		ByEvent.FindOrAdd(Line.Event).Add(Lines.Num());
		Situations.Add(Line.Situation);
		Lines.Add(MoveTemp(Line));
	}
	return OutErrors.Num() == 0;
}

const TArray<int32>& FJamCommentaryBank::ForEvent(FName Event) const
{
	static const TArray<int32> Empty;
	const TArray<int32>* Found = ByEvent.Find(Event);
	return Found ? *Found : Empty;
}

int32 FJamCommentaryBank::Find(FName Id) const
{
	const int32* Found = ById.Find(Id);
	return Found ? *Found : INDEX_NONE;
}

FString FJamCommentaryBank::AbilityJson()
{
	return FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(JamCommentaryAbilityJson)));
}

FString FJamCommentaryBank::RoughJson()
{
	return FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(JamCommentaryRoughJson)));
}

FString FJamCommentaryBank::EmbeddedJson()
{
	return FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(JamCommentaryBankJson)));
}

const FJamCommentaryBank& FJamCommentaryBank::Embedded()
{
	static FJamCommentaryBank Bank = []
	{
		FJamCommentaryBank B;
		TArray<FString> Errors;
		if (!B.Parse(EmbeddedJson(), Errors))
		{
			for (const FString& E : Errors) UE_LOG(LogJamCommentary, Error, TEXT("JAMVO bank %s"), *E);
		}
		// Lane A84 (R-0041 point 3): the ability supplement, appended (its ids never collide with B's; a collision is skipped and logged).
		// Lane A90 (R-0041 point 4): A89's rough-voice supplement the same way (knockdown.human.04..13 extend B's situation).
		auto Merge = [&B](const FString& Json, const TCHAR* Name, TArray<FString>& SupErrors) -> int32
		{
			FJamCommentaryBank Sup;
			if (!Sup.Parse(Json, SupErrors)) for (const FString& E : SupErrors) UE_LOG(LogJamCommentary, Error, TEXT("JAMVO %s bank %s"), Name, *E);
			for (const FJamCommentaryLine& L : Sup.Lines)
			{
				if (B.ById.Contains(L.Id)) { UE_LOG(LogJamCommentary, Error, TEXT("JAMVO %s bank: %s is already in the manifest, skipped"), Name, *L.Id.ToString()); continue; }
				B.ById.Add(L.Id, B.Lines.Num());
				B.ByEvent.FindOrAdd(L.Event).Add(B.Lines.Num());
				B.Situations.Add(L.Situation);
				B.Lines.Add(L);
			}
			return Sup.Lines.Num();
		};
		TArray<FString> SupErrors;
		const int32 NAbility = Merge(AbilityJson(), TEXT("ability"), SupErrors);
		const int32 NRough = Merge(RoughJson(), TEXT("rough"), SupErrors);
		UE_LOG(LogJamCommentary, Log, TEXT("JAMVO bank loaded: %d lines (%d ability, %d rough), %d situations, %d errors"), B.Lines.Num(), NAbility, NRough, B.SituationCount(), Errors.Num() + SupErrors.Num());
		return B;
	}();
	return Bank;
}
