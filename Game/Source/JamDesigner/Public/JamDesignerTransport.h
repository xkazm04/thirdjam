// The one door the Designer uses to reach the network. Production: FJamDesignerHttpTransport (FHttpModule, explicit timeout,
// completion on the game thread). Tests swap in a mock that counts calls (Jam.Designer.NoKeyNoNetwork).
#pragma once

#include "CoreMinimal.h"

struct FJamDesignerHttpReply
{
	// False when no HTTP response arrived (timeout, DNS, connection refused).
	bool bConnected = false;
	int32 Status = 0;
	FString Body;
	double Ms = 0;
};

using FJamDesignerHttpDone = TFunction<void(const FJamDesignerHttpReply&)>;

class JAMDESIGNER_API IJamDesignerTransport
{
public:
	virtual ~IJamDesignerTransport() = default;
	// Headers may carry the key: an implementation never logs, stores or echoes them. Done runs on the game thread.
	virtual void Post(const FString& Url, const TArray<TPair<FString, FString>>& Headers, const FString& Body, float TimeoutSec, FJamDesignerHttpDone Done) = 0;
};

class JAMDESIGNER_API FJamDesignerHttpTransport : public IJamDesignerTransport
{
public:
	virtual void Post(const FString& Url, const TArray<TPair<FString, FString>>& Headers, const FString& Body, float TimeoutSec, FJamDesignerHttpDone Done) override;
	// Number of IHttpRequest objects this process created through the Designer (NoKeyNoNetwork reads it).
	static int32 GetRequestsCreated();
};

// Ruling R-0004 point 2: the same request through `claude -p` (the Claude Code CLI login). Development and the operator's demo
// build only; the subsystem picks it when there is no API key, `claude` is on PATH, and the build is not Shipping (or
// bAllowCliTransport=True under [/Script/JamDesigner.JamDesignerSubsystem] in DefaultGame.ini). Runs off the game thread;
// Done runs on the game thread. The sentence travels on stdin, never on argv.
class JAMDESIGNER_API FJamCliTransport : public IJamDesignerTransport
{
public:
	explicit FJamCliTransport(const FString& InExe);
	virtual void Post(const FString& Url, const TArray<TPair<FString, FString>>& Headers, const FString& Body, float TimeoutSec, FJamDesignerHttpDone Done) override;

	// claude.exe / claude.cmd / claude.bat in the first PATH folder that has one; empty when none.
	static FString FindClaudeOnPath();
	static FString BuildArgs(const FString& Model, const FString& Effort, const FString& SystemFile);
	// The CLI's --output-format json result as a Messages API response body; OutStatus 200, or the error status.
	static FString ToApiBody(const FString& CliStdout, int32 ExitCode, int32& OutStatus);
	// Processes this process started through the CLI transport (tests read it).
	static int32 GetProcessesStarted();

private:
	FString Exe;
	FString WorkDir;
};
