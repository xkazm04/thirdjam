#include "JamEventBus.h"

DEFINE_LOG_CATEGORY(LogJamEvents);

namespace JamEvents
{
	const FName Shot(TEXT("shot"));
	const FName Board(TEXT("board"));
	const FName Goal(TEXT("goal"));
	const FName Faceoff(TEXT("faceoff"));
	const FName Check(TEXT("check"));
	const FName Knockdown(TEXT("knockdown"));
	const FName PickupSpawn(TEXT("pickup_spawn"));
	const FName PickupTake(TEXT("pickup_take"));
	const FName Win(TEXT("win"));
	const FName Lose(TEXT("lose"));
}

TArray<FName> UJamEventBus::GetKnownEvents()
{
	return { JamEvents::Shot, JamEvents::Board, JamEvents::Goal, JamEvents::Faceoff, JamEvents::Check,
		JamEvents::Knockdown, JamEvents::PickupSpawn, JamEvents::PickupTake, JamEvents::Win, JamEvents::Lose };
}

bool UJamEventBus::IsKnownEvent(FName Event)
{
	return GetKnownEvents().Contains(Event);
}

bool UJamEventBus::Broadcast(FName Event, const FJamEventPayload& Payload)
{
	if (!IsKnownEvent(Event))
	{
		UE_LOG(LogJamEvents, Error, TEXT("JAMEVENT_UNKNOWN %s is not in contracts/events.md; not delivered"), *Event.ToString());
		return false;
	}
	++BroadcastCount;
	if (bLogEvents)
	{
		UE_LOG(LogJamEvents, Log, TEXT("JAMEVENT %s who=%d where=(%.1f,%.1f,%.1f) strength=%.3f frame=%llu"),
			*Event.ToString(), Payload.Who, Payload.Where.X, Payload.Where.Y, Payload.Where.Z, Payload.Strength,
			static_cast<unsigned long long>(GFrameCounter));
	}
	OnJamEvent.Broadcast(Event, Payload);
	return true;
}
