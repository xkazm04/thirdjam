// The C++ port of tools/designer-eval/assemble.mjs (request body, byte for byte: Jam.Designer.PromptParity) and of
// parseDesignerReply / validateDesignerOutput in schema/validate.mjs. The cartridge rules themselves are lane A1's
// UJamCartridge::Validate (the C++ mirror of validateCartridge); this file adds only the Designer-specific checks on top.
#pragma once

#include "CoreMinimal.h"

struct FJamJson;

namespace JamDesignerModels
{
	// Attempt chain (prompts/README.md): haiku fresh, haiku repair, sonnet repair.
	JAMDESIGNER_API extern const TCHAR* First;
	JAMDESIGNER_API extern const TCHAR* Repair;
	JAMDESIGNER_API extern const TCHAR* Retry;
	constexpr int32 MaxTokens = 8000;
}

enum class EJamDesignerReplyKind : uint8 { Cartridge, Refusal, Invalid };

struct JAMDESIGNER_API FJamDesignerParsedReply
{
	EJamDesignerReplyKind Kind = EJamDesignerReplyKind::Invalid;
	FString CartridgeJson;   // the reply text, trimmed, when Kind == Cartridge
	FString Refusal;         // when Kind == Refusal
	TArray<FString> Errors;  // when Kind == Invalid; every sentence starts "at <pointer>:"
};

// What one Messages API response carried (only the fields the Designer reads).
struct JAMDESIGNER_API FJamDesignerApiReply
{
	bool bParsed = false;
	FString Text;
	FString StopReason;
	FString ErrorMessage;
	int64 InputTokens = 0;   // input + cache creation + cache read, as live-model.mjs counts it
	int64 OutputTokens = 0;
};

struct JAMDESIGNER_API FJamDesignerPrompt
{
	// Embedded file (prompts, schema, cached cuts) as text; empty when unknown.
	static FString GetEmbeddedFile(const FString& Name);

	// buildSystemText(): designer.system.md with {{LIMITS}} (limits.json without blocked_terms), {{ENVELOPE_SCHEMA}} and
	// {{GAME_SCHEMA}} as JSON.stringify(x, null, 2). Built once.
	static const FString& GetSystemText();

	// assembleRequest(): the request body as compact JSON. Previous empty = first attempt; otherwise a repair attempt with
	// PreviousErrors. False (and OutError) on an empty or over-long sentence or an unparseable cartridge.
	static bool AssembleBody(const FString& Sentence, const FString& CartridgeJson, const FString& Model, const FString* PreviousOutput,
		const TArray<FString>& PreviousErrors, FString& OutBody, FString& OutError);

	// parseDesignerReply(text, base): strict JSON, refusal shape, then UJamCartridge::Validate + the Designer's immutability rules.
	static FJamDesignerParsedReply ParseReply(const FString& Text, const FString& BaseCartridgeJson);

	// validateDesignerOutput(base, out) without the cartridge validation (that part is A1's loader).
	static void CheckDesignerEdits(const FJamJson& Base, const FJamJson& Out, TArray<FString>& OutErrors);
	// C-0005 (lane A15): the frame-600 guard on one game.rules.hooks[] entry (mirror of hookEndsMatchEarly in validate.mjs).
	static bool HookEndsMatchEarly(const TSharedPtr<FJamJson>& Hook);

	static FJamDesignerApiReply ParseApiResponse(const FString& Body);

	// JSON.stringify(v, null, 2).
	static FString Pretty(const FJamJson& V, const FString& Indent = FString());
	static bool DeepEqual(const FJamJson* A, const FJamJson* B);
	static uint64 Fnv1a64(const FString& Text);
	static int32 GetLimit(const FString& Key, int32 Fallback);
};
