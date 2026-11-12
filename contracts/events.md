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

## Additions (C-0005, lane A16 registered on the bus, H7)
| Event | When | Who | Where | Strength | Kind |
|---|---|---|---|---|---|
| `announce` | a rule hook action `Announce` | -1 | zero | 0 | `rule` (the text rides the side channel `FJamSimEvent::Text`; the HUD shows it on the scoreboard line) |
| `pass` | a lead pass leaves the stick (lane A16) | the passer | puck position | pass speed / `game.puck.max_speed` | none |
| `effect_start` | an effect becomes active (lane A14) | target skater, or -1 | that skater (or zero) | 0 | the effect id |
| `effect_end` | an effect expires (lane A14) | target skater, or -1 | that skater (or zero) | 0 | the effect id |
| `zone_enter` | a skater enters a rink zone (lane A15) | the skater | the skater | 0 | the zone id |
| `zone_exit` | a skater leaves a rink zone (lane A15) | the skater | the skater | 0 | the zone id |
Constants: `JamEvents::Announce`, `Pass`, `EffectStart`, `EffectEnd`, `ZoneEnter`, `ZoneExit`. Asset names follow the rule: `NS_pass`, `MS_pass`, ... .

## Additions (R-0013, lane A16 registered on the bus, H7.5): the stick and the faceoff
| Event | When | Who | Where | Strength | Kind |
|---|---|---|---|---|---|
| `poke` | a poke check (left mouse / RB) | the poker | the blade | 1 when it took the puck from an opponent, else 0 | `clean` (took the puck) / `miss` (the window ran out next to a carrier: the poker stumbles) |
| `lift` | a stick lift (right mouse / LB) | the lifter | the carrier (clean) or the blade (miss) | 1 clean, 0 miss | `clean` / `miss` |
| `foul` | a body check from behind or on a skater without the puck, with `rules.penalties` on | the fouler | contact point | the check strength | `foul_from_behind` / `foul_no_puck` |
| `faceoff` | now carries a Kind: the puck goes live at the drop, then the draw resolves | `drop`: -1; `won` / `lost`: the centre | the dot | 0 | `drop` / `won` / `lost` |
A faceoff therefore emits up to three `faceoff` events (drop, won, lost) within `UJamSim::DrawWindowFrames` of each other. Consumers that count faceoffs should count `Kind == drop`.
| `shot_blocked` | a shot refused inside a `no_shoot` zone (R-0019) | the shooter | the puck | 0 | the zone id |
Constants: `JamEvents::Poke`, `Lift`, `Foul`, `ShotBlocked`.

## Proposed by B, accepted (B-to-A-0012, lane A21, H9.3): the `Variant` input, and what the binder sets
Where an event's `Kind` changes the picture or the sound, B makes **one asset with one extra input** instead of an asset per kind: Niagara user parameter **`User.Variant`**
(float) and MetaSound float input **`Variant`**, default 0 (a binder that sets nothing gets the first kind). `UJamEffectsBinder::VariantFor(Event, Kind)` is the one mapping:

| Event | `Kind` -> `Variant` |
|---|---|
| `poke`, `lift` | `clean` 0, `miss` 1 |
| `faceoff` | `drop` 0, `won` 1, `lost` 2 |
| `foul` | `foul_from_behind` 0 (single whistle), `foul_no_puck` 1 (double whistle); 2 (long whistle with a buzzer) is free for a harder foul if a ruling adds one |
| `signal` | `defend` 0, `space` 1, `pass` 2, `praise` 3 (the order of `EJamTeamIntent` in `SignalIntent.h`, confirmed on A's side) |
| every other event | 0 |

The binder also: plays `bark`, `signal` and `announce` as **2D** sounds (not at `Where`); for `zone_enter` / `zone_exit` (Kind = zone **id**) sends the zone **kind's** colour as
`User.Colour`: `slow_ice` (0.35, 0.65, 1.0), `fast_ice` (0.2, 1.0, 0.8), `bumper` (1.0, 0.35, 0.1), `no_shoot` (1.0, 0.1, 0.2), `boost_only` (0.9, 0.8, 0.2) (the same colours
are on B's `MI_Zone_<kind>`); for `pickup_spawn` / `pickup_take` / `effect_start` / `effect_end` sends the pickup kind's colour (`speed` (0.1, 0.9, 1.0), `precision` (1.0, 0.2, 0.9),
`heavy_puck` (0.55, 0.3, 1.0), `big_puck` (1.0, 0.75, 0.1), `reverse_other` (1.0, 0.15, 0.1), custom and unknown white); `pass` sends `Strength` = speed / `game.puck.max_speed` (the sim
already does); `shot_blocked` reuses `NS_foul` / `MS_foul` at Variant 0 until B makes `NS_shot_blocked` / `MS_shot_blocked` (the binder looks for those first); `period` is silent.
Asset names, as on `dev-b`: `NS_` / `MS_` for `poke`, `lift`, `foul`, `faceoff`, `pass`, `effect_start`, `effect_end`, `zone_enter`, `zone_exit`; `MS_announce`, `MS_bark`, `MS_signal`
(sound only); `NS_pickup_idle` (the pickup actor, `User.Colour`); `NS_blade_spray` (the toy figure, `User.Strength` = |lean| / 35 deg, 0..1). `lose`, `bark`, `signal` and
`announce` are **sound-only by design** (`UJamEffectsBinder::IsSoundOnlyEvent`): the binder skips their Niagara lookup, so a bot match logs no "missing NS_" line for them
(Director, H9.4). A custom effect id on `effect_start` / `effect_end` gets a white burst: colouring it by the effect's `kind` needs the kind in the payload, a C-0003 addition for the Director. (The Director signs this in a ruling.)

## Proposed, for the Director (lane A32, B-to-A-0026 ask 3, H12.9): the commentator's seven events
Registered on the bus as `JamEvents::Save`, `Post`, `Breakaway`, `Carry`, `PeriodEnd`, `Clock`, `SeasonTake`; the first six are emitted by `UJamSim` from state it already
has, `season_take` by `AHokejGameMode` through `UJamCommentarySystem::OnMatchBuilt`. All are read-outs: the state they keep is not hashed and nothing in play reads them
(the Determinism test and `JAMSIM hash=` are unchanged). Definitions and thresholds in `docs/experiments/COMMENTARY.md` section 4.
| Event | When | Who | Where | Strength | Kind |
|---|---|---|---|---|---|
| `save` | a shot by the other team left within 90 frames, the puck's line still crosses the attacked goal line inside the mouth plus a metre, and the defender about to touch it (an aimed blade or the body) is its team's last man; once per shot | the blocker | the puck | the shot's Strength | none |
| `post` | the puck hits a goal post at 3 m/s or more, at most once per 20 frames | -1 | the post | puck speed / `game.puck.max_speed` | none |
| `breakaway` | the carrier has every standing opponent behind it, the net 5 to 45 m ahead, and skates at it from centre ice or beyond; once per rush | the carrier | the carrier | gap to the nearest defender / 10 m | none |
| `carry` | one skater has carried the puck for 300 consecutive frames; once per possession | the carrier | the puck | metres carried / rink length | `long`, or `coast` (from the own net area past the far blue line) |
| `period_end` | a take ends and the match goes on | -1 | zero | 0 | `take_<n>` of the take that ended |
| `clock` | 600 frames or fewer left in a timed take (once per take); a tie at the horn plays on | -1 | zero | 0 | `last_10s` / `sudden_death` |
| `season_take` | a match is built whose cartridge is the running season take, after `period` and before the first frame | -1 | zero | 0 | `level_<n>` (1-based ladder index) |
Assets by the rule: `MS_post` exists on `dev-b` and plays through the binder; `NS_` / `MS_` for the other six are B's to make or to declare silent (`UJamEffectsBinder::IsSilentEvent`).
