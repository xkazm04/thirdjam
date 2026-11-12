// Lane A12, Phase 2b: shouted signals through Windows' built-in speech recognition (SAPI 5.4, in the OS since Vista, offline, no download;
// docs/experiments/ACE.md 1.5 b). An in-process recognizer with a command-and-control grammar of a few phrases per intent; recognitions are polled
// on the game thread once per simulation frame (never a callback thread). Off by default: the keyboard is the shipped path, judges have no mic.
// Non-Windows builds compile to a stub that reports "unavailable".
#pragma once

#include "CoreMinimal.h"
#include "SignalIntent.h"

struct FJamVoiceHeard
{
	FString Phrase;
	EJamTeamIntent Intent = EJamTeamIntent::None;
	float Confidence = 0.f;
};

class JAMGAME_API FJamSignalVoice
{
public:
	FJamSignalVoice();
	~FJamSignalVoice();

	// The phrases the grammar knows, per intent (the first one is the canonical word).
	static const TArray<TPair<FString, EJamTeamIntent>>& Phrases();
	// Pure: recognised text -> intent (case-insensitive, whole-phrase match, NAME None when unknown).
	static EJamTeamIntent IntentForPhrase(const FString& Text);

	// Creates the recognizer, grammar and audio input. False (and Status says why) when SAPI, an English recognizer or a microphone is missing.
	bool Start();
	void Stop();
	bool IsListening() const { return bListening; }
	// "listening" | "off" | the last error, for the settings row.
	const FString& GetStatus() const { return Status; }

	// Drains the recognizer's event queue; returns the phrases heard since the last poll (usually none or one).
	TArray<FJamVoiceHeard> Poll();

private:
	struct FImpl;
	FImpl* Impl = nullptr;
	bool bListening = false;
	FString Status = TEXT("off");
};
