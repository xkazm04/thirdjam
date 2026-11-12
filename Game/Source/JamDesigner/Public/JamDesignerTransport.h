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
