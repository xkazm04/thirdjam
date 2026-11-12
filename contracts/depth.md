# C-0005: depth on P2, the data primitives the Designer composes (Director, H7, 2026-11-12 12:55)

Operator's direction: "each dimension we touched is in very basic game design and gameplay state ... we will need to master game on P2 where we will have most control over the animations
and game - with high probability we can let players to design custom boosts, rules and aspects of the game, while in parallel making effort to upgrade into P3 and P4". Four lanes build
in parallel against this contract (A14 effects, A15 rules and arena, A16 puck control and feel, A17 cuts as progression). Every name below is wire-level; a lane that needs another name
proposes it in its report and uses a clearly temporary one until ruled. Absent value = omit the key. Durations in `_frames`. Unknown keys are errors. All randomness from the sim's seeded stream.

## 1. Ownership for this fan-out (adds to `ownership.json`, all device A)
| Lane | Owns | Does not touch |
|---|---|---|
| A16 feel | `Game/Source/JamCore/Public/JamSim.h`, `Private/JamSim.cpp`, `JamPawns.*`, `schema/game.schema.json` fields `shot.*`, `puck.*`, `skater.*` | effects, rules, zones |
| A14 effects | `Game/Source/JamCore/Effects/**` (new), schema `game.effects`, `game.pickups[]` extension, `prompts/` effect docs | `JamSim.*` except through the seam |
| A15 rules and arena | `Game/Source/JamCore/Rules/**` (new), `Game/Source/JamGame/Private/RinkActor.cpp` zones and obstacles rendering, schema `game.rules.hooks`, `game.rink.zones`, `game.rink.obstacles` | `JamSim.*` except through the seam |
| A17 progression | `Game/Content/Cartridges/ladder/**`, `Game/Source/JamGame/Progression*`, `Game/Source/JamGame/Private/MenuFlow.cpp` one entry "Play the season", `docs/experiments/LADDER.md` | sim, schema |
Lane A16 lands the two seams below **first** (within 30 minutes, as compilable stubs that do nothing), then continues with feel; A14 and A15 build against the seam headers.

## 2. Seam A: modifiers (`Game/Source/JamCore/Public/JamModifiers.h`, owned by A16, consumed by A14)
```
enum class EJamStat : uint8 { SkaterSpeed, SkaterAccel, SkaterTurnRate, SkaterGrip, SkaterMass, ShotSpeed, ShotAimConeDeg, ShotReach, CheckSpeedMin, KnockdownFrames,
                              PuckDrag, PuckMaxSpeed, PuckRadius, IceFriction, BoardRestitution, Count };
enum class EJamTarget : uint8 { Self, OwnTeam, OtherTeam, Everyone, Puck, Rink };
struct FJamStatModifier { EJamStat Stat; float Multiply = 1.f; float Add = 0.f; };
struct FJamActiveModifier { int32 Id; EJamTarget Target; int32 SourceSkater; /* -1 none */ int32 Team; /* -1 none */ TArray<FJamStatModifier> Stats; int32 FramesLeft; FName Kind; };
class IJamModifierSource { virtual void CollectModifiers(int32 Frame, TArray<FJamActiveModifier>& Out) const = 0; };
```
The sim resolves, once per frame before integration, `effective(stat, skater) = base * prod(Multiply) + sum(Add)` over modifiers whose target covers the skater (or the puck / rink),
clamped to the schema range for the stat. The sim exposes `UJamSim::SetModifierSource(IJamModifierSource*)`, `GetEffectiveStat(EJamStat, int32 Skater)` and includes active modifier ids
in `state_hash` (so a modifier changes the hash deterministically). Lane A14 owns the only `IJamModifierSource`: `UJamEffectSystem`.

## 3. Seam B: rule hooks (`Game/Source/JamCore/Public/JamRuleHooks.h`, owned by A16, consumed by A15)
```
enum class EJamRuleTrigger : uint8 { OnGoal, OnPeriodStart, OnPeriodEnd, OnCheck, OnKnockdown, OnPickupTake, OnFaceoff, OnFrame /* every N frames */, OnScoreReached };
enum class EJamRuleAction : uint8 { AddScore, SetScoreMultiplier, EndPeriod, EndMatch, ShrinkRink, GrowRink, SetParam, SpawnPickup, SwapNets, Announce, ApplyModifier };
struct FJamRuleEventIn  { EJamRuleTrigger Trigger; int32 Frame; int32 Who; int32 Team; FName Kind; float Value; };
struct FJamRuleActionOut { EJamRuleAction Action; int32 Team; float Value; FName Param; FName Kind; int32 Frames; FString Text; };
class IJamRuleHook { virtual void OnRuleEvent(const FJamRuleEventIn& In, TArray<FJamRuleActionOut>& Out) = 0; };
```
The sim calls the hook at each trigger and applies the actions in order within the same frame; `ShrinkRink`/`GrowRink` scale `rink.length_m/width_m` by `Value` (clamped to the schema
range) and rebuild the rink geometry at the next faceoff (never mid-play); `SetParam` sets a tunable by key through the same path as Director's Call; `Announce` broadcasts event `announce`
(new C-0003 name: Who -1, Kind = "rule", text in the side channel) which the HUD shows on the scoreboard line. Lane A15 owns the only `IJamRuleHook`: `UJamRuleSystem`.

## 4. Cartridge additions (schema owner per lane; `schema/README.md` and `DEFAULTS.md` updated by the owner; `gen-schema-data` and `gen-parity` re-run by whoever changes the schema last in a window)
```
game.effects: [ { id, kind, target: "self|own_team|other_team|everyone|puck|rink", duration_frames, stats: [ { stat: <EJamStat snake_case>, multiply, add } ],
                 trigger: "pickup|goal_for|goal_against|check_landed|check_received|period_start|zone_enter|zone_exit|director", stack: "refresh|extend|ignore", icon } ]   (A14; max 12)
game.pickups[]: { kind: <effect id or built-in speed|precision|heavy_puck|big_puck|reverse_other>, spawn_every_frames, duration_frames, magnitude, zone?: <zone id>, max_on_ice }   (A14 extends A4's)
game.rules.hooks: [ { id, when: <EJamRuleTrigger snake_case>, every_frames?, score?, team?: "scoring|conceding|any", then: [ { do: <EJamRuleAction snake_case>, team?, value?, param?, kind?, frames?, text? } ] } ]   (A15; max 8)
game.rules.variants: { sudden_death: bool, last_goal_double: bool, mercy_goals?, period_count?, shrink_per_goal? }   (A15; sugar expanded into hooks by the loader)
game.rink.zones: [ { id, shape: "rect|circle", x, z, w?, d?, r?, kind: "slow_ice|fast_ice|bumper|no_shoot|boost_only", strength, visible: bool } ]   (A15; max 8)
game.rink.obstacles[]: existing `box|pillar` plus `bumper` (restitution 1.6) and `goal_shield` (A15)
game.shot: { ..., charge_frames_max?, charge_speed_bonus?, deflect?: bool }   game.puck: { ..., carry_assist? (0..1), count? (1..3) }   game.skater: { ..., pass_speed?, pass_lead_frames? }   (A16)
game.ladder: { id, takes: [ { title, cartridge: <preset id>, twist_text, unlock: "always|previous_won" } ] }   (A17; lives in a separate ladder cartridge, not in play cuts)
```
Designer: A14 and A15 add their objects to `DESIGNER_GAME_EDITABLE` with limits and a capability note in `prompts/designer.system.md` ("you may invent effects and rule hooks from these
parts; here are three examples"). The Designer may **not** change `ladder`.

## 5. Feel (A16), decided
Pass button (gamepad X / keyboard E): a lead pass to the best teammate (open lane, ahead of the puck direction), `pass_speed` default 14, `pass_lead_frames` 20; dribble: `carry_assist` 0.6
pulls the puck toward the stick socket when within reach and the skater turns less than 90 deg/s; charge shot: hold shoot up to `charge_frames_max` 30 for +`charge_speed_bonus` 8 m/s with a
visible wind-up (`IJamSkaterVisual::PlayShoot` gets a charge 0..1 parameter: **contract change C-0001**, A16 updates the interface and the toy figure's call site); deflections: a shot hitting
a skater within 0.6 m of its stick deflects with 70 % speed; checks: a check that lands within `check_speed_min` of the carrier always strips the puck. Every change re-runs `Jam.Bot` and the A3 sweep; the curve must hold within 5 points or A16 reports the delta with the numbers.

## 6. Events added to C-0003
`announce` (rule text on the scoreboard), `effect_start` / `effect_end` (Who = target skater or -1, Kind = effect id), `zone_enter` / `zone_exit` (Who, Kind = zone id), `pass` (Who = passer, Strength = speed).
