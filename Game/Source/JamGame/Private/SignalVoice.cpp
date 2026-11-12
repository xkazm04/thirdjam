#include "SignalVoice.h"
#include "SignalSystem.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/WindowsHWrapper.h"
THIRD_PARTY_INCLUDES_START
#include <sapi.h>
THIRD_PARTY_INCLUDES_END
#include "Windows/HideWindowsPlatformTypes.h"
#include "Windows/WindowsPlatformMisc.h"
#endif

const TArray<TPair<FString, EJamTeamIntent>>& FJamSignalVoice::Phrases()
{
	static const TArray<TPair<FString, EJamTeamIntent>> P = {
		{ TEXT("defend"), EJamTeamIntent::Defend }, { TEXT("fall back"), EJamTeamIntent::Defend }, { TEXT("get back"), EJamTeamIntent::Defend }, { TEXT("cover the net"), EJamTeamIntent::Defend },
		{ TEXT("space"), EJamTeamIntent::Space }, { TEXT("spread out"), EJamTeamIntent::Space }, { TEXT("open ice"), EJamTeamIntent::Space }, { TEXT("get open"), EJamTeamIntent::Space },
		{ TEXT("pass"), EJamTeamIntent::Pass }, { TEXT("pass it"), EJamTeamIntent::Pass }, { TEXT("give me the puck"), EJamTeamIntent::Pass }, { TEXT("puck"), EJamTeamIntent::Pass },
		{ TEXT("nice one"), EJamTeamIntent::Praise }, { TEXT("nice"), EJamTeamIntent::Praise }, { TEXT("good job"), EJamTeamIntent::Praise }, { TEXT("well done"), EJamTeamIntent::Praise }, { TEXT("great"), EJamTeamIntent::Praise },
	};
	return P;
}

EJamTeamIntent FJamSignalVoice::IntentForPhrase(const FString& Text)
{
	FString T = Text.TrimStartAndEnd().ToLower();
	T.ReplaceInline(TEXT("."), TEXT(""));
	T.ReplaceInline(TEXT(","), TEXT(""));
	T.ReplaceInline(TEXT("!"), TEXT(""));
	for (const TPair<FString, EJamTeamIntent>& P : Phrases()) if (T == P.Key) return P.Value;
	return EJamTeamIntent::None;
}

#if PLATFORM_WINDOWS

struct FJamSignalVoice::FImpl
{
	ISpRecognizer* Recognizer = nullptr;
	ISpRecoContext* Context = nullptr;
	ISpRecoGrammar* Grammar = nullptr;
	ISpObjectToken* AudioToken = nullptr;
	bool bCom = false;

	template <typename T> static void SafeRelease(T*& P) { if (P) { P->Release(); P = nullptr; } }

	void Release()
	{
		if (Grammar) Grammar->SetRuleState(nullptr, nullptr, SPRS_INACTIVE);
		SafeRelease(Grammar);
		SafeRelease(Context);
		SafeRelease(Recognizer);
		SafeRelease(AudioToken);
		if (bCom) { FWindowsPlatformMisc::CoUninitialize(); bCom = false; }
	}

	// The default audio-in device as an object token (what sphelper's SpGetDefaultTokenFromCategoryId does, without ATL).
	bool DefaultAudioToken()
	{
		ISpObjectTokenCategory* Cat = nullptr;
		if (FAILED(CoCreateInstance(__uuidof(SpObjectTokenCategory), nullptr, CLSCTX_ALL, __uuidof(ISpObjectTokenCategory), reinterpret_cast<void**>(&Cat))) || !Cat) return false;
		LPWSTR DefaultId = nullptr;
		bool bOk = SUCCEEDED(Cat->SetId(SPCAT_AUDIOIN, 0)) && SUCCEEDED(Cat->GetDefaultTokenId(&DefaultId)) && DefaultId;
		if (bOk)
		{
			bOk = SUCCEEDED(CoCreateInstance(__uuidof(SpObjectToken), nullptr, CLSCTX_ALL, __uuidof(ISpObjectToken), reinterpret_cast<void**>(&AudioToken))) && AudioToken
				&& SUCCEEDED(AudioToken->SetId(nullptr, DefaultId, 0));
		}
		if (DefaultId) CoTaskMemFree(DefaultId);
		SafeRelease(Cat);
		return bOk;
	}
};

FJamSignalVoice::FJamSignalVoice() { Impl = new FImpl(); }
FJamSignalVoice::~FJamSignalVoice() { Stop(); delete Impl; Impl = nullptr; }

bool FJamSignalVoice::Start()
{
	if (bListening) return true;
	FImpl& I = *Impl;
	I.bCom = FWindowsPlatformMisc::CoInitialize();
	auto Fail = [&](const TCHAR* Why) { Status = Why; UE_LOG(LogJamSignals, Warning, TEXT("JAMVOICE unavailable: %s"), Why); I.Release(); return false; };

	// In-process recognizer with the default microphone; the shared recognizer would start the OS speech UI.
	if (FAILED(CoCreateInstance(__uuidof(SpInprocRecognizer), nullptr, CLSCTX_ALL, __uuidof(ISpRecognizer), reinterpret_cast<void**>(&I.Recognizer))) || !I.Recognizer)
		return Fail(TEXT("no SAPI recognizer (speech recognition is not installed for this Windows language)"));
	if (!I.DefaultAudioToken()) return Fail(TEXT("no microphone: Windows has no default audio input device"));
	if (FAILED(I.Recognizer->SetInput(I.AudioToken, 1))) return Fail(TEXT("the recognizer refused the default audio input"));
	if (FAILED(I.Recognizer->CreateRecoContext(&I.Context)) || !I.Context) return Fail(TEXT("no recognition context"));
	if (FAILED(I.Context->SetNotifyWin32Event())) return Fail(TEXT("no event queue"));
	const ULONGLONG Interest = SPFEI(SPEI_RECOGNITION) | SPFEI(SPEI_FALSE_RECOGNITION);
	I.Context->SetInterest(Interest, Interest);
	if (FAILED(I.Context->CreateGrammar(1, &I.Grammar)) || !I.Grammar) return Fail(TEXT("no grammar"));

	// One top-level rule whose initial state has a word transition per phrase straight to the end.
	SPSTATEHANDLE Rule = 0;
	if (FAILED(I.Grammar->GetRule(L"hokej_signals", 1, SPRAF_TopLevel | SPRAF_Active | SPRAF_Dynamic, 1, &Rule))) return Fail(TEXT("rule failed"));
	for (const TPair<FString, EJamTeamIntent>& P : Phrases())
	{
		if (FAILED(I.Grammar->AddWordTransition(Rule, nullptr, *P.Key, L" ", SPWT_LEXICAL, 1.0f, nullptr))) return Fail(TEXT("word transition failed"));
	}
	if (FAILED(I.Grammar->Commit(0))) return Fail(TEXT("grammar commit failed (no English recognizer?)"));
	if (FAILED(I.Grammar->SetGrammarState(SPGS_ENABLED)) || FAILED(I.Grammar->SetRuleState(nullptr, nullptr, SPRS_ACTIVE))) return Fail(TEXT("rule activation failed"));
	if (FAILED(I.Recognizer->SetRecoState(SPRST_ACTIVE))) return Fail(TEXT("recognizer activation failed"));
	bListening = true;
	Status = TEXT("listening");
	UE_LOG(LogJamSignals, Log, TEXT("JAMVOICE listening: SAPI in-process recognizer, %d phrases"), Phrases().Num());
	return true;
}

void FJamSignalVoice::Stop()
{
	if (Impl) Impl->Release();
	if (bListening) UE_LOG(LogJamSignals, Log, TEXT("JAMVOICE stopped"));
	bListening = false;
	if (Status == TEXT("listening")) Status = TEXT("off");
}

TArray<FJamVoiceHeard> FJamSignalVoice::Poll()
{
	TArray<FJamVoiceHeard> Out;
	if (!bListening || !Impl || !Impl->Context) return Out;
	SPEVENT Ev;
	ULONG Fetched = 0;
	while (SUCCEEDED(Impl->Context->GetEvents(1, &Ev, &Fetched)) && Fetched == 1)
	{
		if (Ev.eEventId == SPEI_RECOGNITION && Ev.elParamType == SPET_LPARAM_IS_OBJECT && Ev.lParam)
		{
			ISpRecoResult* Result = reinterpret_cast<ISpRecoResult*>(Ev.lParam);
			LPWSTR Text = nullptr;
			BYTE Attr = 0;
			if (SUCCEEDED(Result->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, 1, &Text, &Attr)) && Text)
			{
				FJamVoiceHeard H;
				H.Phrase = FString(Text);
				H.Intent = IntentForPhrase(H.Phrase);
				SPPHRASE* Phrase = nullptr;
				if (SUCCEEDED(Result->GetPhrase(&Phrase)) && Phrase)
				{
					H.Confidence = Phrase->pElements && Phrase->Rule.ulCountOfElements ? Phrase->pElements[0].SREngineConfidence : 0.f;
					CoTaskMemFree(Phrase);
				}
				Out.Add(H);
				CoTaskMemFree(Text);
			}
			Result->Release();
		}
		else if (Ev.elParamType == SPET_LPARAM_IS_OBJECT && Ev.lParam)
		{
			reinterpret_cast<IUnknown*>(Ev.lParam)->Release();
		}
		else if (Ev.elParamType == SPET_LPARAM_IS_POINTER && Ev.lParam)
		{
			CoTaskMemFree(reinterpret_cast<void*>(Ev.lParam));
		}
		Fetched = 0;
	}
	return Out;
}

#else

FJamSignalVoice::FJamSignalVoice() {}
FJamSignalVoice::~FJamSignalVoice() {}
bool FJamSignalVoice::Start() { Status = TEXT("unavailable on this platform"); return false; }
void FJamSignalVoice::Stop() { bListening = false; }
TArray<FJamVoiceHeard> FJamSignalVoice::Poll() { return {}; }

#endif
