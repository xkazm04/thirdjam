# C-0003: gameplay events

id: C-0003
version: 1
status: active
owner: Director
parties: A, B
opened: H4

Rules for every contract are in `contracts/README.md` (including the absent-value convention). This file only fills in the specifics.

## Purpose

Gameplay (device A) announces what happens on the ice through one event bus; device B binds VFX and sounds to those events without touching play
code, and can build against the stub today by calling `Broadcast` itself from a test map. Events are cosmetic only: no gameplay code reads them,
so they can never change the simulation or its state hash. C++ source: `Game/Source/JamCore/Public/JamEventBus.h`.

## Inputs, producers and first consumers

| Input | Producer | First consumer | Notes |
|---|---|---|---|
| `UJamEventBus::Broadcast(Event, Payload)` | A, gameplay (simulation tick) | B, VFX/audio binder (`BP_EventFx`) | stub compiled at H4 |
| `OnJamEvent` delegate | A, `UJamEventBus` | B, `BP_EventFx` binds it | dynamic multicast, Blueprint-assignable |
| log line `JAMEVENT <name> who=<int> where=(x,y,z) strength=<float> frame=<uint64>` | A, `UJamEventBus` | A, bot tests (count events per match) | `JAMEVENT_UNKNOWN <name>` is an error line |
| `NS_<event>` Niagara systems | B | B, `BP_EventFx` | one per event that has a visual |
| `MS_<event>` MetaSound sources | B | B, `BP_EventFx` | one per event that has a sound |

## Types and fields

### `UJamEventBus` (C++ `UGameInstanceSubsystem`, module `JamCore`)

| Member | Signature | Meaning |
|---|---|---|
| `Broadcast` | `bool Broadcast(FName Event, const FJamEventPayload& Payload)` | known event: logs it (when `bLogEvents`), fires `OnJamEvent`, returns true; unknown: logs `JAMEVENT_UNKNOWN`, delivers nothing, returns false |
| `OnJamEvent` | `FJamEventSignature(FName Event, const FJamEventPayload& Payload)` | the delegate consumers bind |
| `IsKnownEvent` | `static bool IsKnownEvent(FName)` | |
| `GetKnownEvents` | `static TArray<FName>` | the ten names in the order of the enum below |
| `GetBroadcastCount` | `int32` | events delivered since the game instance started |
| `bLogEvents` | `bool`, default true | |

Reach it from Blueprint with "Get Game Instance Subsystem (JamEventBus)"; from C++ `GetGameInstance()->GetSubsystem<UJamEventBus>()`.

### `FJamEventPayload` (USTRUCT)

| Field | Type | Unit or range | Required | Meaning |
|---|---|---|---|---|
| `Who` | int32 | -1, or 0..N-1 | yes (default -1) | skater index in `game.teams` order, team 0's skaters first; -1 = no skater |
| `Where` | FVector | Unreal units (cm), world space | yes | where it happened |
| `Strength` | float | 0..1 | yes (default 0) | event-specific, table below |

The payload is a C++ struct with defaults, not JSON; "absent" here means the default value given above, stated per event.

## Enum strings

### `JamEvent` (C++ constants `JamEvents::*`)

| Event | Constant | When | `Who` | `Where` | `Strength` |
|---|---|---|---|---|---|
| `shot` | `Shot` | a shot leaves the stick | shooter | puck position | exit speed / `game.puck.max_speed` |
| `board` | `Board` | the puck hits the boards | -1 | contact point | normal impact speed / `game.puck.max_speed` |
| `goal` | `Goal` | the puck enters a goal trigger | last skater to touch the puck, or -1 | puck position | 1 |
| `faceoff` | `Faceoff` | a faceoff reset ends and play resumes | -1 | centre ice | 0 |
| `check` | `Check` | a body check makes contact | the checker | contact point | closing speed / `game.skater.speed` (clamped to 1) |
| `knockdown` | `Knockdown` | a skater is knocked down | the skater knocked down | that skater's position | same as the `check` that caused it |
| `pickup_spawn` | `PickupSpawn` | a pickup appears | -1 | pickup position | 0 |
| `pickup_take` | `PickupTake` | a skater takes a pickup | the taker | pickup position | 0 |
| `win` | `Win` | the human player's team wins | -1 | centre ice | 1 |
| `lose` | `Lose` | the human player's team loses | -1 | centre ice | 1 |

Unknown values are errors. The pickup's kind is not in the payload (v1); see open question 1.

## Asset names and prefixes

| Asset name | Kind | Folder | Used by |
|---|---|---|---|
| `NS_shot`, `NS_board`, `NS_goal`, `NS_faceoff`, `NS_check`, `NS_knockdown`, `NS_pickup_spawn`, `NS_pickup_take`, `NS_win`, `NS_lose` | Niagara system | `Game/Plugins/JamAssets/Content/VFX/` | `BP_EventFx` |
| `MS_shot`, `MS_board`, `MS_goal`, `MS_faceoff`, `MS_check`, `MS_knockdown`, `MS_pickup_spawn`, `MS_pickup_take`, `MS_win`, `MS_lose` | MetaSound source | `Game/Plugins/JamAssets/Content/Audio/` | `BP_EventFx` |
| `BP_EventFx` | Blueprint actor or component that binds `OnJamEvent` | `Game/Plugins/JamAssets/Content/VFX/` | placed or spawned in `Rink` (open question 2) |

The suffix after `NS_` / `MS_` is the event name exactly (lower case with underscore). An event may have no `NS_` or no `MS_`; a missing asset is
not an error, a misspelled one is. The binder finds assets by the rule `NS_<event>` and `MS_<event>`.

## Folders

| Folder | Written by | Read by |
|---|---|---|
| `Game/Source/JamCore/Public/` (`JamEventBus.h`) | A | A, B |
| `Game/Plugins/JamAssets/Content/VFX/` | B | B's binder |
| `Game/Plugins/JamAssets/Content/Audio/` | B | B's binder |

## Data asset names

None.

## Acceptance

- [x] `JamEventBus.h` compiles; in PIE, `Broadcast("goal", {Who 2, Where (100,200,0), Strength 0.5})` returns true and logs
      `JAMEVENT goal who=2 where=(100.0,200.0,0.0) strength=0.500 frame=...`; `Broadcast("nope", ...)` returns false and logs `JAMEVENT_UNKNOWN` (H4, lane A0).
- [ ] B: in PIE, broadcasting each of the ten events from a test call spawns its `NS_` and plays its `MS_` (where they exist).
- [ ] A: the bot test logs at least one `shot`, `board`, `goal` and `faceoff` per match.

## Version history

- v1 (H4, lane A0): first version, with the compiled bus and logger.

## Open questions

1. `pickup_spawn` / `pickup_take` do not say which kind; proposal for v2: add `FName Kind` to the payload (`NAME_None` for other events).
2. Where `BP_EventFx` lives at runtime: placed in `Rink` (A owns the map) or spawned by the game mode from a class path. Proposal: the game mode spawns it from a soft class path B names here.
3. `frame` in the log line is the engine frame counter (`GFrameCounter`), not the simulation frame; the skater lane should switch it to the simulation frame once that exists.

## Additions (Director, H6, from R-0007 and lane A12)
| Event | When | Who | Where | Strength | Kind |
|---|---|---|---|---|---|
| `bark` | the bark system picks a line | speaker skater index | speaker | 0 | the situation (`goal_for`, `goal_against`, `check_landed`, `check_received`, `steal`, `stall`, `near_miss`, `faceoff`, `win`, `lose`, `directors_call`, `signal_ack_*`, `praise_reply`) |
| `signal` | the human issues a team signal (keys 1-4, d-pad, later voice) | the human's skater | human | frames / 600 | `defend` / `space` / `pass` / `praise` |
The bark text itself travels in the bark system's own side channel, not in the payload. The `TextToSpeech` engine plugin (Flite, offline) is enabled for `jam.BarkVoice 1`.
