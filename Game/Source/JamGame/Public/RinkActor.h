// The rink, built procedurally from game.rink at load (M0 greybox, M3b arena manipulation): ice, boards 1.2 m, glass 0.9 m above,
// emissive trim on the board top, two nets (goal_width x goal_height (0.61 by default, R-0033) x 1.5 deep) at x = +-(length/2 - 4), goal trigger boxes behind
// the lines (cosmetic: the simulation decides goals), centre and faceoff markings, obstacles. Geometry only: gameplay collision
// lives in UJamSim. Stencils and device B's /JamAssets/Look instances per R-0002 (B-to-A-0005). Fallback materials are placeholders under /Game/Placeholders (M_Ice, M_Boards, M_Net, M_Trim) with the contract's
// parameter names (Wet, Scuff, Emissive, TeamColour); device B's masters drop in by replacing those assets or by SetMaterials.
// Lane A25 (R-0024 point 1, LOOK-LAB.md recommendation 6): the construction layer on top of the greybox: a 25 cm kick plate at the board foot, a dark glass rail with
// stanchions every 2.4 m, a 14 cm net frame with a back frame and braces, goal-light housings, benches and a penalty bench, and the ice markings B's paint does not
// carry (faceoff circles with hash marks, dots, creases, trapezoids). Visual only: the sim's boards, nets and goal line are untouched. `jam.rink.construction 0` hides it.
// Lane A103 (track F3, the arena in Blender): the benches and the penalty boxes as static meshes made by tools/blender/arena/arena.py and imported to /Game/Arena
// (two team benches outside the +y boards where A25's code benches stand, two penalty boxes outside the -y boards at x = -3 and +3, where UJamSim::BoxPos pins a
// boxed skater). Device B's instances by reference on the five slots. Shown on the dressed tiers, hidden on the bare ones; `jam.rink.arena 0` or a missing mesh
// gives back A25's code benches. Visual only: no collision, no sim state. docs/experiments/ARENA.md.
// Lane A127 (track F3, evolution 3): the arena answers the game. On a `goal` event the scorer's table's lamp and the goal light behind the scored net flash and
// the scored net's cords shake; on a hard skater `board` event the bench, box or table behind that stretch of boards rocks. The rink listens to UJamEventBus
// itself (BeginPlay) and counts the bus's sim frames, never wall clock. Presentation only: parameters on dynamic material instances and component transforms,
// no physics body, nothing the sim reads. `jam.rink.react 0` (or `jam.rink.arena 0`) gives back the still arena.
// Lane A136 (track F3, evolution 4): the whole boards answer a hit. A clay cap (SM_Arena_BoardCap, 24 triangles) stands on the board top at every stanchion
// place round the ring, all in one instanced component; a hard skater `board` event anywhere rocks the nearest cap (a tilt away from the rink and a hop) and
// its neighbours less and later. `jam.rink.caps 0` takes the caps away. The scored end's lamp light hangs over the net instead of beside the lamp.
// Lane A145 (track F3, evolution 5): the hit is seen and the box is used. A hard board hit throws a short flash of light at the hit (`jam.rink.react.hit`,
// candelas at Strength 1; 0 = the quiet hit of evolution 4); the caps are domes in the team colours (blue on x < 0, orange on x > 0; the 24-triangle lump
// is the far level, taken on a rink whose caps would pass the triangle budget as domes); a goal runs round the scored end's caps as a wave with two lights
// that fill the lamp's troughs (`jam.rink.react.wave`, 0 = none); on a `foul` the fouler's team's penalty box opens its door, its clock's hand runs one
// turn with the penalty's frames, and the door opens again at the end. Presentation only, counted on the bus's sim frames.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JamCartridge.h"
#include "JamEventBus.h"
#include "RinkActor.generated.h"

class UProceduralMeshComponent;
class UBoxComponent;
class UStaticMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UPointLightComponent;
class UInstancedStaticMeshComponent;

UCLASS()
class JAMGAME_API ARinkActor : public AActor
{
	GENERATED_BODY()

public:
	ARinkActor();

	// Rebuilds everything from the config (safe to call again after a Director's Call changes the rink).
	UFUNCTION(BlueprintCallable, Category = "Jam|Rink")
	void Build(const FJamGameConfig& Game);

	// Placeholder material paths; a missing asset falls back to the engine's basic shape material.
	static constexpr const TCHAR* IceMaterialPath = TEXT("/Game/Placeholders/M_Ice.M_Ice");
	static constexpr const TCHAR* BoardsMaterialPath = TEXT("/Game/Placeholders/M_Boards.M_Boards");
	static constexpr const TCHAR* NetMaterialPath = TEXT("/Game/Placeholders/M_Net.M_Net");
	static constexpr const TCHAR* TrimMaterialPath = TEXT("/Game/Placeholders/M_Trim.M_Trim");

	static constexpr double BoardHeightM = 1.2;
	static constexpr double GlassHeightM = 0.9;
	static constexpr double BoardThicknessM = 0.2;
	// Lane A64: the emissive fill B's MI_Toon_Bumper may carry at most (presentation only; the toon ramp needs the light to show the round).
	static constexpr float BumperEmissiveCap = 0.8f;
	// Lane A25: the construction's fixed sizes (they do not scale with the rink; the benches do).
	static constexpr double KickPlateHeightM = 0.25;
	static constexpr double KickPlateProudM = 0.02;
	static constexpr double StanchionSpacingM = 3.6;   // 2.4 read as a black comb at 24 m (eyes tool, EYES.md finding 2; Director 17:17)
	static constexpr double NetBarM = 0.14;
	static constexpr double NetBackHeightM = 0.9;
	// Lane A103: the Blender-made arena dressing. The bench mesh is built 12 m long (0.1 x a 120 m rink) and scaled along the boards to 0.1 x the rink's length;
	// the penalty box is 4.4 m on every rink, centred where the sim boxes a skater (UJamSim::BoxPos: x = -3 team 0, +3 team 1, on the -y side).
	static constexpr const TCHAR* TeamBenchMeshPath = TEXT("/Game/Arena/SM_Arena_TeamBench.SM_Arena_TeamBench");
	static constexpr const TCHAR* PenaltyBoxMeshPath = TEXT("/Game/Arena/SM_Arena_PenaltyBox.SM_Arena_PenaltyBox");
	static constexpr double ArenaBenchBuiltLengthM = 12.0;
	static constexpr double ArenaBenchLengthFactor = 0.1;
	static constexpr double ArenaBenchCentreFactor = 0.0833;
	static constexpr double PenaltyBoxCentreXM = 3.0;
	static constexpr double ArenaGapM = 0.05;   // between the board's outer face and a piece (A25's benches: the same 5 cm)
	// Lane A119 (track F3, evolution 2): the goals and the scorer's table. A goal mesh is built by arena.py for one opening (game.rink.goal_width_m x
	// goal_height_m: its posts' inner faces stand at +-W/2 and its crossbar's underside at H, the sim's goal volume); Build takes the variant nearest the
	// cartridge's goal (the table is in RinkActor.cpp) and scales it W/Wv along the goal line and H/Hv up, which keeps the opening exact. The scorer's table
	// stands between the two penalty boxes at x = 0 on the -y side.
	static constexpr const TCHAR* ScorerTableMeshPath = TEXT("/Game/Arena/SM_Arena_ScorerTable.SM_Arena_ScorerTable");
	// Lane A127 (evolution 3): the goal light, a clay post with a red lamp outside the end boards behind each net (arena.py goal_light(); slots dark, kick, lamp).
	static constexpr const TCHAR* GoalLightMeshPath = TEXT("/Game/Arena/SM_Arena_GoalLight.SM_Arena_GoalLight");
	// The reactions' numbers (sim frames at 60 Hz). The lamp: lit on the goal's frame, pulsing every LampPulseFrames, fading over the last LampFadeFrames, at its
	// resting value again from LampFlashFrames on (the bar: back by frame 120). The net: a damped wobble of the scored net's scale (depth and height) for
	// NetShakeFrames. The rattle: a skater `board` event at RattleMinStrength or more (Strength = normal speed / game.skater.speed) rocks the piece standing
	// within RattleReachM of the hit along the boards about its rink-side foot, RattleMaxDeg at Strength 1, for RattleFrames.
	static constexpr int32 LampFlashFrames = 108;
	static constexpr int32 LampPulseFrames = 18;
	static constexpr int32 LampFadeFrames = 36;
	static constexpr float LampPeakEmissive = 14.f;
	static constexpr int32 NetShakeFrames = 54;
	static constexpr float NetShakeDepth = 0.12f;
	static constexpr float NetShakeHeight = 0.06f;
	static constexpr int32 RattleFrames = 18;
	static constexpr float RattleMinStrength = 0.45f;
	static constexpr float RattleMaxDeg = 3.5f;
	static constexpr double RattleReachM = 2.0;
	// The lamp lights (candelas at the top of a flash = `jam.rink.react.light`, default 3000, picked from a sweep of off-screen stills at 100 / 1,000 / 10,000 /
	// 100,000: 1,000 is a halo and a reflection on clay, 10,000 washes the end zone, 100,000 blows the picture out). The scorer's table's light stands 1.5 m
	// from the boxes' walls and takes TableLightShare of it; on l2_night the same candelas read far stronger against the dark and take NightLightShare.
	// Lane A136 (evolution 4): the goal on p3_clay is as loud as on l2_night. A red light adds only red, and the clay barn's ice is bright and warm: beside the
	// lamp at 3,000 cd it changed 4 % of the game camera's picture. The scored end's light now hangs over the net (EndLightBehindLineM behind the goal line,
	// EndLightHeightM up: it still lights the lamp, and it reaches the crease and the slot) at 6,000 cd: 14.8 % of the picture, with the puck, the figures
	// and the lines still read (two sweeps of off-screen stills with stand-ins, ARENA.md section 31). The table's light and the night tier keep the candelas
	// they had (750 and 1,200 / 300). `jam.rink.react.loud 0` gives back evolution 3's light: beside the lamp, half the cvar, the shares 0.25 and 0.4.
	static constexpr float TableLightShare = 0.125f;
	static constexpr float NightLightShare = 0.2f;
	static constexpr float TableLightShareE3 = 0.25f;
	static constexpr float NightLightShareE3 = 0.4f;
	static constexpr double EndLightBehindLineM = 3.0;
	static constexpr double EndLightHeightM = 3.6;
	// Lane A136 (evolution 4): the board caps. One at every stanchion place (the outline's length / StanchionSpacingM, rounded: never more than 1.8 m along
	// the boards from a hit). A skater `board` event at RattleMinStrength or more rocks the nearest: CapRockDeg of tilt away from the rink and CapHopCm off the
	// board top at Strength 1 on the hit's frame, a damped swing for CapRockFrames; the CapRippleReach caps on each side answer CapRippleDelay frames later
	// per place with 0.55 and 0.25 of it. The whole ripple is over in CapRippleFrames (= RattleFrames). Up to CapRipples hits are live at once.
	static constexpr const TCHAR* BoardCapMeshPath = TEXT("/Game/Arena/SM_Arena_BoardCap.SM_Arena_BoardCap");
	static constexpr int32 CapRockFrames = 14;
	static constexpr int32 CapRippleReach = 2;
	static constexpr int32 CapRippleDelay = 2;
	static constexpr int32 CapRippleFrames = CapRockFrames + CapRippleReach * CapRippleDelay;
	static constexpr float CapRockDeg = 26.f;
	static constexpr float CapHopCm = 22.f;
	static constexpr int32 CapRipples = 4;
	// Lane A145 (evolution 5). The cap's far level and the budget that picks it: Build instances the dome (SM_Arena_BoardCap, 40 triangles) while caps x its
	// triangles stay within CapTriangleBudget, else the lump (an instanced component draws ONE level for all its instances, so the level is chosen per rink).
	static constexpr const TCHAR* BoardCapFarMeshPath = TEXT("/Game/Arena/SM_Arena_BoardCapFar.SM_Arena_BoardCapFar");
	// Lane A160 (evolution 6): 6,000, so every shipped rink takes the dome (90 x 45: 73 x 40 = 2,920; 120 x 60: 98 x 40 = 3,920; 140 x 70: 115 x 40 = 4,600;
	// 3,000 left the two big rinks with the lump, which reads as a tent). The far level stays for a rink past 150 caps.
	static constexpr int32 CapTriangleBudget = 6000;
	// The hit flash: a shadowless point light at the hit's place (HitLightInM into the rink along the board's normal, HitLightHeightM up), `jam.rink.react.hit`
	// candelas x Strength on the hit's frame, falling to nothing in HitFlashFrames (a square fall: most of it is gone in six frames). One light per ripple.
	static constexpr int32 HitFlashFrames = 12;
	static constexpr double HitLightInM = 0.6;
	static constexpr double HitLightHeightM = 1.7;
	static constexpr float HitLightRadiusCm = 450.f;      // 9 m washed a fifth of the picture; 4.5 m keeps the flash a pool at the hit
	// The goal wave: in every pulse of the lamp (LampPulseFrames) a front leaves the goal light and runs WaveReachM along the end boards both ways; a cap
	// hops WaveCapShare of a full rock as it passes, and a red light rides each front, WaveLightInM inside the end boards and WaveLightHeightM up, growing
	// from nothing to `jam.rink.react.wave` x the end light's candelas while the lamp's own pulse falls. All of it inside the lamp's LampFlashFrames.
	static constexpr double WaveReachM = 10.0;
	static constexpr double WaveLightInM = 2.0;
	static constexpr double WaveLightHeightM = 3.0;
	static constexpr float WaveCapShare = 0.6f;
	// The box in use: the door (SM_Arena_BoxDoor on the box's socket `Door`) swings DoorOpenDeg open in DoorOpenFrames, stands DoorHoldFrames, closes in
	// DoorCloseFrames; the clock's long hand (SM_Arena_BoxHand on the socket `Hand`) turns 360 degrees in the penalty's frames.
	static constexpr const TCHAR* BoxDoorMeshPath = TEXT("/Game/Arena/SM_Arena_BoxDoor.SM_Arena_BoxDoor");
	static constexpr const TCHAR* BoxHandMeshPath = TEXT("/Game/Arena/SM_Arena_BoxHand.SM_Arena_BoxHand");
	static constexpr int32 DoorOpenFrames = 10;
	static constexpr int32 DoorHoldFrames = 50;
	static constexpr int32 DoorCloseFrames = 14;
	static constexpr int32 DoorCycleFrames = DoorOpenFrames + DoorHoldFrames + DoorCloseFrames;
	static constexpr float DoorOpenDeg = 72.f;      // arena.py P["door_open_deg"]: how the door stood when it was part of the box mesh
	// Lane A160 (evolution 6): the box seen from the ice. A box in use is lit from inside for the penalty's frames (`jam.rink.react.box` candelas, 0 = the
	// dark box of evolution 5): one shadowless light a box, BoxLightHeightM over the floor, BoxLightTowardsRinkM from the seat towards the boards, so the
	// boxed figure's rink side, the back wall and the clock are lit. It fades in and out over BoxLightFadeFrames.
	// Build 2 had it 2.2 m up, 0.7 m towards the boards, 4.2 m wide: it threw a pool on the ice inside the boards (the stills). Over the seat, 1.9 m up
	// and 2.5 m wide, the nearest ice is 2.2 m away at the radius's edge: the figure, the back wall and the clock are lit, the ice is not.
	static constexpr double BoxLightHeightM = 1.9;
	static constexpr double BoxLightTowardsRinkM = 0.0;
	static constexpr float BoxLightRadiusCm = 250.f;
	// Lane A160: the goal's lights at device B's numbers (see `jam.rink.react.light`): 3.5 m unless the goal is loud; the quiet wave lights ride the
	// boards' inner face instead of 2 m inside the rink.
	static constexpr float LampLightRadiusCm = 350.f;
	static constexpr float LampLightRadiusLoudCm = 1600.f;
	static constexpr float WaveLightRadiusLoudCm = 1400.f;
	static constexpr double WaveLightInQuietM = 0.05;
	static constexpr int32 BoxLightFadeFrames = 12;
	// Lane A160: the net's bulge starts on the goal's own frame, with device B's NS_goal (it was at rest on that frame and first moved the frame after).
	static constexpr int32 NetShakeLeadFrames = 1;
	// Lane A160: the goal wave's first front leaves from the posts, not from the middle of the net: the caps within WaveStartHalfM of the middle hop on
	// the goal's own frame on every rink (a cap stands every 3.6 m at most, so one always does).
	static constexpr double WaveStartHalfM = 2.0;

	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UProceduralMeshComponent> Ice;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UProceduralMeshComponent> Markings;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UProceduralMeshComponent> Boards;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UProceduralMeshComponent> Glass;
	// R-0002: 10 cm emissive strips on the board tops, blue half (x < 0, MI_Trim_Blue) and orange half (x > 0, MI_Trim_Orange).
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UProceduralMeshComponent> Trim;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UProceduralMeshComponent> TrimOrange;
	// R-0002: far-net beacon posts behind the +x and -x nets (MI_Trim_Beacon).
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UStaticMeshComponent> BeaconPos;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UStaticMeshComponent> BeaconNeg;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UProceduralMeshComponent> Nets;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UProceduralMeshComponent> Obstacles;
	// C-0005 / A15: flat zone decals on the ice (game.rink.zones), one section pair (fill, rim) per zone kind. Fallback only since
	// B-to-A-0016 (lane A21): a zone whose /JamAssets/Look/MI_Zone_<kind> exists is drawn as a ZoneQuads entry instead.
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UProceduralMeshComponent> Zones;
	// B-to-A-0016: one engine plane per visible zone with a dynamic MI_Zone_<kind> (scalars Circle, Strength), 0.2 cm above the ice,
	// no shadow, no collision, stencil 0; scaled to (w, d, 1) m or (2r, 2r, 1) m so the material reads the zone's size from the UVs.
	UPROPERTY(Transient, VisibleAnywhere, Category = "Jam|Rink") TArray<TObjectPtr<UStaticMeshComponent>> ZoneQuads;
	// Lane A25: the construction layer. KickPlate: the 25 cm ochre band at the foot of the inner board face (section 0). GlassRail: the dark rail on top of the
	// glass (section 0) and the stanchions between the panels (section 1). GoalLights: a housing on the end-board rail behind each net (section 0) with a red
	// lens (section 1). Benches: two team benches and a penalty bench outside the +y boards, walls (section 0) and seats (section 1). All stencil 1, no collision.
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UProceduralMeshComponent> KickPlate;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UProceduralMeshComponent> GlassRail;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UProceduralMeshComponent> GoalLights;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UProceduralMeshComponent> Benches;
	// Lane A103: the Blender-made benches (+y side; Neg = x < 0, team 0, blue; Pos = x > 0, team 1, orange) and penalty boxes (-y side, turned 180 degrees).
	// Stencil 1, no collision. The mesh is loaded at Build; without it the component stays empty and hidden and A25's code benches show.
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UStaticMeshComponent> ArenaBenchNeg;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UStaticMeshComponent> ArenaBenchPos;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UStaticMeshComponent> ArenaBoxNeg;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UStaticMeshComponent> ArenaBoxPos;
	// Lane A119: the Blender-made goals (Neg = the -x end, yaw +90; Pos = the +x end, yaw -90: the mesh's +y runs into the net) and the scorer's table.
	// While the goals show, the code frame and its sheet netting (`Nets`) are hidden; `jam.rink.arena 0`, a bare tier or a missing mesh gives them back.
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UStaticMeshComponent> ArenaGoalNeg;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UStaticMeshComponent> ArenaGoalPos;
	// The goal's net is a mesh of its own on the same transform, WITHOUT the custom depth stencil: on the outlined tiers every inked cord made the net a black
	// comb from the game camera (off-screen engine stills on l2_night, ARENA.md); the frame keeps the stencil and its outline.
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UStaticMeshComponent> ArenaGoalNetNeg;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UStaticMeshComponent> ArenaGoalNetPos;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UStaticMeshComponent> ArenaTable;
	// Lane A127: the Blender-made goal lights outside the end boards (Neg = behind the -x net, Pos = behind the +x net). While they show, A25's code housings
	// on the glass rail (`GoalLights`) are hidden: one light behind each net, never both.
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UStaticMeshComponent> ArenaGoalLightNeg;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UStaticMeshComponent> ArenaGoalLightPos;
	// Lane A127: a red point light over each lamp (0 the scorer's table's, 1 the goal light behind the -x net, 2 behind the +x net): no shadows, hidden and at
	// intensity 0 at rest; during a flash it follows the lamp's curve up to `jam.rink.react.light` candelas, so the boards, the ice and the net behind the
	// scored goal pulse red with the lamp (the first engine stills: a lamp's Emissive alone is a small flat dot from the game camera).
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UPointLightComponent> ArenaLampLights[3];
	// Lane A136: the board caps, every one an instance of SM_Arena_BoardCap (one draw for the ring). Stencil 1, no collision, no shadow. Shown with the pieces.
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UInstancedStaticMeshComponent> ArenaCaps;
	// Lane A145: the caps in the team colours are two components: ArenaCaps holds the caps of the x < 0 half (team 0, B's MI_Toon_Blue), ArenaCapsPos the
	// x >= 0 half (team 1, MI_Toon_Orange). A cap's index (GetCapPlaceM, GetCapTransform) runs round the whole ring as before.
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UInstancedStaticMeshComponent> ArenaCapsPos;
	// Lane A145: the hit flash's lights (one per live ripple) and the goal wave's two (0 runs towards -y, 1 towards +y). Shadowless, hidden at rest.
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UPointLightComponent> ArenaHitLights[4];
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UPointLightComponent> ArenaWaveLights[2];
	// Lane A145: the penalty boxes' doors and clock hands (Neg = team 0's box at x = -3, Pos = team 1's), children of their box so they rock with it.
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UStaticMeshComponent> ArenaBoxDoorNeg;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UStaticMeshComponent> ArenaBoxDoorPos;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UStaticMeshComponent> ArenaBoxHandNeg;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UStaticMeshComponent> ArenaBoxHandPos;
	// Lane A160: the light inside each penalty box (0 team 0's at x = -3, 1 team 1's at x = +3), lit while that box's penalty runs.
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UPointLightComponent> ArenaBoxLights[2];
	// Behind the +x goal line (team 0 scores here) and behind the -x line (team 1 scores here).
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UBoxComponent> GoalTriggerPos;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UBoxComponent> GoalTriggerNeg;

	// The dynamic instances, so look tiers and the Designer can set Wet / Scuff / Emissive / TeamColour.
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Jam|Rink") TObjectPtr<UMaterialInstanceDynamic> IceMid;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Jam|Rink") TObjectPtr<UMaterialInstanceDynamic> TrimMid;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Jam|Rink") TObjectPtr<UMaterialInstanceDynamic> TrimOrangeMid;

	// B-to-A-0018 point 5 (lane A21): the trim strips' Emissive, per tier from ULookTierSwitcher (night 30, clay 12); Build reads it,
	// SetTier pushes a change. Only B's MI_Trim_Blue / MI_Trim_Orange take it; the placeholder keeps its own scale and the beacon stays at 40.
	UFUNCTION(BlueprintCallable, Category = "Jam|Rink")
	void SetTrimEmissive(float Emissive);

	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	int32 GetZoneQuadCount() const { return ZoneQuads.Num(); }

	// Lane A69 (B-to-A-0034 point 3 / 0035 point 4a): on p3_clay each zone that has a ZoneQuad also gets an opaque ring mesh on its outline (6 cm high, 12 cm wide,
	// B's MI_Zone_<kind> on it, so it shows in the ice's reflections); the decal quad is hidden there. Parallel to ZoneQuads (same index). Built with the rink,
	// shown or hidden by SetZoneRingsForTier; there is no per-switch spawn, so a tier switch can leave nothing behind.
	UPROPERTY(Transient, VisibleAnywhere, Category = "Jam|Rink") TArray<TObjectPtr<UProceduralMeshComponent>> ZoneRings;
	UFUNCTION(BlueprintCallable, Category = "Jam|Rink")
	void SetZoneRingsForTier(const FString& Tier);
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	int32 GetZoneRingCount() const { return ZoneRings.Num(); }
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	bool AreZoneRingsShown() const { return bClayTier; }

	// Lane A31 (B-to-A-0023 point 2, 0025): the glass panel's instance per look tier: /JamAssets/Look/MI_Bcast_Glass on p4_broadcast, MI_Glass on every
	// other tier (the net placeholder stands in when B's instance is absent). Build reads game.look.tier; ULookTierSwitcher::SetTier pushes a change.
	UFUNCTION(BlueprintCallable, Category = "Jam|Rink")
	void SetGlassForTier(const FString& Tier);
	// The look name the glass carries right now ("MI_Glass", "MI_Bcast_Glass", or "placeholder"), for the PIE proof.
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	FString GetGlassLookName() const { return GlassLookName; }

	// Lane A25: shows or hides the construction layer (kick plate, rail, net frame extras, goal lights, benches, extra markings) without a rebuild;
	// `jam.rink.construction` drives it on every rink actor, so a ProfileGPU before/after pair is two console commands on one frame.
	UFUNCTION(BlueprintCallable, Category = "Jam|Rink")
	void SetConstructionVisible(bool bVisible);
	static bool IsConstructionEnabled();

	// Lane A103: the arena dressing's switch (`jam.rink.arena`, default 1) and the tier rule. A bare tier is one whose rig C-0002 lists as an empty placeholder
	// (l1_day, l3_neon): nothing lights or dresses the outside of the boards there, so the Blender pieces hide and A25's code benches stay.
	static bool IsArenaDressingEnabled();
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	static bool IsBareTier(const FString& Tier);
	// True when the four Blender pieces are what the rink shows (meshes loaded, construction on, the cvar on, a dressed tier); A25's code benches are hidden then.
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	bool IsArenaDressingShown() const { return bArenaShown; }
	// Lane A119: true when the Blender goals are what stands on the goal lines (the code frame `Nets` is hidden then).
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	bool IsArenaGoalShown() const { return bArenaGoalShown; }
	// Where a boxed skater of Team (0 or 1) would stand inside its penalty box, facing the rink: the box mesh's socket `Seat` in world space.
	// False when the box is not shown (the caller keeps the sim's place on the ice). Presentation only: UJamSim::BoxPos is not changed by this lane.
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	bool GetPenaltyBoxSeat(int32 Team, FVector& OutLocation, FRotator& OutRotation) const;
	// ---- Lane A127 (evolution 3): the arena answers the game ----
	// The pure curves (tests read them). LampAt: 0 outside [0, LampFlashFrames), else 0..1 (1 on the goal's frame). NetShakeAt: 0 outside [0, NetShakeFrames),
	// else a damped sine in -1..1. RattleAt: 0 outside [0, RattleFrames), else a damped cosine times the clamped Strength.
	static float LampAt(int64 FramesSince);
	static float NetShakeAt(int64 FramesSince);
	static float RattleAt(int64 FramesSince, float Strength);
	// `jam.rink.react` (default 1): 0 = the pieces stand and never move or light.
	static bool AreReactionsEnabled();
	// The bus's delivery (what OnJamEvent calls with the bus's sim frame): `goal` starts the lamp, the scored end's goal light and net; a skater `board` at
	// RattleMinStrength or more starts a rattle. Applies the reaction's first frame at once. Does nothing while the Blender pieces are not shown.
	UFUNCTION(BlueprintCallable, Category = "Jam|Rink")
	void ReactToEvent(FName Event, const FJamEventPayload& Payload, int64 SimFrame);
	// Writes every live reaction's value for SimFrame (the actor's tick calls it with the bus's frame; a frame before the reaction's start ends it).
	UFUNCTION(BlueprintCallable, Category = "Jam|Rink")
	void AdvanceReactions(int64 SimFrame);
	// Listens to this bus (BeginPlay passes the game instance's); null stops listening.
	void BindBus(UJamEventBus* InBus);
	// Read-outs for the proofs. The lamp's Emissive as its dynamic instance holds it (the scorer's table), and its resting value; -1 when there is no lamp.
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	float GetLampEmissive() const;
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	float GetLampRestEmissive() const { return LampRest[0]; }
	// The goal light's Emissive behind the +x net (End > 0) or the -x net; -1 when there is none.
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	float GetGoalLightEmissive(int32 End) const;
	// The lamp light's intensity right now in candelas (Index as ArenaLampLights), 0 while it is hidden; and the peak a flash drives it to.
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	float GetLampLightIntensity(int32 Index) const;
	static float LampLightPeak();
	// Lane A136: `jam.rink.react.loud` (default 1): the scored end's light over the net at the cvar's candelas; 0 = evolution 3's light.
	static bool IsGoalLoud();
	// Puts the three lamp lights where the loud switch says (Build and every cvar change call it).
	void PlaceLampLights();
	// The peak of one lamp's light on this rink's tier: the cvar times the table's share (Index 0) and the night tier's share.
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	float LampLightPeakAt(int32 Index) const;
	// The net's scale over its resting scale at the +x end (End > 0) or the -x end ((1, 1, 1) at rest), and the rattled piece's roll in degrees right now.
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	FVector GetNetShakeScale(int32 End) const;
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	float GetRattleDeg() const { return RattleDegNow; }
	// The component rocking right now, or the last one that did ("" before the first).
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	FString GetRattledPieceName() const;
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	int32 GetGoalReactionCount() const { return GoalReactions; }
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	int32 GetRattleCount() const { return RattleReactions; }
	// True while any reaction is live (a lamp lit, a net, a piece or a board cap moving).
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	bool IsReacting() const { return GoalFrame >= 0 || RattleFrame >= 0 || bCapsLive || bBoxLive; }
	// ---- Lane A136 (evolution 4): the board caps ----
	// `jam.rink.caps` (default 1): 0 = no caps on the boards (the arena of evolution 3).
	static bool AreCapsEnabled();
	// The pure curve: 0 outside [0, CapRockFrames), else a damped cosine times the clamped Strength (1 on the hit's frame at Strength 1).
	static float CapRockAt(int64 FramesSince, float Strength);
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	bool AreCapsShown() const { return bCapsShown; }
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	int32 GetCapCount() const { return CapRest.Num(); }
	// Where cap Index stands (rink metres, on the boards' middle line).
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	FVector2D GetCapPlaceM(int32 Index) const { return CapAtM.IsValidIndex(Index) ? CapAtM[Index] : FVector2D::ZeroVector; }
	// Hard hits the caps answered, the cap the last one found and how far that cap stands from the hit (metres, in the plane of the ice).
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	int32 GetCapRockCount() const { return CapRockReactions; }
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	int32 GetRockedCapIndex() const { return LastCapIndex; }
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	float GetRockedCapDistanceM() const { return LastCapDistM; }
	// Cap Index right now: its tilt away from the rink in degrees and its hop off the board top in centimetres (0 at rest).
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	float GetCapTiltDeg(int32 Index) const { const FVector2f* V = CapNow.Find(Index); return V ? V->X : 0.f; }
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	float GetCapHopCm(int32 Index) const { const FVector2f* V = CapNow.Find(Index); return V ? V->Y : 0.f; }
	// Triangles the caps add to the scene (instances x the mesh's triangles).
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	int32 GetCapTriangles() const;
	// ---- Lane A145 (evolution 5): the hit flash, the goal wave, the box in use ----
	// Cap Index's transform right now, relative to the rink (whichever of the two components holds it).
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	FTransform GetCapTransform(int32 Index) const;
	// True when the caps are the dome (the near level), false when the rink took the 24-triangle lump.
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	bool AreCapsRound() const { return bCapsRound; }
	// The flash's curve: 0 outside [0, HitFlashFrames), else the clamped Strength x a square fall (1 on the hit's frame at Strength 1).
	static float HitFlashAt(int64 FramesSince, float Strength);
	// `jam.rink.react.hit`: candelas at Strength 1 (0 = no flash), and what this rink's tier makes of it (l2_night takes NightLightShare).
	static float HitLightPeak();
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	float HitLightPeakHere() const;
	// Light Index's candelas right now (ArenaHitLights / ArenaWaveLights), 0 while it is hidden.
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	float GetHitLightIntensity(int32 Index) const;
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	float GetWaveLightIntensity(int32 Index) const;
	// `jam.rink.react.wave`: the wave lights' share of the scored end's light at the end of a pulse (0 = no wave: no cap hops on a goal).
	static float WaveShare();
	// The door's curve: how far open (0..1) FramesSince after it was asked to open; 1 from DoorOpenFrames - 1, 0 again after DoorCycleFrames.
	static float DoorAt(int64 FramesSince);
	// Lane A160 (evolution 6). The penalty boxes stand with the other pieces on a dressed tier, and ALONE on a bare tier when the cut whistles
	// (game.rules.penalties: Glass House plays on l3_neon): there the boxed figure gets its seat too. Without the whistle they are furniture.
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	bool AreBoxesShown() const { return bBoxesShown; }
	// Candelas of a used box's light on this rink's tier (`jam.rink.react.box`; l2_night takes NightLightShare), and what Team's box light gives now.
	static float BoxLightPeak();
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	float BoxLightPeakHere() const;
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	float GetBoxLightIntensity(int32 Team) const;
	// On a bare tier the Blender goals and the board caps stand too (`jam.rink.arena.bare`, 0 = the code frame and no caps there, as before the lane).
	static bool IsBareDressed();
	// A goal's reactions, measured: frames after the bus's goal frame at which each was first off rest in the last goal (-1 not yet).
	// Which: 0 the lamp and its light, 1 the net's bulge, 2 a cap behind the scored line (the boards' answer), 3 the wave's lights.
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	int32 GetGoalFirstFrame(int32 Which) const { return Which >= 0 && Which < 4 ? GoalSeenAt[Which] : -1; }
	// Team's box right now: the door's swing and the clock hand's turn in degrees (0 = closed / the hand at the top), and the fouls the boxes answered.
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	float GetBoxDoorDeg(int32 Team) const { return BoxDoorDegNow[Team == 0 ? 0 : 1]; }
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	float GetBoxHandDeg(int32 Team) const { return BoxHandDegNow[Team == 0 ? 0 : 1]; }
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	int32 GetBoxReactionCount() const { return BoxReactions; }
	// The penalty's length the boxes count with (game.rules.penalty_frames at the last Build).
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	int32 GetBoxPenaltyFrames() const { return PenaltyFramesCfg; }
	// True when the doors and hands are meshes of their own on the boxes (the box mesh carries the sockets Door and Hand).
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	bool IsBoxUsable() const;
	// The BoxSeat hook's note: a figure of Team was drawn in its box on this bus frame (the live proof logs the frame it leaves).
	void NoteBoxed(int32 Team) const;
	// Serves AJamSkater::BoxSeat (JamCore): a boxed skater's figure is drawn at its team's GetPenaltyBoxSeat by the rink of its own world. BeginPlay calls it.
	static void ServeBoxSeat();

	virtual void Tick(float DeltaSeconds) override;

	// Lane A25 (Director 17:17, eyes finding 3): the far-net beacon column read as noise and as a dithered artefact. `jam.rink.beacon` 0 = off, 1 = B's MI_Trim_Beacon
	// (translucent, as before), 2 = an opaque emissive column (lab flat instance in the trim colour). Build and the cvar sink apply it.
	void ApplyBeaconMode();
	static int32 GetBeaconMode();

	// The rounded-rectangle outline used for the ice and the boards (rink metres, counter-clockwise from +x).
	static void Outline(double LengthM, double WidthM, double CornerM, int32 ArcSegments, TArray<FVector2D>& Out);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

private:
	// Lane A127: the reactions' state. LampMids: 0 the scorer's table's lamp, 1 the goal light behind the -x net, 2 behind the +x net (null = no such lamp or
	// a material without Emissive). The rattle's pieces: bench -x, bench +x, box -x, box +x, table, with the rotation Build gave each.
	UFUNCTION()
	void OnJamEvent(FName Event, const FJamEventPayload& Payload);
	void ResetReactions();
	void ApplyGoalReaction(int64 Since);
	void ApplyRattle(int64 Since);
	UStaticMeshComponent* RattlePiece(int32 Index) const;
	UPROPERTY(Transient) TObjectPtr<UJamEventBus> ReactBus;
	UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> LampMids[3];
	float LampRest[3] = { -1.f, -1.f, -1.f };
	FVector NetRestScale = FVector::OneVector;
	FQuat RattleRestRot[5] = { FQuat::Identity, FQuat::Identity, FQuat::Identity, FQuat::Identity, FQuat::Identity };
	double RattleCentreX[5] = { 0, 0, 0, 0, 0 };     // rink metres
	double RattleHalfLen[5] = { 0, 0, 0, 0, 0 };
	double RinkHalfWidthM = 0.0;
	double GoalLineXM = 0.0;    // lane A136: where the end lights hang from
	int64 GoalFrame = -1;
	int32 GoalEnd = 0;          // +1 the +x net was scored on, -1 the -x net
	int64 RattleFrame = -1;
	int32 RattleIndex = -1;
	float RattleStrength = 0.f;
	float RattleDegNow = 0.f;
	int32 GoalReactions = 0;
	int32 RattleReactions = 0;
	// Lane A136: the board caps' state. CapAtM / CapYawDeg: the stanchion places Build walked (rink metres, the yaw that turns a cap's +y away from the rink);
	// CapRest: each instance's transform at rest; CapNow: the caps off rest right now -> (tilt degrees, hop cm).
	struct FCapRipple { int64 Frame = -1; int32 Index = -1; float Strength = 0.f; };
	// Lane A145: which component holds cap k (0 ArenaCaps, 1 ArenaCapsPos) and at which instance; the boxes' use (0 team 0's, 1 team 1's).
	struct FBoxUse { int64 FoulFrame = -1; int32 Frames = 0; bool bEndLogged = false; };
	void ApplyBox(int64 SimFrame);
	// Lane A160: see AreBoxesShown, GetGoalFirstFrame.
	void NoteGoalTiming(int64 Since);
	bool bBoxesShown = false;
	bool bWhistle = false;
	int32 GoalSeenAt[4] = { -1, -1, -1, -1 };
	bool bGoalTimingLogged = true;
	TArray<uint8> CapSide;
	TArray<int32> CapInst;
	bool bCapsRound = false;
	FBoxUse BoxUse[2];
	float BoxDoorDegNow[2] = { 0.f, 0.f };
	float BoxHandDegNow[2] = { 0.f, 0.f };
	mutable int64 BoxSeenAt[2] = { -1, -1 };
	int64 LastBoxFrame = 0;
	int32 PenaltyFramesCfg = 300;
	int32 TeamSplit = 0;         // skaters of team 0 (the bus names a skater by its index, team 0's first)
	int32 BoxReactions = 0;
	bool bBoxLive = false;
	void ApplyCaps(int64 SimFrame);
	void ResetCaps();
	TArray<FVector2D> CapAtM;
	TArray<double> CapYawDeg;
	TArray<FTransform> CapRest;
	TMap<int32, FVector2f> CapNow;
	FCapRipple CapRipple[CapRipples];
	bool bCapsShown = false;
	bool bCapsLive = false;
	int32 CapRockReactions = 0;
	int32 LastCapIndex = -1;
	float LastCapDistM = 0.f;

	UMaterialInterface* LoadMat(const TCHAR* Path) const;
	UMaterialInterface* LoadLook(const TCHAR* LookName, const TCHAR* FallbackPath, bool* bOutFromLook = nullptr) const;
	// Lane A25: B's /JamAssets/Look/<LookName> first; else lane A23's /Game/LookLab/<LabName> proposal with one log line; else the basic shape material tinted.
	// LabEmissive >= 0 lifts the lab instance's Emissive in a dynamic copy (the flat lab master has no ambient floor); -1 uses the instance as it is.
	// bLabTint also writes Tint into that copy (the lab instance's own colour is replaced: benches, goal-light housings).
	UMaterialInterface* LoadConstructionMat(const TCHAR* LookName, const TCHAR* LabName, const FLinearColor& Tint, float LabEmissive = -1.f, bool bLabTint = false);
	void ApplyArenaDressing();   // lane A103: shows the Blender pieces or A25's code benches, never both
	FString ArenaTier;           // lane A103: the tier the dressing was last decided for (Build and SetZoneRingsForTier write it)
	bool bArenaShown = false;
	bool bArenaGoalShown = false;   // lane A119
	bool bClayTier = false;   // lane A69: the tier the rink shows is p3_clay (rings on, decal quads off, glass and rail not drawn)
	bool bLookIce = false;
	bool bLookTrim = false;
	FString GlassLookName = TEXT("placeholder");
	FLinearColor BeaconColour = FLinearColor::White;   // game.look.trim_colour at the last Build, for the opaque beacon mode
};
