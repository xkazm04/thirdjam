#include "JamDesignerPrompt.h"
#include "JamJson.h"
#include "JamCartridge.h"

#include "JamDesignerData.inl"

const TCHAR* JamDesignerModels::First = TEXT("claude-haiku-5-5");
const TCHAR* JamDesignerModels::Repair = TEXT("claude-haiku-5-5");
const TCHAR* JamDesignerModels::Retry = TEXT("claude-sonnet-5-5");

namespace JamDesignerPromptImpl
{
	using FJ = FJamJson;
	using FJPtr = TSharedPtr<FJamJson>;

	static FJPtr MakeStr(const FString& S) { FJPtr N = MakeShared<FJ>(); N->Type = FJ::EType::String; N->String = S; return N; }
	static FJPtr MakeNum(double V) { FJPtr N = MakeShared<FJ>(); N->Type = FJ::EType::Number; N->Number = V; return N; }
	static FJPtr MakeObj() { FJPtr N = MakeShared<FJ>(); N->Type = FJ::EType::Object; return N; }
	static FJPtr MakeArr() { FJPtr N = MakeShared<FJ>(); N->Type = FJ::EType::Array; return N; }
	static void Put(const FJPtr& O, const FString& K, const FJPtr& V) { O->Object.Add(TPair<FString, FJPtr>(K, V)); }

	static FJPtr Clone(const FJ& V)
	{
		FJPtr N = MakeShared<FJ>();
		N->Type = V.Type; N->Bool = V.Bool; N->Number = V.Number; N->String = V.String;
		for (const FJPtr& A : V.Array) N->Array.Add(A ? Clone(*A) : FJPtr());
		for (const TPair<FString, FJPtr>& P : V.Object) N->Object.Add(TPair<FString, FJPtr>(P.Key, P.Value ? Clone(*P.Value) : FJPtr()));
		return N;
	}

	static FJPtr ParseEmbedded(const FString& Name)
	{
		FString Err;
		return FJ::Parse(FJamDesignerPrompt::GetEmbeddedFile(Name), Err);
	}

	// The template filler of assemble.mjs: /\{\{([A-Z_]+)\}\}/g with a function replacer (no "$" specials).
	static bool Fill(const FString& Template, const TMap<FString, FString>& Vars, FString& Out, FString& OutError)
	{
		Out.Reset(Template.Len() + 1024);
		int32 i = 0;
		const int32 N = Template.Len();
		while (i < N)
		{
			if (i + 1 < N && Template[i] == '{' && Template[i + 1] == '{')
			{
				int32 j = i + 2;
				while (j < N && ((Template[j] >= 'A' && Template[j] <= 'Z') || Template[j] == '_')) ++j;
				if (j > i + 2 && j + 1 < N && Template[j] == '}' && Template[j + 1] == '}')
				{
					const FString Name = Template.Mid(i + 2, j - i - 2);
					const FString* V = Vars.Find(Name);
					if (!V) { OutError = FString::Printf(TEXT("prompt placeholder {{%s}} has no value"), *Name); return false; }
					Out += *V;
					i = j + 2;
					continue;
				}
			}
			Out.AppendChar(Template[i]);
			++i;
		}
		return true;
	}

	static FString EscapeText(const FString& S) { return S.Replace(TEXT("<"), TEXT("&lt;")).Replace(TEXT(">"), TEXT("&gt;")); }
	static FString EscapeJson(const FString& S) { return S.Replace(TEXT("<"), TEXT("\\u003c")).Replace(TEXT(">"), TEXT("\\u003e")); }

	// JavaScript String.prototype.trim for the whitespace a sentence can carry.
	static FString JsTrim(const FString& S)
	{
		auto IsWs = [](TCHAR C) { return C == ' ' || C == '\t' || C == '\n' || C == '\r' || C == '\v' || C == '\f' || C == 0xA0 || C == 0xFEFF || C == 0x2028 || C == 0x2029 || (C >= 0x2000 && C <= 0x200A) || C == 0x3000 || C == 0x1680 || C == 0x202F || C == 0x205F; };
		int32 A = 0, B = S.Len();
		while (A < B && IsWs(S[A])) ++A;
		while (B > A && IsWs(S[B - 1])) --B;
		return S.Mid(A, B - A);
	}

	static const TCHAR* Immutable[] = { TEXT("schema"), TEXT("id"), TEXT("title"), TEXT("seed"), TEXT("tunables") };
	// Editable lists come from schema/validate.mjs through JamDesignerData.inl (JamDesignerGameEditable, JamDesignerEditableList).

	static FJPtr WithoutPaths(const FJ& Game)
	{
		FJPtr Copy = Clone(Game);
		for (const TCHAR* Path : JamDesignerGameEditable)
		{
			TArray<FString> Parts;
			FString(Path).ParseIntoArray(Parts, TEXT("."));
			FJ* Node = Copy.Get();
			for (int32 k = 0; k + 1 < Parts.Num() && Node; ++k)
			{
				FJPtr Next = Node->IsObject() ? Node->Get(Parts[k]) : FJPtr();
				Node = Next.Get();
			}
			if (Node && Node->IsObject())
			{
				Node->Object.RemoveAll([&](const TPair<FString, FJPtr>& P) { return P.Key == Parts.Last(); });
			}
			// validate.mjs (lane A3): an object emptied by the removal counts as absent (adding `ai` around `difficulty` is no change).
			if (Parts.Num() == 2)
			{
				FJPtr Top = Copy->Get(Parts[0]);
				if (Top && Top->IsObject() && Top->Object.Num() == 0) Copy->Object.RemoveAll([&](const TPair<FString, FJPtr>& P) { return P.Key == Parts[0]; });
			}
		}
		return Copy;
	}

	// Lane A44: the game schema excerpt the Designer sees, cut down to the parts of `game` it may change (JamDesignerGameEditable), so the
	// system prompt stays under 70 KB as keys are added. Mirror of trimGameSchema in tools/designer-eval/assemble.mjs, key order kept
	// (Jam.Designer.PromptParity): a top-level key named whole stays whole; one named through sub-keys keeps those and loses the rest, its
	// additionalProperties and the required entries that went; a key never named goes.
	static FJPtr TrimGameSchema(const FJ& Schema)
	{
		FJPtr Out = Clone(Schema);
		FJPtr Props = Out->Get(TEXT("properties"));
		if (!Props || !Props->IsObject()) return Out;
		TArray<FString> Keys;
		for (const TPair<FString, FJPtr>& P : Props->Object) Keys.Add(P.Key);
		for (const FString& Key : Keys)
		{
			TArray<FString> Mine;
			for (const TCHAR* P : JamDesignerGameEditable) { const FString S(P); if (S == Key || S.StartsWith(Key + TEXT("."))) Mine.Add(S); }
			if (Mine.Num() == 0) { Props->Object.RemoveAll([&](const TPair<FString, FJPtr>& P) { return P.Key == Key; }); continue; }
			if (Mine.Contains(Key)) continue;
			FJPtr Node = Props->Get(Key);
			FJPtr Sub = Node && Node->IsObject() ? Node->Get(TEXT("properties")) : FJPtr();
			if (!Sub || !Sub->IsObject()) continue;
			TSet<FString> Keep;
			for (const FString& M : Mine) { TArray<FString> Parts; M.ParseIntoArray(Parts, TEXT(".")); if (Parts.Num() > 1) Keep.Add(Parts[1]); }
			Sub->Object.RemoveAll([&](const TPair<FString, FJPtr>& P) { return !Keep.Contains(P.Key); });
			Node->Object.RemoveAll([](const TPair<FString, FJPtr>& P) { return P.Key == TEXT("additionalProperties"); });
			if (FJPtr Req = Node->Get(TEXT("required")); Req && Req->IsArray())
			{
				Req->Array.RemoveAll([&](const FJPtr& R) { return !(R && R->IsString() && Keep.Contains(R->String)); });
				if (Req->Array.Num() == 0) Node->Object.RemoveAll([](const TPair<FString, FJPtr>& P) { return P.Key == TEXT("required"); });
			}
		}
		Out->Object.RemoveAll([](const TPair<FString, FJPtr>& P) { return P.Key == TEXT("additionalProperties"); });
		if (FJPtr Req = Out->Get(TEXT("required")); Req && Req->IsArray())
		{
			Req->Array.RemoveAll([&](const FJPtr& R) { return !(R && R->IsString() && Props->Has(R->String)); });
			if (Req->Array.Num() == 0) Out->Object.RemoveAll([](const TPair<FString, FJPtr>& P) { return P.Key == TEXT("required"); });
		}
		return Out;
	}

	static void FindDiffs(const FJ* X, const FJ* Y, const FString& Ptr, TArray<FString>& Out)
	{
		const bool bObjs = X && Y && X->IsObject() && Y->IsObject();
		const bool bArrs = X && Y && X->IsArray() && Y->IsArray() && X->Array.Num() == Y->Array.Num();
		if (bObjs)
		{
			TArray<FString> Keys;
			for (const auto& P : X->Object) Keys.AddUnique(P.Key);
			for (const auto& P : Y->Object) Keys.AddUnique(P.Key);
			for (const FString& K : Keys) FindDiffs(X->Get(K).Get(), Y->Get(K).Get(), FJ::PointerChild(Ptr, K), Out);
		}
		else if (bArrs)
		{
			for (int32 i = 0; i < X->Array.Num(); ++i) FindDiffs(X->Array[i].Get(), Y->Array[i].Get(), FJ::PointerChild(Ptr, i), Out);
		}
		else if (!FJamDesignerPrompt::DeepEqual(X, Y))
		{
			Out.Add(Ptr);
		}
	}
}

using namespace JamDesignerPromptImpl;

FString FJamDesignerPrompt::GetEmbeddedFile(const FString& Name)
{
	for (const FJamDesignerEmbeddedFile& F : JamDesignerEmbeddedFiles)
	{
		if (Name == F.Name) return FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(F.Data)));
	}
	return FString();
}

FString FJamDesignerPrompt::Pretty(const FJamJson& V, const FString& Indent)
{
	const FString Inner = Indent + TEXT("  ");
	if (V.IsObject())
	{
		if (V.Object.Num() == 0) return TEXT("{}");
		FString Out = TEXT("{\n");
		for (int32 i = 0; i < V.Object.Num(); ++i)
		{
			if (i) Out += TEXT(",\n");
			Out += Inner + MakeStr(V.Object[i].Key)->ToCompact() + TEXT(": ") + Pretty(*V.Object[i].Value, Inner);
		}
		return Out + TEXT("\n") + Indent + TEXT("}");
	}
	if (V.IsArray())
	{
		if (V.Array.Num() == 0) return TEXT("[]");
		FString Out = TEXT("[\n");
		for (int32 i = 0; i < V.Array.Num(); ++i)
		{
			if (i) Out += TEXT(",\n");
			Out += Inner + Pretty(*V.Array[i], Inner);
		}
		return Out + TEXT("\n") + Indent + TEXT("]");
	}
	return V.ToCompact();
}

bool FJamDesignerPrompt::DeepEqual(const FJamJson* A, const FJamJson* B)
{
	if (!A || !B) return A == B;
	if (A->Type != B->Type) return false;
	switch (A->Type)
	{
	case FJamJson::EType::Null: return true;
	case FJamJson::EType::Bool: return A->Bool == B->Bool;
	case FJamJson::EType::Number: return A->Number == B->Number;
	case FJamJson::EType::String: return A->String == B->String;
	case FJamJson::EType::Array:
		if (A->Array.Num() != B->Array.Num()) return false;
		for (int32 i = 0; i < A->Array.Num(); ++i) if (!DeepEqual(A->Array[i].Get(), B->Array[i].Get())) return false;
		return true;
	case FJamJson::EType::Object:
		if (A->Object.Num() != B->Object.Num()) return false;
		for (const auto& P : A->Object) if (!DeepEqual(P.Value.Get(), B->Get(P.Key).Get())) return false;
		return true;
	}
	return false;
}

uint64 FJamDesignerPrompt::Fnv1a64(const FString& Text)
{
	FTCHARToUTF8 Utf8(*Text, Text.Len());
	uint64 H = 0xcbf29ce484222325ull;
	const uint8* P = reinterpret_cast<const uint8*>(Utf8.Get());
	for (int32 i = 0; i < Utf8.Length(); ++i) { H ^= P[i]; H *= 0x100000001b3ull; }
	return H;
}

int32 FJamDesignerPrompt::GetLimit(const FString& Key, int32 Fallback)
{
	static FJPtr Limits = ParseEmbedded(TEXT("limits.json"));
	if (!Limits) return Fallback;
	FJPtr V = Limits->Get(Key);
	return V && V->IsNumber() ? static_cast<int32>(V->Number) : Fallback;
}

const FString& FJamDesignerPrompt::GetSystemText()
{
	static FString Text = []
	{
		FJPtr Limits = ParseEmbedded(TEXT("limits.json"));
		FJPtr Envelope = ParseEmbedded(TEXT("cartridge.schema.json"));
		FJPtr Game = ParseEmbedded(TEXT("game.schema.json"));
		if (!Limits || !Envelope || !Game) return FString();
		// blocked_terms stay out of the prompt (a program-side net, not instructions to the model).
		Limits->Object.RemoveAll([](const TPair<FString, FJPtr>& P) { return P.Key == TEXT("blocked_terms"); });
		TMap<FString, FString> Vars;
		Vars.Add(TEXT("LIMITS"), Pretty(*Limits));
		Vars.Add(TEXT("ENVELOPE_SCHEMA"), Pretty(*Envelope));
		Vars.Add(TEXT("GAME_SCHEMA"), Pretty(*TrimGameSchema(*Game)));
		FString Out, Err;
		if (!Fill(GetEmbeddedFile(TEXT("designer.system.md")), Vars, Out, Err)) return FString();
		return Out;
	}();
	return Text;
}

bool FJamDesignerPrompt::AssembleBody(const FString& Sentence, const FString& CartridgeJson, const FString& Model, const FString* PreviousOutput,
	const TArray<FString>& PreviousErrors, FString& OutBody, FString& OutError)
{
	const FString Trimmed = JsTrim(Sentence);
	if (Trimmed.IsEmpty()) { OutError = TEXT("the player sentence is empty"); return false; }
	const int32 MaxChars = GetLimit(TEXT("max_sentence_chars"), 300);
	if (Sentence.Len() > MaxChars) { OutError = FString::Printf(TEXT("the player sentence is %d characters; the limit is %d"), Sentence.Len(), MaxChars); return false; }
	const FString& System = GetSystemText();
	if (System.IsEmpty()) { OutError = TEXT("the embedded system prompt could not be built"); return false; }

	FString ParseErr;
	FJPtr Cart = FJamJson::Parse(CartridgeJson, ParseErr);
	if (!Cart) { OutError = FString::Printf(TEXT("the current cartridge is not JSON: %s"), *ParseErr); return false; }

	const FString SafeSentence = EscapeText(Trimmed);
	const FString SafeCartridge = EscapeJson(Cart->ToCompact());
	FString UserText;
	if (PreviousOutput)
	{
		TArray<FString> Lines;
		for (const FString& E : PreviousErrors) Lines.Add(TEXT("- ") + E);
		TMap<FString, FString> Vars;
		Vars.Add(TEXT("ERRORS"), FString::Join(Lines, TEXT("\n")));
		Vars.Add(TEXT("SENTENCE"), SafeSentence);
		Vars.Add(TEXT("CARTRIDGE"), SafeCartridge);
		Vars.Add(TEXT("PREVIOUS"), EscapeText(*PreviousOutput));
		if (!Fill(GetEmbeddedFile(TEXT("designer.repair.md")), Vars, UserText, OutError)) return false;
	}
	else
	{
		UserText = FString::Printf(TEXT("<player_sentence>\n%s\n</player_sentence>\n\n<current_cartridge>\n%s\n</current_cartridge>"), *SafeSentence, *SafeCartridge);
	}

	FJPtr Body = MakeObj();
	Put(Body, TEXT("model"), MakeStr(Model));
	Put(Body, TEXT("max_tokens"), MakeNum(JamDesignerModels::MaxTokens));
	FJPtr SysArr = MakeArr();
	FJPtr SysBlock = MakeObj();
	Put(SysBlock, TEXT("type"), MakeStr(TEXT("text")));
	Put(SysBlock, TEXT("text"), MakeStr(System));
	FJPtr Cache = MakeObj();
	Put(Cache, TEXT("type"), MakeStr(TEXT("ephemeral")));
	Put(SysBlock, TEXT("cache_control"), Cache);
	SysArr->Array.Add(SysBlock);
	Put(Body, TEXT("system"), SysArr);
	FJPtr Msgs = MakeArr();
	FJPtr Msg = MakeObj();
	Put(Msg, TEXT("role"), MakeStr(TEXT("user")));
	Put(Msg, TEXT("content"), MakeStr(UserText));
	Msgs->Array.Add(Msg);
	Put(Body, TEXT("messages"), Msgs);
	FJPtr OutCfg = MakeObj();
	Put(OutCfg, TEXT("effort"), MakeStr(TEXT("low")));
	Put(Body, TEXT("output_config"), OutCfg);
	OutBody = Body->ToCompact();
	return true;
}

// C-0005 (lane A15): true when a hook with an end_match action is not guarded (after_frame >= 600, on_frame every_frames >= 600,
// on_period_end, or a score needing a second goal). Mirror of hookEndsMatchEarly in schema/validate.mjs.
bool FJamDesignerPrompt::HookEndsMatchEarly(const TSharedPtr<FJamJson>& H)
{
	if (!H || !H->IsObject()) return false;
	FJPtr Then = H->Get(TEXT("then"));
	if (!Then || !Then->IsArray()) return false;
	bool bEnds = false;
	for (const FJPtr& A : Then->Array)
	{
		FJPtr Do = A && A->IsObject() ? A->Get(TEXT("do")) : FJPtr();
		if (Do && Do->IsString() && Do->String == TEXT("end_match")) bEnds = true;
	}
	if (!bEnds) return false;
	auto NumOf = [&](const FJPtr& O, const TCHAR* K, double& V) { FJPtr X = O && O->IsObject() ? O->Get(K) : FJPtr(); if (X && X->IsNumber()) { V = X->Number; return true; } return false; };
	double V = 0;
	if (NumOf(H, TEXT("after_frame"), V) && V >= 600) return false;
	FJPtr When = H->Get(TEXT("when"));
	const FString W = When && When->IsString() ? When->String : FString();
	if (W == TEXT("on_frame") && NumOf(H, TEXT("every_frames"), V) && V >= 600) return false;
	if (W == TEXT("on_period_end")) return false;
	FJPtr Score = H->Get(TEXT("score"));
	if ((NumOf(Score, TEXT("at_least"), V) && V >= 2) || (NumOf(Score, TEXT("lead_at_least"), V) && V >= 2)) return false;
	return true;
}

void FJamDesignerPrompt::CheckDesignerEdits(const FJamJson& Base, const FJamJson& Out, TArray<FString>& OutErrors)
{
	if (!Base.IsObject() || !Out.IsObject()) return;
	for (const TCHAR* K : Immutable)
	{
		FJPtr A = Base.Get(K), B = Out.Get(K);
		if (A && B && !DeepEqual(A.Get(), B.Get()))
		{
			OutErrors.Add(FString::Printf(TEXT("at %s: the Designer may not change \"%s\". Restore it exactly as it was in the current cartridge. Only %s (and some of \"game\") may change."),
				*FJamJson::PointerChild(FString(), K), K, JamDesignerEditableList));
		}
	}
	FJPtr BG = Base.Get(TEXT("game")), OG = Out.Get(TEXT("game"));
	if (BG && OG && BG->IsObject() && OG->IsObject())
	{
		FJPtr A = WithoutPaths(*BG), B = WithoutPaths(*OG);
		if (!DeepEqual(A.Get(), B.Get()))
		{
			TArray<FString> Diffs;
			FindDiffs(A.Get(), B.Get(), TEXT("/game"), Diffs);
			TArray<FString> Paths;
			for (const TCHAR* P : JamDesignerGameEditable) Paths.Add(P);
			const FString List = FString::Join(Paths, TEXT(", "));
			for (const FString& Ptr : Diffs)
			{
				OutErrors.Add(FString::Printf(TEXT("at %s: the Designer may not change this part of \"game\". Restore it exactly as it was. In \"game\" only these may change: %s."), *Ptr, *List));
			}
		}
	}
	else if (BG && !DeepEqual(BG.Get(), OG.Get()))
	{
		OutErrors.Add(TEXT("at /game: the Designer may not change \"game\" as a whole. Restore it exactly as it was."));
	}
	// C-0005 (lane A15): the Designer may not create a hook that ends the match before frame 600 (validate.mjs hookEndsMatchEarly).
	if (OG && OG->IsObject())
	{
		FJPtr Rules = OG->Get(TEXT("rules"));
		FJPtr Hooks = Rules && Rules->IsObject() ? Rules->Get(TEXT("hooks")) : FJPtr();
		if (Hooks && Hooks->IsArray())
		{
			for (int32 i = 0; i < Hooks->Array.Num(); ++i)
			{
				if (HookEndsMatchEarly(Hooks->Array[i]))
				{
					OutErrors.Add(FString::Printf(TEXT("at /game/rules/hooks/%d: this hook could end the match before frame 600 (10 seconds). Add \"after_frame\": 600, make it wait for a second goal (score.at_least or lead_at_least of 2), or fire it on_period_end."), i));
				}
			}
		}
	}
}

FJamDesignerParsedReply FJamDesignerPrompt::ParseReply(const FString& Text, const FString& BaseCartridgeJson)
{
	FJamDesignerParsedReply R;
	const FString Trimmed = JsTrim(Text);
	if (Trimmed.IsEmpty())
	{
		R.Errors.Add(TEXT("at /: the reply is empty. Return a JSON object: the full cartridge, or {\"refusal\": \"<reason>\"}."));
		return R;
	}
	FString ParseErr;
	FJPtr Parsed = FJamJson::Parse(Trimmed, ParseErr);
	if (!Parsed)
	{
		const FString Head = Trimmed.Len() > 40 ? Trimmed.Left(40) + TEXT("...") : Trimmed;
		const bool bWrapped = !Trimmed.StartsWith(TEXT("{")) && Trimmed.Contains(TEXT("{"));
		R.Errors.Add(bWrapped
			? FString::Printf(TEXT("at /: the reply has text outside the JSON object (it starts with %s). Return only the JSON object: no sentence before or after it and no markdown fence."), *MakeStr(Head)->ToCompact())
			: FString::Printf(TEXT("at /: the reply is not valid JSON (%s). Return only one complete JSON object."), *ParseErr));
		return R;
	}
	if (!Parsed->IsObject())
	{
		R.Errors.Add(TEXT("at /: the reply must be a JSON object."));
		return R;
	}
	if (FJPtr Ref = Parsed->Get(TEXT("refusal")))
	{
		if (Parsed->Object.Num() != 1) R.Errors.Add(TEXT("at /: a refusal must be exactly {\"refusal\": \"<reason>\"} with no other keys."));
		const int32 MaxRefusal = GetLimit(TEXT("max_refusal_chars"), 300);
		if (!Ref->IsString() || JsTrim(Ref->String).IsEmpty()) R.Errors.Add(TEXT("at /refusal: the reason must be a non-empty string."));
		else if (Ref->String.Len() > MaxRefusal) R.Errors.Add(FString::Printf(TEXT("at /refusal: the reason is %d characters; the limit is %d."), Ref->String.Len(), MaxRefusal));
		if (R.Errors.Num() == 0) { R.Kind = EJamDesignerReplyKind::Refusal; R.Refusal = Ref->String; }
		return R;
	}

	TArray<FString> Errors;
	UJamCartridge::Validate(Trimmed, Errors);
	FString BaseErr;
	FJPtr Base = FJamJson::Parse(BaseCartridgeJson, BaseErr);
	if (Base) CheckDesignerEdits(*Base, *Parsed, Errors);
	TArray<FString> Unique;
	for (const FString& E : Errors) Unique.AddUnique(E);
	if (Unique.Num() == 0) { R.Kind = EJamDesignerReplyKind::Cartridge; R.CartridgeJson = Trimmed; }
	else R.Errors = MoveTemp(Unique);
	return R;
}

FJamDesignerApiReply FJamDesignerPrompt::ParseApiResponse(const FString& Body)
{
	FJamDesignerApiReply R;
	FString Err;
	FJPtr Root = FJamJson::Parse(Body, Err);
	if (!Root || !Root->IsObject()) return R;
	R.bParsed = true;
	if (FJPtr Content = Root->Get(TEXT("content")); Content && Content->IsArray())
	{
		for (const FJPtr& B : Content->Array)
		{
			if (!B || !B->IsObject()) continue;
			FJPtr T = B->Get(TEXT("type")), X = B->Get(TEXT("text"));
			if (T && T->IsString() && T->String == TEXT("text") && X && X->IsString()) R.Text += X->String;
		}
	}
	if (FJPtr S = Root->Get(TEXT("stop_reason")); S && S->IsString()) R.StopReason = S->String;
	if (FJPtr E = Root->Get(TEXT("error")); E && E->IsObject())
	{
		if (FJPtr M = E->Get(TEXT("message")); M && M->IsString()) R.ErrorMessage = M->String;
	}
	if (FJPtr U = Root->Get(TEXT("usage")); U && U->IsObject())
	{
		auto Num = [&](const TCHAR* K) -> int64 { FJPtr V = U->Get(K); return V && V->IsNumber() ? static_cast<int64>(V->Number) : 0; };
		R.InputTokens = Num(TEXT("input_tokens")) + Num(TEXT("cache_creation_input_tokens")) + Num(TEXT("cache_read_input_tokens"));
		R.OutputTokens = Num(TEXT("output_tokens"));
	}
	return R;
}
