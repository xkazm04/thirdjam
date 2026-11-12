#include "JamDesignerTransport.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "HAL/PlatformTime.h"

static int32 GJamDesignerRequestsCreated = 0;

int32 FJamDesignerHttpTransport::GetRequestsCreated()
{
	return GJamDesignerRequestsCreated;
}

void FJamDesignerHttpTransport::Post(const FString& Url, const TArray<TPair<FString, FString>>& Headers, const FString& Body, float TimeoutSec, FJamDesignerHttpDone Done)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
	++GJamDesignerRequestsCreated;
	Req->SetURL(Url);
	Req->SetVerb(TEXT("POST"));
	for (const TPair<FString, FString>& H : Headers) Req->SetHeader(H.Key, H.Value);
	Req->SetContentAsString(Body);
	// Explicit total timeout: a stuck connection never leaves the Designer waiting forever.
	Req->SetTimeout(TimeoutSec);
	const double Started = FPlatformTime::Seconds();
	// The completion delegate runs on the game thread (default HTTP delegate thread policy), so callers may touch UObjects.
	Req->OnProcessRequestComplete().BindLambda([Done = MoveTemp(Done), Started](FHttpRequestPtr, FHttpResponsePtr Resp, bool bOk)
	{
		FJamDesignerHttpReply R;
		R.Ms = (FPlatformTime::Seconds() - Started) * 1000.0;
		R.bConnected = bOk && Resp.IsValid();
		if (Resp.IsValid())
		{
			R.Status = Resp->GetResponseCode();
			R.Body = Resp->GetContentAsString();
		}
		Done(R);
	});
	Req->ProcessRequest();
}
