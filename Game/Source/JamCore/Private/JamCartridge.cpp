#include "JamCartridge.h"
#include "JamJson.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Internationalization/Regex.h"

#include "JamSchemaData.inl"

DEFINE_LOG_CATEGORY(LogJamCartridge);

// ---------------------------------------------------------------------------------------------------------------------
// The validator: a port of schema/validate.mjs (schema walker over the embedded schema files + checkEngine).
// ---------------------------------------------------------------------------------------------------------------------
namespace JamValidate
{
	using FJ = FJamJson;
	using FJPtr = TSharedPtr<FJamJson>;

	struct FSchemas
	{
		TMap<FString, FJPtr> Files;   // file name -> parsed schema
		FJPtr Limits;
		bool bOk = false;
		FString Error;
	};

	static const FSchemas& GetSchemas()
	{
		static FSchemas S = []
		{
			FSchemas Out;
			Out.bOk = true;
			for (const FJamEmbeddedSchemaFile& F : JamEmbeddedSchemaFiles)
			{
				const FString Text = FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(F.Data)));
				FString Err;
				FJPtr Root = FJamJson::Parse(Text, Err);
				if (!Root) { Out.bOk = false; Out.Error = FString::Printf(TEXT("embedded %s: %s"), F.Name, *Err); continue; }
				if (FString(F.Name) == TEXT("limits.json")) Out.Limits = Root;
				else Out.Files.Add(F.Name, Root);
			}
			return Out;
		}();
		return S;
	}

	static FString Show(const FString& P) { return FJ::Show(P); }
	static FString Num(double V) { return FJ::NumberToString(V); }

	static FString Quote(const FString& S)
	{
		FJ Tmp; Tmp.Type = FJ::EType::String; Tmp.String = S;
		return Tmp.ToCompact();
	}

	static FString Describe(const FJ& V)
	{
		switch (V.Type)
		{
		case FJ::EType::Null: return TEXT("null (omit the key instead of writing null)");
		case FJ::EType::Array: return FString::Printf(TEXT("an array of %d item%s"), V.Array.Num(), V.Array.Num() == 1 ? TEXT("") : TEXT("s"));
		case FJ::EType::String: return FString::Printf(TEXT("a string (%s)"), *Quote(V.String.Len() > 40 ? V.String.Left(40) + TEXT("...") : V.String));
		case FJ::EType::Number: return FString::Printf(TEXT("%s (%s)"), V.IsInteger() ? TEXT("a whole number") : TEXT("a number"), *Num(V.Number));
		case FJ::EType::Bool: return FString::Printf(TEXT("a boolean (%s)"), V.Bool ? TEXT("true") : TEXT("false"));
		default: return TEXT("an object");
		}
	}

	static FString Article(const FString& W)
	{
		return (W == TEXT("array") || W == TEXT("object") || W == TEXT("integer")) ? TEXT("an ") + W : TEXT("a ") + W;
	}

	static bool MatchesType(const FJ& V, const FString& T)
	{
		if (T == TEXT("object")) return V.IsObject();
		if (T == TEXT("array")) return V.IsArray();
		if (T == TEXT("string")) return V.IsString();
		if (T == TEXT("number")) return V.IsNumber() && FMath::IsFinite(V.Number);
		if (T == TEXT("integer")) return V.IsInteger();
		if (T == TEXT("boolean")) return V.IsBool();
		if (T == TEXT("null")) return V.IsNull();
		return false;
	}

	static bool DeepEqual(const FJ& A, const FJ& B)
	{
		if (A.Type != B.Type) return false;
		switch (A.Type)
		{
		case FJ::EType::Null: return true;
		case FJ::EType::Bool: return A.Bool == B.Bool;
		case FJ::EType::Number: return A.Number == B.Number;
		case FJ::EType::String: return A.String == B.String;
		case FJ::EType::Array:
			if (A.Array.Num() != B.Array.Num()) return false;
			for (int32 i = 0; i < A.Array.Num(); ++i) if (!DeepEqual(*A.Array[i], *B.Array[i])) return false;
			return true;
		case FJ::EType::Object:
			if (A.Object.Num() != B.Object.Num()) return false;
			for (const auto& KV : A.Object)
			{
				FJPtr Other = B.Get(KV.Key);
				if (!Other || !DeepEqual(*KV.Value, *Other)) return false;
			}
			return true;
		}
		return false;
	}

	struct FCtx { FString File; };

	static FJPtr ResolveRef(const FString& Ref, FCtx& InOutCtx)
	{
		FString File, Frag;
		if (!Ref.Split(TEXT("#"), &File, &Frag)) { File = Ref; Frag.Reset(); }
		const FSchemas& S = GetSchemas();
		if (!File.IsEmpty()) InOutCtx.File = File;
		const FJPtr* Root = S.Files.Find(InOutCtx.File);
		if (!Root) return nullptr;
		FJPtr Node = *Root;
		if (!Frag.IsEmpty())
		{
			TArray<FString> Parts;
			Frag.ParseIntoArray(Parts, TEXT("/"), true);
			for (const FString& P : Parts)
			{
				if (!Node) return nullptr;
				Node = Node->Get(P.Replace(TEXT("~1"), TEXT("/")).Replace(TEXT("~0"), TEXT("~")));
			}
		}
		return Node;
	}

	static void Walk(const FJ& Schema, const FJ& Value, const FString& Ptr, FCtx Ctx, TArray<FString>& Errors)
	{
		if (Schema.IsBool())
		{
			if (!Schema.Bool) Errors.Add(FString::Printf(TEXT("at %s: no value is allowed here."), *Show(Ptr)));
			return;
		}
		if (FJPtr Ref = Schema.Get(TEXT("$ref")))
		{
			FCtx Next = Ctx;
			FJPtr Target = ResolveRef(Ref->String, Next);
			if (!Target) { Errors.Add(FString::Printf(TEXT("at %s: schema $ref \"%s\" does not resolve."), *Show(Ptr), *Ref->String)); return; }
			Walk(*Target, Value, Ptr, Next, Errors);
			return;
		}
		const FString At = FString::Printf(TEXT("at %s:"), *Show(Ptr));

		if (FJPtr Type = Schema.Get(TEXT("type")))
		{
			TArray<FString> Types;
			if (Type->IsArray()) { for (const FJPtr& T : Type->Array) Types.Add(T->String); }
			else Types.Add(Type->String);
			bool bAny = false;
			for (const FString& T : Types) bAny |= MatchesType(Value, T);
			if (!bAny)
			{
				TArray<FString> Names;
				for (const FString& T : Types) Names.Add(Article(T));
				FString Want = Names.Last();
				if (Names.Num() > 1)
				{
					Want = FString::Join(TArrayView<const FString>(Names.GetData(), Names.Num() - 1), TEXT(", ")) + TEXT(" or ") + Names.Last();
				}
				Errors.Add(FString::Printf(TEXT("%s expected %s but found %s."), *At, *Want, *Describe(Value)));
				return;
			}
		}
		if (FJPtr Const = Schema.Get(TEXT("const")))
		{
			if (!DeepEqual(Value, *Const))
				Errors.Add(FString::Printf(TEXT("%s must be exactly %s but found %s."), *At, *Const->ToCompact(), *Value.ToCompact()));
		}
		if (FJPtr Enum = Schema.Get(TEXT("enum")))
		{
			bool bFound = false;
			TArray<FString> Opts;
			for (const FJPtr& E : Enum->Array) { bFound |= DeepEqual(*E, Value); Opts.Add(E->ToCompact()); }
			if (!bFound)
			{
				Errors.Add(FString::Printf(TEXT("%s %s is not allowed; use one of %s."), *At, *Value.ToCompact(), *FString::Join(Opts, TEXT(", "))));
				return;
			}
		}
		if (Value.IsNumber())
		{
			if (FJPtr Min = Schema.Get(TEXT("minimum")); Min && Value.Number < Min->Number)
				Errors.Add(FString::Printf(TEXT("%s %s is below the minimum %s."), *At, *Num(Value.Number), *Num(Min->Number)));
			if (FJPtr Max = Schema.Get(TEXT("maximum")); Max && Value.Number > Max->Number)
				Errors.Add(FString::Printf(TEXT("%s %s is above the maximum %s."), *At, *Num(Value.Number), *Num(Max->Number)));
		}
		if (Value.IsString())
		{
			const int32 L = Value.String.Len();
			if (FJPtr Min = Schema.Get(TEXT("minLength")); Min && L < Min->Number)
				Errors.Add(FString::Printf(TEXT("%s the string is %d characters long; it needs at least %s."), *At, L, *Num(Min->Number)));
			if (FJPtr Max = Schema.Get(TEXT("maxLength")); Max && L > Max->Number)
				Errors.Add(FString::Printf(TEXT("%s the string is %d characters long; the limit is %s."), *At, L, *Num(Max->Number)));
			if (FJPtr Pat = Schema.Get(TEXT("pattern")))
			{
				const FRegexPattern Pattern(Pat->String);
				FRegexMatcher M(Pattern, Value.String);
				if (!M.FindNext())
				{
					FJPtr Desc = Schema.Get(TEXT("description"));
					const FString Why = Desc ? TEXT(": ") + Desc->String : FString::Printf(TEXT(" (pattern %s)"), *Pat->String);
					Errors.Add(FString::Printf(TEXT("%s %s is not valid%s."), *At, *Quote(Value.String), *Why));
				}
			}
		}
		if (Value.IsArray())
		{
			const int32 N = Value.Array.Num();
			if (FJPtr Min = Schema.Get(TEXT("minItems")); Min && N < Min->Number)
				Errors.Add(FString::Printf(TEXT("%s the list has %d item%s; it needs at least %s."), *At, N, N == 1 ? TEXT("") : TEXT("s"), *Num(Min->Number)));
			if (FJPtr Max = Schema.Get(TEXT("maxItems")); Max && N > Max->Number)
				Errors.Add(FString::Printf(TEXT("%s the list has %d items; the limit is %s."), *At, N, *Num(Max->Number)));
			if (FJPtr Items = Schema.Get(TEXT("items")))
			{
				for (int32 i = 0; i < N; ++i) Walk(*Items, *Value.Array[i], FJ::PointerChild(Ptr, i), Ctx, Errors);
			}
		}
		if (Value.IsObject())
		{
			FJPtr Props = Schema.Get(TEXT("properties"));
			if (FJPtr Req = Schema.Get(TEXT("required")))
			{
				for (const FJPtr& R : Req->Array)
				{
					if (!Value.Has(R->String))
						Errors.Add(FString::Printf(TEXT("at %s: required key \"%s\" is missing."), *Show(FJ::PointerChild(Ptr, R->String)), *R->String));
				}
			}
			FJPtr Additional = Schema.Get(TEXT("additionalProperties"));
			for (const auto& KV : Value.Object)
			{
				FJPtr Prop = Props ? Props->Get(KV.Key) : nullptr;
				if (Prop)
				{
					Walk(*Prop, *KV.Value, FJ::PointerChild(Ptr, KV.Key), Ctx, Errors);
				}
				else if (Additional && Additional->IsBool() && !Additional->Bool)
				{
					TArray<FString> Allowed;
					if (Props) for (const auto& P : Props->Object) Allowed.Add(P.Key);
					Errors.Add(FString::Printf(TEXT("at %s: unknown key \"%s\" is not allowed here; remove it.%s"), *Show(FJ::PointerChild(Ptr, KV.Key)), *KV.Key,
						Allowed.Num() ? *FString::Printf(TEXT(" Allowed keys: %s."), *FString::Join(Allowed, TEXT(", "))) : TEXT(" No keys are allowed in this object.")));
				}
				else if (Additional && Additional->IsObject())
				{
					Walk(*Additional, *KV.Value, FJ::PointerChild(Ptr, KV.Key), Ctx, Errors);
				}
			}
		}
	}

	// ---------- engine checks (checkEngine in validate.mjs) ----------

	static double LimitNum(const TCHAR* Key, double Fallback)
	{
		const FSchemas& S = GetSchemas();
		if (S.Limits) if (FJPtr V = S.Limits->Get(Key); V && V->IsNumber()) return V->Number;
		return Fallback;
	}

	static bool IsWallClock(const FString& K)
	{
		static const FRegexPattern P(TEXT("_(seconds?|secs?|ms|millis|milliseconds|minutes?|mins?|hours?)$"));
		FRegexMatcher M(P, K);
		return M.FindNext();
	}

	static FString WallClockToFrames(const FString& K)
	{
		static const FRegexPattern P(TEXT("^(.*)_(seconds?|secs?|ms|millis|milliseconds|minutes?|mins?|hours?)$"));
		FRegexMatcher M(P, K);
		return M.FindNext() ? M.GetCaptureGroup(1) + TEXT("_frames") : K;
	}

	static void CheckParams(const FJPtr& Params, const FString& Ptr, TArray<FString>& Errors)
	{
		if (!Params || !Params->IsObject()) return;
		const int32 MaxKeys = static_cast<int32>(LimitNum(TEXT("max_param_keys"), 16));
		if (Params->Object.Num() > MaxKeys)
			Errors.Add(FString::Printf(TEXT("at %s: %d parameters; the engine allows at most %d."), *Show(Ptr), Params->Object.Num(), MaxKeys));
		static const FRegexPattern Snake(TEXT("^[a-z][a-z0-9]*(_[a-z0-9]+)*$"));
		for (const auto& KV : Params->Object)
		{
			const FString P = FJ::PointerChild(Ptr, KV.Key);
			FRegexMatcher M(Snake, KV.Key);
			if (!M.FindNext() || KV.Key.Len() > 40)
				Errors.Add(FString::Printf(TEXT("at %s: the parameter name %s must be snake_case (lowercase letters and digits separated by single underscores, starting with a letter, at most 40 characters)."), *Show(P), *Quote(KV.Key)));
			else if (IsWallClock(KV.Key))
				Errors.Add(FString::Printf(TEXT("at %s: \"%s\" counts wall-clock time. Time is physics frames (60 per second): rename it to \"%s\" and convert the number."), *Show(P), *KV.Key, *WallClockToFrames(KV.Key)));
			else if (KV.Key.EndsWith(TEXT("_frames")) && !(KV.Value->IsInteger() && KV.Value->Number >= 0))
				Errors.Add(FString::Printf(TEXT("at %s: \"%s\" is a duration in frames and must be a whole number of 0 or more, found %s."), *Show(P), *KV.Key, *KV.Value->ToCompact()));
		}
	}

	static TMap<FString, int32> CheckUniqueIds(const FJPtr& List, const FString& Ptr, const TCHAR* What, TArray<FString>& Errors, const TCHAR* IdKey = TEXT("id"))
	{
		TMap<FString, int32> Seen;
		if (!List || !List->IsArray()) return Seen;
		for (int32 i = 0; i < List->Array.Num(); ++i)
		{
			const FJPtr& Item = List->Array[i];
			FJPtr Id = Item->IsObject() ? Item->Get(IdKey) : nullptr;
			if (!Id || !Id->IsString()) continue;
			if (const int32* First = Seen.Find(Id->String))
			{
				Errors.Add(FString::Printf(TEXT("at %s: duplicate %s %s \"%s\"; it is already used at %s. The %ss in this list must be unique."),
					*Show(FJ::PointerChild(FJ::PointerChild(Ptr, i), IdKey)), What, IdKey, *Id->String, *Show(FJ::PointerChild(FJ::PointerChild(Ptr, *First), IdKey)), IdKey));
			}
			else Seen.Add(Id->String, i);
		}
		return Seen;
	}

	static void CheckCount(const FJPtr& List, int32 Max, const FString& Ptr, const TCHAR* What, TArray<FString>& Errors)
	{
		if (List && List->IsArray() && List->Array.Num() > Max)
			Errors.Add(FString::Printf(TEXT("at %s: %d %s; the engine allows at most %d. Remove %d."), *Show(Ptr), List->Array.Num(), What, Max, List->Array.Num() - Max));
	}

	static void CheckValueAgainst(const FJ& T, const FJ& V, const FString& Ptr, const FString& Label, TArray<FString>& Errors)
	{
		FJPtr KeyV = T.Get(TEXT("key"));
		const FString Name = KeyV && KeyV->IsString() ? KeyV->String : TEXT("?");
		FJPtr TypeV = T.Get(TEXT("type"));
		const FString Type = TypeV && TypeV->IsString() ? TypeV->String : FString();
		FJPtr Min = T.Get(TEXT("min")), Max = T.Get(TEXT("max"));
		const bool bMin = Min && Min->IsNumber(), bMax = Max && Max->IsNumber();
		if (Type == TEXT("number") || Type == TEXT("integer"))
		{
			if (!V.IsNumber()) { Errors.Add(FString::Printf(TEXT("at %s: %s must be a %s but is %s."), *Show(Ptr), *Label, *Type, *Describe(V))); return; }
			if (Type == TEXT("integer") && !V.IsInteger()) { Errors.Add(FString::Printf(TEXT("at %s: %s must be a whole number but is %s."), *Show(Ptr), *Label, *Num(V.Number))); return; }
			if (bMin && bMax && Min->Number > Max->Number) return;
			const FString Range = (bMin && bMax) ? FString::Printf(TEXT(" (allowed range %s to %s)"), *Num(Min->Number), *Num(Max->Number)) : FString();
			if (bMin && V.Number < Min->Number) Errors.Add(FString::Printf(TEXT("at %s: %s is %s, below the minimum %s%s."), *Show(Ptr), *Label, *Num(V.Number), *Num(Min->Number), *Range));
			else if (bMax && V.Number > Max->Number) Errors.Add(FString::Printf(TEXT("at %s: %s is %s, above the maximum %s%s."), *Show(Ptr), *Label, *Num(V.Number), *Num(Max->Number), *Range));
		}
		else if (Type == TEXT("boolean"))
		{
			if (!V.IsBool()) Errors.Add(FString::Printf(TEXT("at %s: %s must be true or false but is %s."), *Show(Ptr), *Label, *Describe(V)));
		}
		else if (Type == TEXT("enum"))
		{
			FJPtr Opts = T.Get(TEXT("options"));
			if (!Opts || !Opts->IsArray()) return;
			bool bFound = false;
			TArray<FString> Names;
			for (const FJPtr& O : Opts->Array) { Names.Add(O->IsString() ? O->String : O->ToCompact()); bFound |= (V.IsString() && O->IsString() && O->String == V.String); }
			if (!bFound) Errors.Add(FString::Printf(TEXT("at %s: %s is %s, which is not an option of \"%s\"; use one of %s."), *Show(Ptr), *Label, *V.ToCompact(), *Name, *FString::Join(Names, TEXT(", "))));
		}
	}

	static void CheckTunables(const FJPtr& Tunables, TArray<FString>& Errors)
	{
		CheckCount(Tunables, static_cast<int32>(LimitNum(TEXT("max_tunables"), 24)), TEXT("/tunables"), TEXT("tunables"), Errors);
		CheckUniqueIds(Tunables, TEXT("/tunables"), TEXT("tunable"), Errors, TEXT("key"));
		if (!Tunables || !Tunables->IsArray()) return;
		for (int32 i = 0; i < Tunables->Array.Num(); ++i)
		{
			const FJ& T = *Tunables->Array[i];
			if (!T.IsObject()) continue;
			const FString P = FString::Printf(TEXT("/tunables/%d"), i);
			FJPtr KeyV = T.Get(TEXT("key"));
			const FString Name = KeyV && KeyV->IsString() ? KeyV->String : FString::Printf(TEXT("#%d"), i);
			FJPtr TypeV = T.Get(TEXT("type"));
			const FString Type = TypeV && TypeV->IsString() ? TypeV->String : FString();
			if (KeyV && KeyV->IsString())
			{
				if (IsWallClock(KeyV->String))
					Errors.Add(FString::Printf(TEXT("at %s: \"%s\" counts wall-clock time. Time is physics frames (60 per second): name it \"%s\"."), *Show(FJ::PointerChild(P, TEXT("key"))), *KeyV->String, *WallClockToFrames(KeyV->String)));
				else if (KeyV->String.EndsWith(TEXT("_frames")) && Type != TEXT("integer"))
					Errors.Add(FString::Printf(TEXT("at %s: tunable \"%s\" is a duration in frames, so its type must be \"integer\"."), *Show(FJ::PointerChild(P, TEXT("type"))), *KeyV->String));
			}
			auto Has = [&T](const TCHAR* K) { return T.Has(K); };
			if (Type == TEXT("number") || Type == TEXT("integer"))
			{
				if (Has(TEXT("options"))) Errors.Add(FString::Printf(TEXT("at %s: only enum tunables have options; remove it from \"%s\"."), *Show(FJ::PointerChild(P, TEXT("options"))), *Name));
				for (const TCHAR* K : { TEXT("min"), TEXT("max") })
					if (!Has(K)) Errors.Add(FString::Printf(TEXT("at %s: %s tunable \"%s\" must declare \"%s\" so its range is bounded."), *Show(FJ::PointerChild(P, K)), *Type, *Name, K));
				FJPtr Min = T.Get(TEXT("min")), Max = T.Get(TEXT("max"));
				if (Min && Max && Min->IsNumber() && Max->IsNumber() && Min->Number > Max->Number)
					Errors.Add(FString::Printf(TEXT("at %s: min %s is above max %s for tunable \"%s\"."), *Show(FJ::PointerChild(P, TEXT("min"))), *Num(Min->Number), *Num(Max->Number), *Name));
				if (Type == TEXT("integer"))
				{
					for (const TCHAR* K : { TEXT("min"), TEXT("max") })
					{
						FJPtr V = T.Get(K);
						if (V && V->IsNumber() && !V->IsInteger())
							Errors.Add(FString::Printf(TEXT("at %s: integer tunable \"%s\" needs a whole-number %s, found %s."), *Show(FJ::PointerChild(P, K)), *Name, K, *Num(V->Number)));
					}
				}
				if (FJPtr D = T.Get(TEXT("default"))) CheckValueAgainst(T, *D, FJ::PointerChild(P, TEXT("default")), FString::Printf(TEXT("the default of tunable \"%s\""), *Name), Errors);
			}
			else if (Type == TEXT("boolean"))
			{
				for (const TCHAR* K : { TEXT("min"), TEXT("max"), TEXT("options") })
					if (Has(K)) Errors.Add(FString::Printf(TEXT("at %s: boolean tunable \"%s\" must not have \"%s\"; remove it."), *Show(FJ::PointerChild(P, K)), *Name, K));
				if (FJPtr D = T.Get(TEXT("default")); D && !D->IsBool())
					Errors.Add(FString::Printf(TEXT("at %s: the default of boolean tunable \"%s\" must be true or false, found %s."), *Show(FJ::PointerChild(P, TEXT("default"))), *Name, *Describe(*D)));
			}
			else if (Type == TEXT("enum"))
			{
				for (const TCHAR* K : { TEXT("min"), TEXT("max") })
					if (Has(K)) Errors.Add(FString::Printf(TEXT("at %s: enum tunable \"%s\" must not have \"%s\"; remove it."), *Show(FJ::PointerChild(P, K)), *Name, K));
				FJPtr Opts = T.Get(TEXT("options"));
				if (!Opts) Errors.Add(FString::Printf(TEXT("at %s: enum tunable \"%s\" must list its options."), *Show(FJ::PointerChild(P, TEXT("options"))), *Name));
				else if (Opts->IsArray())
				{
					TSet<FString> Seen; bool bDup = false; TArray<FString> Names;
					for (const FJPtr& O : Opts->Array) { const FString S = O->IsString() ? O->String : O->ToCompact(); bDup |= Seen.Contains(S); Seen.Add(S); Names.Add(S); }
					if (bDup) Errors.Add(FString::Printf(TEXT("at %s: the options of \"%s\" contain a duplicate."), *Show(FJ::PointerChild(P, TEXT("options"))), *Name));
					if (FJPtr D = T.Get(TEXT("default")); D && !(D->IsString() && Seen.Contains(D->String)))
						Errors.Add(FString::Printf(TEXT("at %s: the default %s of \"%s\" is not one of its options (%s)."), *Show(FJ::PointerChild(P, TEXT("default"))), *D->ToCompact(), *Name, *FString::Join(Names, TEXT(", "))));
				}
			}
		}
	}

	static void CheckValues(const FJ& Obj, TArray<FString>& Errors)
	{
		FJPtr Values = Obj.Get(TEXT("values"));
		FJPtr Tunables = Obj.Get(TEXT("tunables"));
		if (!Values || !Values->IsObject()) return;
		TMap<FString, FJPtr> ByKey;
		TArray<FString> Keys;
		if (Tunables && Tunables->IsArray())
			for (const FJPtr& T : Tunables->Array)
				if (T->IsObject()) if (FJPtr K = T->Get(TEXT("key")); K && K->IsString()) { if (!ByKey.Contains(K->String)) Keys.Add(K->String); ByKey.Add(K->String, T); }
		for (const auto& KV : Values->Object)
		{
			const FString P = FJ::PointerChild(TEXT("/values"), KV.Key);
			if (KV.Value->IsNull()) continue;
			FJPtr* T = ByKey.Find(KV.Key);
			if (!T)
			{
				Errors.Add(FString::Printf(TEXT("at %s: \"%s\" is not a declared tunable, so it cannot have a value. Tunable keys are: %s."), *Show(P), *KV.Key, Keys.Num() ? *FString::Join(Keys, TEXT(", ")) : TEXT("(none)")));
				continue;
			}
			CheckValueAgainst(**T, *KV.Value, P, FString::Printf(TEXT("the value of tunable \"%s\""), *KV.Key), Errors);
		}
	}

	static void CollectStrings(const FJ& V, const FString& Ptr, TArray<TPair<FString, FString>>& Out)
	{
		if (V.IsString()) Out.Emplace(Ptr, V.String);
		else if (V.IsArray()) for (int32 i = 0; i < V.Array.Num(); ++i) CollectStrings(*V.Array[i], FJ::PointerChild(Ptr, i), Out);
		else if (V.IsObject()) for (const auto& KV : V.Object) CollectStrings(*KV.Value, FJ::PointerChild(Ptr, KV.Key), Out);
	}

	static constexpr double SpawnMarginM = 0.5;
	static constexpr double DefaultCornerM = 5;

	static bool InsideRink(double X, double Z, double L, double W, double Corner)
	{
		const double Hx = L / 2 - SpawnMarginM;
		const double Hz = W / 2 - SpawnMarginM;
		if (FMath::Abs(X) > Hx || FMath::Abs(Z) > Hz) return false;
		const double R = FMath::Max(0.0, FMath::Min(FMath::Min(Corner - SpawnMarginM, Hx), Hz));
		const double Dx = FMath::Abs(X) - (Hx - R);
		const double Dz = FMath::Abs(Z) - (Hz - R);
		return !(Dx > 0 && Dz > 0 && Dx * Dx + Dz * Dz > R * R);
	}

	static double NumOr(const FJPtr& Obj, const TCHAR* Key, bool& bHas)
	{
		bHas = false;
		if (!Obj || !Obj->IsObject()) return 0;
		FJPtr V = Obj->Get(Key);
		if (V && V->IsNumber()) { bHas = true; return V->Number; }
		return 0;
	}

	static void CheckGame(const FJPtr& Game, TArray<FString>& Errors)
	{
		if (!Game || !Game->IsObject()) return;
		FJPtr Rink = Game->Get(TEXT("rink"));
		if (Rink && !Rink->IsObject()) Rink.Reset();
		bool bL, bW, bC, bG;
		const double L = NumOr(Rink, TEXT("length_m"), bL);
		const double W = NumOr(Rink, TEXT("width_m"), bW);
		const double CornerRaw = NumOr(Rink, TEXT("corner_radius_m"), bC);
		const double GoalW = NumOr(Rink, TEXT("goal_width_m"), bG);
		const bool bSize = bL && bW;
		const double Corner = bC ? CornerRaw : DefaultCornerM;

		if (bSize && bC && CornerRaw > FMath::Min(L, W) / 2)
			Errors.Add(FString::Printf(TEXT("at /game/rink/corner_radius_m: %s is more than half of the shorter rink side (%s); lower it."), *Num(CornerRaw), *Num(FMath::Min(L, W) / 2)));
		if (bSize && bG && GoalW >= W)
			Errors.Add(FString::Printf(TEXT("at /game/rink/goal_width_m: the net (%s m) must be narrower than the rink (%s m)."), *Num(GoalW), *Num(W)));
		if (bSize && Rink)
		{
			if (FJPtr Obs = Rink->Get(TEXT("obstacles")); Obs && Obs->IsArray())
			{
				for (int32 i = 0; i < Obs->Array.Num(); ++i)
				{
					bool bX, bZ;
					const double X = NumOr(Obs->Array[i], TEXT("x"), bX), Z = NumOr(Obs->Array[i], TEXT("z"), bZ);
					if (bX && bZ && !InsideRink(X, Z, L, W, Corner))
						Errors.Add(FString::Printf(TEXT("at /game/rink/obstacles/%d: the obstacle at [%s, %s] is outside the rink (x within %s to %s, z within %s to %s, allowing %s m for the boards and the rounded corners)."),
							i, *Num(X), *Num(Z), *Num(-L / 2), *Num(L / 2), *Num(-W / 2), *Num(W / 2), *Num(SpawnMarginM)));
				}
			}
		}

		FJPtr Rules = Game->Get(TEXT("rules"));
		if (Rules && Rules->IsObject())
		{
			FJPtr Mode = Rules->Get(TEXT("mode"));
			const FString M = Mode && Mode->IsString() ? Mode->String : FString();
			if (M == TEXT("first_to") && !Rules->Has(TEXT("score_to_win")))
				Errors.Add(TEXT("at /game/rules/score_to_win: mode \"first_to\" needs score_to_win (1 to 9)."));
			if (M == TEXT("timer") && !Rules->Has(TEXT("timer_frames")))
				Errors.Add(TEXT("at /game/rules/timer_frames: mode \"timer\" needs timer_frames (600 to 18000 frames, 60 per second)."));
		}

		FJPtr Sk = Game->Get(TEXT("skater"));
		{
			bool bMin, bSpeed;
			const double CMin = NumOr(Sk, TEXT("check_speed_min"), bMin), Speed = NumOr(Sk, TEXT("speed"), bSpeed);
			if (bMin && bSpeed && CMin > Speed)
				Errors.Add(FString::Printf(TEXT("at /game/skater/check_speed_min: %s is above the skater's top speed %s, so no check could ever land; lower it."), *Num(CMin), *Num(Speed)));
		}

		if (FJPtr Pickups = Game->Get(TEXT("pickups")); Pickups && Pickups->IsArray())
		{
			for (int32 i = 0; i < Pickups->Array.Num(); ++i)
			{
				bool bD;
				const double D = NumOr(Pickups->Array[i], TEXT("duration_frames"), bD);
				if (bD && !(D > 0))
					Errors.Add(FString::Printf(TEXT("at /game/pickups/%d/duration_frames: a pickup effect must last at least 1 frame, found %s."), i, *Num(D)));
			}
		}

		FJPtr Teams = Game->Get(TEXT("teams"));
		if (Teams && Teams->IsArray())
		{
			CheckUniqueIds(Teams, TEXT("/game/teams"), TEXT("team"), Errors);
			TArray<FString> Colours;
			for (const FJPtr& T : Teams->Array)
			{
				FJPtr C = T->IsObject() ? T->Get(TEXT("colour_hex")) : nullptr;
				Colours.Add(C && C->IsString() ? C->String.ToLower() : FString());
			}
			if (Colours.Num() == 2 && !Colours[0].IsEmpty() && Colours[0] == Colours[1])
				Errors.Add(TEXT("at /game/teams/1/colour_hex: both teams have the same jersey colour; players could not tell them apart."));
			TArray<FString> Players;
			struct FSp { double X, Z; FString P; };
			TArray<FSp> Spawns;
			for (int32 Ti = 0; Ti < Teams->Array.Num(); ++Ti)
			{
				const FJPtr& T = Teams->Array[Ti];
				if (!T->IsObject()) continue;
				FJPtr List = T->Get(TEXT("skaters"));
				if (!List || !List->IsArray()) continue;
				for (int32 Si = 0; Si < List->Array.Num(); ++Si)
				{
					const FJPtr& S = List->Array[Si];
					if (!S->IsObject()) continue;
					const FString P = FString::Printf(TEXT("/game/teams/%d/skaters/%d"), Ti, Si);
					if (FJPtr Role = S->Get(TEXT("role")); Role && Role->IsString() && Role->String == TEXT("player")) Players.Add(P + TEXT("/role"));
					FJPtr Sp = S->Get(TEXT("spawn"));
					if (Sp && Sp->IsArray() && Sp->Array.Num() == 2 && Sp->Array[0]->IsNumber() && Sp->Array[1]->IsNumber())
					{
						const double X = Sp->Array[0]->Number, Z = Sp->Array[1]->Number;
						if (bSize && !InsideRink(X, Z, L, W, Corner))
							Errors.Add(FString::Printf(TEXT("at %s/spawn: the spawn [%s, %s] is outside the rink (x within %s to %s, z within %s to %s, and inside the rounded corners). Move it toward the centre or enlarge the rink."),
								*P, *Num(X), *Num(Z), *Num(-(L / 2 - SpawnMarginM)), *Num(L / 2 - SpawnMarginM), *Num(-(W / 2 - SpawnMarginM)), *Num(W / 2 - SpawnMarginM)));
						for (const FSp& O : Spawns)
						{
							if (FMath::Sqrt(FMath::Square(O.X - X) + FMath::Square(O.Z - Z)) < 1)
								Errors.Add(FString::Printf(TEXT("at %s/spawn: the spawn [%s, %s] is less than 1 m from the spawn at %s; skaters would start inside each other."), *P, *Num(X), *Num(Z), *O.P));
						}
						Spawns.Add({ X, Z, P + TEXT("/spawn") });
					}
				}
			}
			if (Players.Num() != 1)
			{
				const FString Where = Players.Num() ? FString::Printf(TEXT(" (found at %s)"), *FString::Join(Players, TEXT(", "))) : FString();
				Errors.Add(FString::Printf(TEXT("at /game/teams: exactly one skater across both teams must have role \"player\", found %d%s."), Players.Num(), *Where));
			}
		}
	}

	static void CheckEngine(const FJ& Obj, TArray<FString>& Errors)
	{
		if (!Obj.IsObject()) return;
		CheckGame(Obj.Get(TEXT("game")), Errors);
		const int32 Size = Obj.ToCompact().Len();
		const int32 MaxBytes = static_cast<int32>(LimitNum(TEXT("max_json_bytes"), 24000));
		if (Size > MaxBytes) Errors.Add(FString::Printf(TEXT("at /: the cartridge is %d characters; the engine allows at most %d. Shorten lists and text."), Size, MaxBytes));

		CheckTunables(Obj.Get(TEXT("tunables")), Errors);
		CheckValues(Obj, Errors);

		FJPtr RulesL = Obj.Get(TEXT("rules"));
		CheckCount(RulesL, static_cast<int32>(LimitNum(TEXT("max_rules"), 24)), TEXT("/rules"), TEXT("rules"), Errors);
		CheckUniqueIds(RulesL, TEXT("/rules"), TEXT("rule"), Errors);
		if (RulesL && RulesL->IsArray())
			for (int32 i = 0; i < RulesL->Array.Num(); ++i)
				if (RulesL->Array[i]->IsObject()) CheckParams(RulesL->Array[i]->Get(TEXT("params")), FString::Printf(TEXT("/rules/%d/params"), i), Errors);

		FJPtr Ents = Obj.Get(TEXT("entities"));
		CheckCount(Ents, static_cast<int32>(LimitNum(TEXT("max_entities"), 32)), TEXT("/entities"), TEXT("entities"), Errors);
		TMap<FString, int32> EntityIds = CheckUniqueIds(Ents, TEXT("/entities"), TEXT("entity"), Errors);
		if (Ents && Ents->IsArray())
		{
			int64 Total = 0;
			for (int32 i = 0; i < Ents->Array.Num(); ++i)
			{
				const FJPtr& E = Ents->Array[i];
				if (!E->IsObject()) continue;
				FJPtr Count = E->Get(TEXT("count"));
				Total += (Count && Count->IsInteger()) ? static_cast<int64>(Count->Number) : 1;
				CheckParams(E->Get(TEXT("params")), FString::Printf(TEXT("/entities/%d/params"), i), Errors);
			}
			const int32 MaxInst = static_cast<int32>(LimitNum(TEXT("max_total_entity_instances"), 400));
			if (Total > MaxInst)
				Errors.Add(FString::Printf(TEXT("at /entities: the counts add up to %lld entity instances; the engine allows at most %d. Lower some \"count\" values or remove entities."), Total, MaxInst));
		}

		FJPtr Ai = Obj.Get(TEXT("ai"));
		CheckCount(Ai, static_cast<int32>(LimitNum(TEXT("max_ai"), 16)), TEXT("/ai"), TEXT("ai behaviours"), Errors);
		CheckUniqueIds(Ai, TEXT("/ai"), TEXT("ai behaviour"), Errors);
		if (Ai && Ai->IsArray())
		{
			for (int32 i = 0; i < Ai->Array.Num(); ++i)
			{
				const FJPtr& A = Ai->Array[i];
				if (!A->IsObject()) continue;
				CheckParams(A->Get(TEXT("params")), FString::Printf(TEXT("/ai/%d/params"), i), Errors);
				FJPtr Applies = A->Get(TEXT("applies_to"));
				if (Applies && Applies->IsArray() && Ents && Ents->IsArray())
				{
					TArray<FString> Known; EntityIds.GetKeys(Known);
					for (int32 j = 0; j < Applies->Array.Num(); ++j)
					{
						const FJPtr& Ref = Applies->Array[j];
						if (Ref->IsString() && !EntityIds.Contains(Ref->String))
							Errors.Add(FString::Printf(TEXT("at /ai/%d/applies_to/%d: \"%s\" is not the id of any entity. Entity ids are: %s."), i, j, *Ref->String, Known.Num() ? *FString::Join(Known, TEXT(", ")) : TEXT("(none)")));
					}
				}
			}
		}

		if (FJPtr Theme = Obj.Get(TEXT("theme")); Theme && Theme->IsObject())
		{
			CheckCount(Theme->Get(TEXT("palette")), static_cast<int32>(LimitNum(TEXT("max_palette"), 8)), TEXT("/theme/palette"), TEXT("palette colours"), Errors);
			CheckCount(Theme->Get(TEXT("mood_words")), static_cast<int32>(LimitNum(TEXT("max_mood_words"), 8)), TEXT("/theme/mood_words"), TEXT("mood words"), Errors);
			CheckUniqueIds(Theme->Get(TEXT("palette")), TEXT("/theme/palette"), TEXT("palette colour"), Errors, TEXT("name"));
		}

		const FSchemas& S = GetSchemas();
		if (S.Limits)
		{
			if (FJPtr Blocked = S.Limits->Get(TEXT("blocked_terms")); Blocked && Blocked->IsArray() && Blocked->Array.Num())
			{
				TArray<TPair<FString, FString>> Strings;
				CollectStrings(Obj, FString(), Strings);
				for (const auto& PS : Strings)
				{
					const FString Low = PS.Value.ToLower();
					for (const FJPtr& T : Blocked->Array)
					{
						const FString Term = T->IsString() ? T->String : T->ToCompact();
						if (Low.Contains(Term.ToLower()))
						{
							Errors.Add(FString::Printf(TEXT("at %s: the text contains a term that is not allowed in this game (\"%s\"). Reword it."), *Show(PS.Key), *Term));
							break;
						}
					}
				}
			}
		}
	}

	static bool ValidateTree(const FJ& Root, TArray<FString>& OutErrors)
	{
		const FSchemas& S = GetSchemas();
		if (!S.bOk)
		{
			OutErrors.Add(FString::Printf(TEXT("at /: the loader's embedded schema is broken (%s)."), *S.Error));
			return false;
		}
		TArray<FString> Errors;
		const FJPtr* Envelope = S.Files.Find(TEXT("cartridge.schema.json"));
		FCtx Ctx{ TEXT("cartridge.schema.json") };
		Walk(**Envelope, Root, FString(), Ctx, Errors);
		CheckEngine(Root, Errors);
		// Unique, in first-seen order (new Set(errors) in Node).
		for (const FString& E : Errors) OutErrors.AddUnique(E);
		return OutErrors.Num() == 0;
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// Loader
// ---------------------------------------------------------------------------------------------------------------------

namespace
{
	using FJPtr = TSharedPtr<FJamJson>;

	FJPtr Obj(const FJPtr& Parent, const TCHAR* Key)
	{
		if (!Parent) return nullptr;
		FJPtr V = Parent->Get(Key);
		return (V && V->IsObject()) ? V : nullptr;
	}

	void ReadNum(const FJPtr& O, const TCHAR* Key, double& Out) { if (O) if (FJPtr V = O->Get(Key); V && V->IsNumber()) Out = V->Number; }
	void ReadInt(const FJPtr& O, const TCHAR* Key, int32& Out) { if (O) if (FJPtr V = O->Get(Key); V && V->IsNumber()) Out = static_cast<int32>(V->Number); }
	void ReadBool(const FJPtr& O, const TCHAR* Key, bool& Out) { if (O) if (FJPtr V = O->Get(Key); V && V->IsBool()) Out = V->Bool; }
	void ReadName(const FJPtr& O, const TCHAR* Key, FName& Out) { if (O) if (FJPtr V = O->Get(Key); V && V->IsString()) Out = FName(*V->String); }
	void ReadStr(const FJPtr& O, const TCHAR* Key, FString& Out) { if (O) if (FJPtr V = O->Get(Key); V && V->IsString()) Out = V->String; }
	void ReadColour(const FJPtr& O, const TCHAR* Key, FLinearColor& Out) { if (O) if (FJPtr V = O->Get(Key); V && V->IsString()) UJamCartridge::HexToLinear(V->String, Out); }

	void FillGame(const FJPtr& G, FJamGameConfig& C)
	{
		// Defaults are the struct initialisers (schema/DEFAULTS.md); a present key overrides them.
		FJPtr Rink = Obj(G, TEXT("rink"));
		ReadNum(Rink, TEXT("length_m"), C.Rink.LengthM);
		ReadNum(Rink, TEXT("width_m"), C.Rink.WidthM);
		ReadNum(Rink, TEXT("corner_radius_m"), C.Rink.CornerRadiusM);
		ReadNum(Rink, TEXT("board_restitution"), C.Rink.BoardRestitution);
		ReadNum(Rink, TEXT("ice_friction"), C.Rink.IceFriction);
		ReadNum(Rink, TEXT("goal_width_m"), C.Rink.GoalWidthM);
		if (Rink) if (FJPtr Obs = Rink->Get(TEXT("obstacles")); Obs && Obs->IsArray())
		{
			for (const FJPtr& O : Obs->Array)
			{
				FJamObstacle Ob;
				ReadName(O, TEXT("kind"), Ob.Kind);
				ReadNum(O, TEXT("x"), Ob.X);
				ReadNum(O, TEXT("z"), Ob.Z);
				ReadNum(O, TEXT("size"), Ob.Size);
				C.Rink.Obstacles.Add(Ob);
			}
		}

		FJPtr Sk = Obj(G, TEXT("skater"));
		ReadNum(Sk, TEXT("speed"), C.Skater.Speed);
		ReadNum(Sk, TEXT("accel"), C.Skater.Accel);
		ReadNum(Sk, TEXT("turn_rate"), C.Skater.TurnRate);
		ReadNum(Sk, TEXT("lateral_grip"), C.Skater.LateralGrip);
		ReadNum(Sk, TEXT("mass"), C.Skater.Mass);
		ReadNum(Sk, TEXT("glide_drag"), C.Skater.GlideDrag);
		ReadNum(Sk, TEXT("brake_decel"), C.Skater.BrakeDecel);
		ReadBool(Sk, TEXT("check_enabled"), C.Skater.bCheckEnabled);
		ReadNum(Sk, TEXT("check_speed_min"), C.Skater.CheckSpeedMin);
		ReadInt(Sk, TEXT("knockdown_frames"), C.Skater.KnockdownFrames);
		ReadName(Sk, TEXT("visual"), C.Skater.Visual);

		FJPtr Pk = Obj(G, TEXT("puck"));
		UJamCartridge::HexToLinear(TEXT("#111111"), C.Puck.Colour);
		ReadNum(Pk, TEXT("radius_m"), C.Puck.RadiusM);
		ReadNum(Pk, TEXT("max_speed"), C.Puck.MaxSpeed);
		ReadNum(Pk, TEXT("drag"), C.Puck.Drag);
		ReadColour(Pk, TEXT("colour_hex"), C.Puck.Colour);

		FJPtr Shot = Obj(G, TEXT("shot"));
		ReadNum(Shot, TEXT("reach_m"), C.Shot.ReachM);
		ReadNum(Shot, TEXT("speed"), C.Shot.Speed);
		ReadInt(Shot, TEXT("cooldown_frames"), C.Shot.CooldownFrames);
		if (Shot && Shot->Has(TEXT("aim_cone_deg"))) { C.Shot.bHasAimCone = true; ReadNum(Shot, TEXT("aim_cone_deg"), C.Shot.AimConeDeg); }

		FJPtr Rules = Obj(G, TEXT("rules"));
		ReadName(Rules, TEXT("mode"), C.Rules.Mode);
		ReadInt(Rules, TEXT("score_to_win"), C.Rules.ScoreToWin);
		ReadInt(Rules, TEXT("timer_frames"), C.Rules.TimerFrames);
		ReadInt(Rules, TEXT("goal_pause_frames"), C.Rules.GoalPauseFrames);

		if (FJPtr Teams = G->Get(TEXT("teams")); Teams && Teams->IsArray())
		{
			for (const FJPtr& T : Teams->Array)
			{
				FJamTeamConfig Team;
				ReadStr(T, TEXT("id"), Team.Id);
				ReadStr(T, TEXT("colour_hex"), Team.ColourHex);
				UJamCartridge::HexToLinear(Team.ColourHex, Team.Colour);
				if (FJPtr List = T->Get(TEXT("skaters")); List && List->IsArray())
				{
					for (const FJPtr& S : List->Array)
					{
						FJamSkaterSpawn Sp;
						ReadName(S, TEXT("role"), Sp.Role);
						if (FJPtr P = S->Get(TEXT("spawn")); P && P->IsArray() && P->Array.Num() == 2)
						{
							Sp.X = P->Array[0]->Number;
							Sp.Z = P->Array[1]->Number;
						}
						Team.Skaters.Add(Sp);
					}
				}
				C.Teams.Add(Team);
			}
		}

		FJPtr Ai = Obj(G, TEXT("ai"));
		ReadNum(Ai, TEXT("yield_margin_m"), C.Ai.YieldMarginM);
		ReadNum(Ai, TEXT("yield_radius_m"), C.Ai.YieldRadiusM);
		ReadNum(Ai, TEXT("hold_radius_m"), C.Ai.HoldRadiusM);
		ReadNum(Ai, TEXT("speed_scale"), C.Ai.SpeedScale);
		ReadNum(Ai, TEXT("red_skill"), C.Ai.RedSkill);

		if (FJPtr Pickups = G->Get(TEXT("pickups")); Pickups && Pickups->IsArray())
		{
			for (const FJPtr& P : Pickups->Array)
			{
				FJamPickupConfig Pc;
				ReadName(P, TEXT("kind"), Pc.Kind);
				ReadInt(P, TEXT("spawn_every_frames"), Pc.SpawnEveryFrames);
				ReadInt(P, TEXT("duration_frames"), Pc.DurationFrames);
				const FString K = Pc.Kind.ToString();
				Pc.Magnitude = K == TEXT("speed") ? 1.5 : K == TEXT("precision") ? 2.0 : K == TEXT("heavy_puck") ? 2.0 : K == TEXT("big_puck") ? 1.5 : 1.0;
				ReadNum(P, TEXT("magnitude"), Pc.Magnitude);
				C.Pickups.Add(Pc);
			}
		}

		FJPtr Look = Obj(G, TEXT("look"));
		UJamCartridge::HexToLinear(TEXT("#38BDF8"), C.Look.TrimColour);
		ReadStr(Look, TEXT("tier"), C.Look.Tier);
		ReadColour(Look, TEXT("trim_colour_hex"), C.Look.TrimColour);
		ReadNum(Look, TEXT("fog_density"), C.Look.FogDensity);
		ReadBool(Look, TEXT("wet_ice"), C.Look.bWetIce);

		FJPtr Cam = Obj(G, TEXT("camera"));
		ReadName(Cam, TEXT("mode"), C.Camera.Mode);
		ReadNum(Cam, TEXT("distance_m"), C.Camera.DistanceM);
		ReadNum(Cam, TEXT("height_m"), C.Camera.HeightM);
		ReadNum(Cam, TEXT("look_ahead_m"), C.Camera.LookAheadM);
		ReadInt(Cam, TEXT("lag_frames"), C.Camera.LagFrames);
		ReadNum(Cam, TEXT("fov_deg"), C.Camera.FovDeg);
		ReadBool(Cam, TEXT("radar"), C.Camera.bRadar);
	}

	// schema/README.md "Tunables": a tunable named after a game field dials that field.
	void ApplyValues(const FJPtr& Values, FJamGameConfig& C)
	{
		if (!Values || !Values->IsObject()) return;
		for (const auto& KV : Values->Object)
		{
			if (!KV.Value->IsNumber()) continue;
			const double V = KV.Value->Number;
			const FString& K = KV.Key;
			if (K == TEXT("skater_speed")) C.Skater.Speed = V;
			else if (K == TEXT("skater_turn_rate")) C.Skater.TurnRate = V;
			else if (K == TEXT("shot_speed")) C.Shot.Speed = V;
			else if (K == TEXT("puck_max_speed")) C.Puck.MaxSpeed = V;
			else if (K == TEXT("ice_friction")) C.Rink.IceFriction = V;
			else if (K == TEXT("board_restitution")) C.Rink.BoardRestitution = V;
			else if (K == TEXT("ai_speed_scale")) C.Ai.SpeedScale = V;
			else if (K == TEXT("ai_red_skill")) C.Ai.RedSkill = V;
			else if (K == TEXT("goal_pause_frames")) C.Rules.GoalPauseFrames = static_cast<int32>(V);
			else UE_LOG(LogJamCartridge, Verbose, TEXT("value \"%s\" names no game field; kept for the Designer only"), *K);
		}
	}
}

bool UJamCartridge::HexToLinear(const FString& Hex, FLinearColor& Out)
{
	if (Hex.Len() != 7 || Hex[0] != '#') return false;
	for (int32 i = 1; i < 7; ++i) if (!FChar::IsHexDigit(Hex[i])) return false;
	const FColor C = FColor::FromHex(Hex);
	Out = FLinearColor::FromSRGBColor(C);
	Out.A = 1.f;
	return true;
}

bool UJamCartridge::Validate(const FString& Json, TArray<FString>& OutErrors)
{
	FString ParseError;
	TSharedPtr<FJamJson> Root = FJamJson::Parse(Json, ParseError);
	if (!Root) { OutErrors.Add(ParseError); return false; }
	return JamValidate::ValidateTree(*Root, OutErrors);
}

UJamCartridge* UJamCartridge::LoadFromString(const FString& Json, TArray<FString>& OutErrors, UObject* Outer)
{
	FString ParseError;
	TSharedPtr<FJamJson> Root = FJamJson::Parse(Json, ParseError);
	if (!Root) { OutErrors.Add(ParseError); return nullptr; }
	if (!JamValidate::ValidateTree(*Root, OutErrors)) return nullptr;

	UJamCartridge* C = NewObject<UJamCartridge>(Outer ? Outer : GetTransientPackage());
	C->RawJson = Json;
	ReadStr(Root, TEXT("id"), C->Id);
	ReadStr(Root, TEXT("title"), C->Title);
	if (FJPtr Theme = Obj(Root, TEXT("theme"))) ReadStr(Theme, TEXT("name"), C->ThemeName);
	if (FJPtr Seed = Root->Get(TEXT("seed")); Seed && Seed->IsNumber()) C->Seed = static_cast<int64>(Seed->Number);
	FillGame(Root->Get(TEXT("game")), C->Game);
	ApplyValues(Root->Get(TEXT("values")), C->Game);
	return C;
}

UJamCartridge* UJamCartridge::LoadFromFile(const FString& Path, TArray<FString>& OutErrors, UObject* Outer)
{
	FString Text;
	if (!FFileHelper::LoadFileToString(Text, *Path))
	{
		OutErrors.Add(FString::Printf(TEXT("at /: cannot read the cartridge file %s."), *Path));
		return nullptr;
	}
	UJamCartridge* C = LoadFromString(Text, OutErrors, Outer);
	if (C) C->SourcePath = Path;
	return C;
}

FString UJamCartridge::GetCartridgeDir()
{
	return FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Cartridges"));
}

FString UJamCartridge::ResolveCartridgePath(const FString& NameOrPath)
{
	if (NameOrPath.EndsWith(TEXT(".json"))) return NameOrPath;
	return FPaths::Combine(GetCartridgeDir(), NameOrPath + TEXT(".cartridge.json"));
}
