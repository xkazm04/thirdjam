// FJamCliTransport (ruling R-0004 point 2): the Designer's request through the Claude Code CLI login, for development and the
// operator's demo build. One `claude -p` process per attempt; the twin of tools/designer-eval/cli-model.mjs.
// The assembled request body is mapped to CLI flags; the user message (with the player's sentence) goes on stdin, never on
// argv; the 31.8 KB system prompt goes in a file (--system-prompt-file: the Windows command line stops at 32767 characters).
// The process runs in a neutral temp folder with no setting sources, no tools and no MCP servers. The CLI's JSON result is
// turned back into a Messages API response body, so the attempt chain, validation and repair are the HTTP path's own.
#include "JamDesignerTransport.h"
#include "JamDesignerPrompt.h"
#include "JamJson.h"
#include "Async/Async.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformTime.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

static int32 GJamDesignerCliProcesses = 0;

namespace JamDesignerCliImpl
{
	using FJ = FJamJson;
	using FJPtr = TSharedPtr<FJamJson>;

	static FJPtr Str(const FString& S) { FJPtr N = MakeShared<FJ>(); N->Type = FJ::EType::String; N->String = S; return N; }
	static FJPtr Obj() { FJPtr N = MakeShared<FJ>(); N->Type = FJ::EType::Object; return N; }

	static FString Quote(const FString& A) { return TEXT("\"") + A + TEXT("\""); }

	struct FRun { FString Stdout; int32 ExitCode = -1; bool bTimedOut = false; bool bStarted = false; };

	static FRun RunBlocking(const FString& Exe, const FString& Args, const FString& Stdin, const FString& WorkDir, float TimeoutSec)
	{
		FRun R;
		void* InRead = nullptr; void* InWrite = nullptr; void* OutRead = nullptr; void* OutWrite = nullptr;
		if (!FPlatformProcess::CreatePipe(InRead, InWrite, /*bWritePipeLocal*/ true) || !FPlatformProcess::CreatePipe(OutRead, OutWrite, false))
		{
			FPlatformProcess::ClosePipe(InRead, InWrite);
			FPlatformProcess::ClosePipe(OutRead, OutWrite);
			return R;
		}
		FString Url = Exe;
		FString Params = Args;
		if (Exe.EndsWith(TEXT(".cmd"), ESearchCase::IgnoreCase) || Exe.EndsWith(TEXT(".bat"), ESearchCase::IgnoreCase))
		{
			// A batch file runs through the command interpreter.
			FString Comspec = FPlatformMisc::GetEnvironmentVariable(TEXT("ComSpec"));
			Url = Comspec.IsEmpty() ? FString(TEXT("cmd.exe")) : Comspec;
			Params = FString::Printf(TEXT("/d /s /c \"%s %s\""), *Quote(Exe), *Args);
		}
		uint32 Pid = 0;
		FProcHandle Proc = FPlatformProcess::CreateProc(*Url, *Params, false, true, true, &Pid, 0, *WorkDir, OutWrite, InRead);
		// The child holds its own copies of its pipe ends.
		FPlatformProcess::ClosePipe(InRead, nullptr);
		if (!Proc.IsValid())
		{
			FPlatformProcess::ClosePipe(nullptr, InWrite);
			FPlatformProcess::ClosePipe(OutRead, OutWrite);
			return R;
		}
		++GJamDesignerCliProcesses;
		R.bStarted = true;

		// stdin on its own thread: a full pipe blocks the writer, never the reader.
		FTCHARToUTF8 Utf8(*Stdin, Stdin.Len());
		TArray<uint8> Bytes(reinterpret_cast<const uint8*>(Utf8.Get()), Utf8.Length());
		TFuture<void> Writer = Async(EAsyncExecution::Thread, [InWrite, Bytes = MoveTemp(Bytes)]()
		{
			int32 Done = 0;
			while (Done < Bytes.Num())
			{
				int32 N = 0;
				if (!FPlatformProcess::WritePipe(InWrite, Bytes.GetData() + Done, Bytes.Num() - Done, &N) || N <= 0) break;
				Done += N;
			}
			FPlatformProcess::ClosePipe(nullptr, InWrite);
		});

		TArray<uint8> Out;
		const double Start = FPlatformTime::Seconds();
		while (FPlatformProcess::IsProcRunning(Proc))
		{
			TArray<uint8> Chunk;
			if (FPlatformProcess::ReadPipeToArray(OutRead, Chunk)) Out.Append(Chunk);
			if (FPlatformTime::Seconds() - Start > TimeoutSec)
			{
				R.bTimedOut = true;
				FPlatformProcess::TerminateProc(Proc, /*KillTree*/ true); // our own child only
				break;
			}
			FPlatformProcess::Sleep(0.02f);
		}
		for (int32 i = 0; i < 100; ++i)
		{
			TArray<uint8> Chunk;
			if (!FPlatformProcess::ReadPipeToArray(OutRead, Chunk) || Chunk.Num() == 0) break;
			Out.Append(Chunk);
		}
		FPlatformProcess::GetProcReturnCode(Proc, &R.ExitCode);
		FPlatformProcess::CloseProc(Proc);
		Writer.Wait();
		FPlatformProcess::ClosePipe(OutRead, OutWrite);
		R.Stdout = FString(FUTF8ToTCHAR(reinterpret_cast<const ANSICHAR*>(Out.GetData()), Out.Num()));
		return R;
	}
}

using namespace JamDesignerCliImpl;

FJamCliTransport::FJamCliTransport(const FString& InExe)
	: Exe(InExe)
	, WorkDir(FPaths::Combine(FPlatformProcess::UserTempDir(), TEXT("hokej-designer-cli")))
{
}

int32 FJamCliTransport::GetProcessesStarted()
{
	return GJamDesignerCliProcesses;
}

FString FJamCliTransport::FindClaudeOnPath()
{
	FString Path = FPlatformMisc::GetEnvironmentVariable(TEXT("PATH"));
	TArray<FString> Dirs;
	Path.ParseIntoArray(Dirs, TEXT(";"));
	static const TCHAR* Names[] = { TEXT("claude.exe"), TEXT("claude.cmd"), TEXT("claude.bat") };
	for (FString Dir : Dirs)
	{
		Dir.TrimStartAndEndInline();
		Dir.TrimQuotesInline();
		if (Dir.IsEmpty()) continue;
		for (const TCHAR* N : Names)
		{
			const FString P = FPaths::Combine(Dir, N);
			if (FPaths::FileExists(P)) return P;
		}
	}
	return FString();
}

FString FJamCliTransport::BuildArgs(const FString& Model, const FString& Effort, const FString& SystemFile)
{
	return FString::Printf(TEXT("-p --model %s --output-format json --max-turns 1 --system-prompt-file %s --tools \"\" --no-session-persistence --setting-sources \"\" --strict-mcp-config --effort %s"),
		*Model, *Quote(SystemFile), *Effort);
}

FString FJamCliTransport::ToApiBody(const FString& CliStdout, int32 ExitCode, int32& OutStatus)
{
	FString Err;
	FJPtr Root = FJ::Parse(CliStdout.TrimStartAndEnd(), Err);
	if (!Root || !Root->IsObject())
	{
		OutStatus = 502;
		FJPtr E = Obj(), M = Obj();
		M->Object.Add({ TEXT("message"), Str(FString::Printf(TEXT("claude CLI exit %d, no JSON result"), ExitCode)) });
		E->Object.Add({ TEXT("error"), M });
		return E->ToCompact();
	}
	FJPtr IsError = Root->Get(TEXT("is_error"));
	FJPtr Result = Root->Get(TEXT("result"));
	const FString ResultText = Result && Result->IsString() ? Result->String : FString();
	if ((IsError && IsError->IsBool() && IsError->Bool) || ExitCode != 0)
	{
		FJPtr Status = Root->Get(TEXT("api_error_status"));
		OutStatus = Status && Status->IsNumber() ? static_cast<int32>(Status->Number) : 503;
		FJPtr E = Obj(), M = Obj();
		M->Object.Add({ TEXT("message"), Str(TEXT("claude CLI: ") + ResultText.Left(200)) });
		E->Object.Add({ TEXT("error"), M });
		return E->ToCompact();
	}
	OutStatus = 200;
	FJPtr Body = Obj();
	TSharedPtr<FJamJson> Content = MakeShared<FJamJson>();
	Content->Type = FJ::EType::Array;
	FJPtr Block = Obj();
	Block->Object.Add({ TEXT("type"), Str(TEXT("text")) });
	Block->Object.Add({ TEXT("text"), Str(ResultText) });
	Content->Array.Add(Block);
	Body->Object.Add({ TEXT("content"), Content });
	if (FJPtr Stop = Root->Get(TEXT("stop_reason")); Stop && Stop->IsString()) Body->Object.Add({ TEXT("stop_reason"), Stop });
	if (FJPtr Usage = Root->Get(TEXT("usage")); Usage && Usage->IsObject()) Body->Object.Add({ TEXT("usage"), Usage });
	return Body->ToCompact();
}

void FJamCliTransport::Post(const FString& Url, const TArray<TPair<FString, FString>>& Headers, const FString& Body, float TimeoutSec, FJamDesignerHttpDone Done)
{
	FString Err;
	FJPtr Req = FJ::Parse(Body, Err);
	FString Model, Effort = TEXT("low"), System, User;
	if (Req && Req->IsObject())
	{
		if (FJPtr M = Req->Get(TEXT("model")); M && M->IsString()) Model = M->String;
		if (FJPtr S = Req->Get(TEXT("system")); S && S->IsArray())
		{
			for (const FJPtr& B : S->Array) if (FJPtr T = B ? B->Get(TEXT("text")) : FJPtr(); T && T->IsString()) System += (System.IsEmpty() ? TEXT("") : TEXT("\n\n")) + T->String;
		}
		if (FJPtr Ms = Req->Get(TEXT("messages")); Ms && Ms->IsArray())
		{
			for (const FJPtr& M : Ms->Array) if (FJPtr C = M ? M->Get(TEXT("content")) : FJPtr(); C && C->IsString()) User += (User.IsEmpty() ? TEXT("") : TEXT("\n\n")) + C->String;
		}
		if (FJPtr O = Req->Get(TEXT("output_config")); O && O->IsObject())
		{
			if (FJPtr E = O->Get(TEXT("effort")); E && E->IsString()) Effort = E->String;
		}
	}
	// Model and effort reach argv, so only plain identifiers are accepted there.
	auto Plain = [](const FString& S) { if (S.IsEmpty()) return false; for (TCHAR C : S) if (!FChar::IsAlnum(C) && C != '-' && C != '.' && C != '_') return false; return true; };
	if (!Plain(Model) || !Plain(Effort) || User.IsEmpty())
	{
		FJamDesignerHttpReply R;
		R.bConnected = true;
		R.Status = 400;
		R.Body = TEXT("{\"error\":{\"message\":\"the CLI transport could not read the request body\"}}");
		Done(R);
		return;
	}
	IFileManager::Get().MakeDirectory(*WorkDir, true);
	const FString SystemFile = FPaths::Combine(WorkDir, FString::Printf(TEXT("system-%016llx.txt"), FJamDesignerPrompt::Fnv1a64(System)));
	if (!FPaths::FileExists(SystemFile)) FFileHelper::SaveStringToFile(System, *SystemFile, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	const FString Args = BuildArgs(Model, Effort, SystemFile);
	const FString ExeCopy = Exe, Dir = WorkDir;

	Async(EAsyncExecution::Thread, [ExeCopy, Args, User, Dir, TimeoutSec, Done = MoveTemp(Done)]() mutable
	{
		const double Start = FPlatformTime::Seconds();
		const FRun Run = RunBlocking(ExeCopy, Args, User, Dir, TimeoutSec);
		FJamDesignerHttpReply R;
		R.Ms = (FPlatformTime::Seconds() - Start) * 1000.0;
		R.bConnected = Run.bStarted && !Run.bTimedOut;
		if (R.bConnected) R.Body = ToApiBody(Run.Stdout, Run.ExitCode, R.Status);
		AsyncTask(ENamedThreads::GameThread, [Done = MoveTemp(Done), R]() { Done(R); });
	});
}
