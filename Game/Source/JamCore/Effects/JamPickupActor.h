// Lane A14: the toy pickup on the ice. Lane A21 (B-to-A-0016): device B's look when its assets are present:
// /JamAssets/Props/SM_Pickup_Base (a 70 cm pedestal ring, MI_Pickup_Base) on the ice, /JamAssets/Props/SM_Pickup_Token at the base's
// socket PickupTop (45 cm up) with /JamAssets/Look/MI_Pickup_<kind> (MI_Pickup_custom for any other effect id), both stencil 1; spin, bob
// and scrolling are in B's materials, so the actor stands still. Optional /JamAssets/VFX/NS_pickup_idle at the base with User.Colour
// (the instance's PickupColour when it has one), Deactivate() on take or despawn. Every load by path is quiet and falls back to the
// A14 look (a glowing capsule, sphere fallback, bobbing and spinning by sim frame, stencil 0) so a missing asset never blocks play.
// Lane A31 (B-to-A-0019, R-0027 point 4): B's prop per kind, /JamAssets/Props/SM_Pickup_<kind> (film reel, lens, canister, spot lamp, megaphone,
// clapperboard; MI_PickupProp_<kind> on the mesh itself), replaces the token at PickupTop wherever it exists: stencil 1, no collision, no shadow,
// spun 90 degrees per second and bobbed 6 cm in code by sim frame (frame + alpha, never delta time). The token stays as the fallback per kind.
// Render-only: AJamPickupActor::Sync mirrors UJamEffectSystem::GetPickups() every render frame; nothing here writes back.
// Lane A188 (track F3, evolution 8, R-0064 point 6, docs/experiments/BOOSTS.md): "special boosts are not seen on ice, not seen equipped on players".
//   The beacon (jam.boost.Beacon 1, default): a pickup is a turning shape by kind (A's stand-ins from /Engine/BasicShapes: speed a cone, precision a
//   diamond, ...; device B's /JamAssets/Props/SM_Boost_<kind> by name wherever it exists) 1.25 m over the ice on a 1 m pedestal of light, with a 2.2 m
//   ground ring in the kind's colour, all in A's /Game/Placeholders/M_Trim (TeamColour, Emissive). It drops to a low profile (35 % height) while a skater
//   or the puck is within 1.6 m of it, so it never hides either. 0 = the look of before (B's base ring with its prop or token).
//   AJamBoostAura: while a skater holds an ability (the kind, an armed kind, a running shield) a waist ring of light in the ability's colour turns round
//   him; device B's figure call SetBoost(FName Kind, bool bOn) is called by name on a change wherever the figure declares it (and A's ring then stands
//   down: jam.boost.Aura 1); PlayCelebrate(Kind) by name on the scorer and the nearest mate on a goal (JamBoostLook::CelebrateGoal).
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JamEffectTypes.h"
#include "JamPickupActor.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UNiagaraComponent;
class UJamEffectSystem;
class UJamSim;
class AJamSkater;
class APlayerController;

UCLASS()
class JAMCORE_API AJamPickupActor : public AActor
{
	GENERATED_BODY()

public:
	static constexpr double HoverM = 0.45;
	static constexpr double BobM = 0.08;
	// Lane A31: B's prop at PickupTop: 90 degrees per second (1.5 per frame) and a 6 cm bob over a 2 s period, by sim frame.
	static constexpr double PropSpinDegPerFrame = 1.5;
	static constexpr double PropBobCm = 6.0;
	static constexpr double PropBobPeriodFrames = 120.0;

	AJamPickupActor();
	void InitPickup(const FJamPickupOnIce& P);
	// Frame + Alpha (0..1 of the next step) drive the bob and the spin (A14 look only; B's materials animate themselves).
	void ApplyRender(double FrameF);
	// The pickup is gone (taken or despawned): the idle effect is deactivated so it ends itself, then the actor is destroyed.
	void Retire();

	int32 GetPickupId() const { return PickupId; }
	// True when B's base and token meshes carry this pickup (tests and the PIE proof read it).
	UFUNCTION(BlueprintPure, Category = "Jam|Pickup")
	bool IsLookFromB() const { return bLookB; }
	// Lane A31: true when B's SM_Pickup_<kind> prop rides the base instead of the token (BlueprintPure so the PIE probe reads it through Python).
	UFUNCTION(BlueprintPure, Category = "Jam|Pickup")
	bool IsPropFromB() const { return bProp; }

	// B-to-A-0016 asset paths (object paths; the loads are quiet, a miss falls back).
	static const TCHAR* BaseMeshPath() { return TEXT("/JamAssets/Props/SM_Pickup_Base.SM_Pickup_Base"); }
	static const TCHAR* TokenMeshPath() { return TEXT("/JamAssets/Props/SM_Pickup_Token.SM_Pickup_Token"); }
	static const TCHAR* IdleFxPath() { return TEXT("/JamAssets/VFX/NS_pickup_idle.NS_pickup_idle"); }
	static FString TokenMaterialPath(FName Kind);
	// Lane A31 (B-to-A-0019): /JamAssets/Props/SM_Pickup_<kind> (a built-in kind, else SM_Pickup_custom).
	static FString PropMeshPath(FName Kind);
	static const FName TokenSocket;   // "PickupTop"

	// Lane A188: the beacon's measures (metres) and its stand-in shape by kind.
	static constexpr double BeaconObjectM = 1.25;      // the shape's centre over the ice
	static constexpr double BeaconObjectSizeM = 0.8;   // the shape's largest side
	static constexpr double BeaconPedestalM = 1.0;     // the pedestal of light's height
	static constexpr double BeaconPedestalWideM = 0.2;
	static constexpr double BeaconRingRadiusM = 1.1;
	static constexpr int32 BeaconRingSegments = 16;
	static constexpr double BeaconClearM = 1.6;        // a skater or the puck this close: the low profile
	static constexpr double BeaconLowScale = 0.35;
	static constexpr double BeaconSpinDegPerFrame = 2.0;   // 120 degrees per second
	// "cone" (speed), "diamond" (precision), "puck" (heavy_puck), "ball" (big_puck), "cube" (reverse_other), "star" (custom and anything else).
	static FName StandInShape(FName Kind);
	static FString StandInMeshPath(FName Shape);
	// Device B's boost prop by name (A-to-B-0027 point 2a), loaded quietly; a miss keeps A's stand-in.
	static FString BoostPropPath(FName Kind);
	static bool BeaconOn();
	// 0..1 of the beacon's full height for the nearest skater or puck at NearestM (BeaconLowScale inside BeaconClearM, full from 1 m outside it).
	static double BeaconHeightFor(double NearestM);
	bool IsBeacon() const { return bBeacon; }
	bool IsBoostPropFromB() const { return bBoostPropB; }
	FName GetKind() const { return Kind; }
	FVector2D GetPosM() const { return PosM; }
	// The beacon's top and foot in world cm (the probe projects them).
	FVector GetTopWorld() const;

	// Spawns, moves and destroys actors so that Actors mirrors Effects' pickups on the ice. World null = no-op.
	// Lane A188: KeepClear (rink metres: the skaters and the puck) lowers a beacon that would hide one of them; null = none.
	static void Sync(UWorld* World, const UJamEffectSystem* Effects, TArray<TObjectPtr<AJamPickupActor>>& Actors, double FrameF, const TArray<FVector2D>* KeepClear = nullptr);
	// Lane A188's measurement (jam.boost.Probe N > 0, every N sim frames): per pickup on the ice its distance from the camera and its height and width
	// on screen in pixels (the visible parts' bounds projected), and the human's passes within 5 m of a pickup that he did not take. JAMBOOST lines.
	static void Probe(APlayerController* PC, const TArray<TObjectPtr<AJamPickupActor>>& Actors, const FVector2D& HumanM, int64 Frame);
	// The visible parts' bounds projected to the screen: height and width in pixels and the camera distance in metres. False when behind the camera.
	bool ScreenSize(APlayerController* PC, double& OutHeightPx, double& OutWidthPx, double& OutDistM) const;
	static void DestroyAll(TArray<TObjectPtr<AJamPickupActor>>& Actors);

private:
	UPROPERTY(VisibleAnywhere, Category = "Jam|Pickup") TObjectPtr<UStaticMeshComponent> Mesh;     // A14 capsule, or B's base ring
	UPROPERTY(VisibleAnywhere, Category = "Jam|Pickup") TObjectPtr<UStaticMeshComponent> Token;    // B's token at PickupTop
	UPROPERTY(VisibleAnywhere, Category = "Jam|Pickup") TObjectPtr<UStaticMeshComponent> Prop;     // lane A31: B's SM_Pickup_<kind> at PickupTop
	UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> Mid;
	UPROPERTY(Transient) TObjectPtr<UNiagaraComponent> Idle;
	// Lane A188: the beacon.
	UPROPERTY(VisibleAnywhere, Category = "Jam|Pickup") TObjectPtr<UStaticMeshComponent> Pedestal;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Pickup") TObjectPtr<UStaticMeshComponent> Shape;
	// The ground ring's segments: plain components, not instances (A's M_Trim is not flagged for instanced meshes: instances drew the default material).
	UPROPERTY(Transient) TArray<TObjectPtr<UStaticMeshComponent>> GroundRing;
	UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> BeaconMid;
	UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> ShapeMid;
	void InitBeacon(FName LookKind, const FLinearColor& Colour);
	void ApplyBeacon(double FrameF);
	int32 PickupId = -1;
	FVector2D PosM = FVector2D::ZeroVector;
	FName Kind;
	bool bLookB = false;
	bool bProp = false;
	bool bBeacon = false;
	bool bBoostPropB = false;
	FVector ShapeScale = FVector::OneVector;
	FRotator ShapeTilt = FRotator::ZeroRotator;
	double HeightNow = 1.0;
	double HeightWant = 1.0;
	double LastFrameF = -1.0;
};

// Lane A188: the equipped look. One per skater, attached to the skater's actor (it follows the drawn figure), hidden while he holds nothing.
UCLASS()
class JAMCORE_API AJamBoostAura : public AActor
{
	GENERATED_BODY()

public:
	static constexpr double RingHeightM = 0.95;
	static constexpr double RingRadiusM = 0.62;
	static constexpr double ShieldRadiusM = 0.85;
	static constexpr int32 RingSegments = 12;

	AJamBoostAura();
	// The ability's colour: shotgun red, hip_check magenta, shield ice-white, slow_puck lime; anything else white.
	static FLinearColor AbilityColour(FName Kind);
	// What the skater holds for the look: the held kind, else the armed kind, else "shield" while one runs; NAME_None = nothing. bOutLive: armed or a shield.
	static FName HeldKindFor(const UJamSim* Sim, int32 Skater, bool& bOutLive);
	// Proof only (jam.boost.Preview "<kind>" or "<kind>!" for live): the human is drawn as holding that kind while the sim gives him none. Never in the sim.
	static FName PreviewKind(bool& bOutLive);
	// 0 off (the screen of before), 1 (default) A's ring unless the figure took B's SetBoost(Kind, bOn), 2 A's ring always.
	static int32 AuraMode();
	// Device B's by-name figure calls (A-to-B-0027 points 2b and 6). True when the figure declares the function with a fitting signature and was called.
	static bool CallSetBoost(UObject* Figure, FName Kind, bool bOn);
	static bool CallPlayCelebrate(UObject* Figure, FName Kind, int32 Index);

	// Mirrors every skater's held ability: creates, shows, hides; calls SetBoost by name on a change. World null = no-op.
	static void Sync(UWorld* World, const UJamSim* Sim, const TArray<TObjectPtr<AJamSkater>>& Skaters, TArray<TObjectPtr<AJamBoostAura>>& Auras, double FrameF);
	static void DestroyAll(TArray<TObjectPtr<AJamBoostAura>>& Auras);

	FName GetShownKind() const { return ShownKind; }
	bool IsRingShown() const;
	int32 SkaterIndex = -1;

private:
	void Show(FName InKind, bool bLive, bool bRing, double FrameF);
	UPROPERTY(VisibleAnywhere, Category = "Jam|Boost") TObjectPtr<USceneComponent> Pivot;
	UPROPERTY(Transient) TArray<TObjectPtr<UStaticMeshComponent>> Ring;
	bool bRingShown = false;
	UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> Mid;
	FName ShownKind;
	FName FigureKind;          // what SetBoost(Kind, true) last told the figure (NAME_None: nothing on)
	TWeakObjectPtr<UObject> Figure;   // the figure told, so a restart can tell it off
	bool bFigureTook = false;  // the figure declares SetBoost(FName, bool) and took the last call
	bool bShieldShape = false;
};

namespace JamBoostLook
{
	// On a goal (Who = the last touch, WhereM = the puck in the net, rink metres): the scorer (the last touch when he skates for the scoring team, else the
	// scoring team's skater nearest the puck) and his nearest mate get PlayCelebrate(Kind) by name; Kind by the goal's count: arms_up, stick_raise,
	// knee_glide for the scorer, stick_raise (knee_glide when the scorer raises the stick) for the mate. One JAMFIG celebrate line. Returns the figures
	// that took the call.
	JAMCORE_API int32 CelebrateGoal(const UJamSim* Sim, const TArray<TObjectPtr<AJamSkater>>& Skaters, int32 Who, const FVector2D& WhereM);
	JAMCORE_API FName CelebrateKind(int32 GoalCount, bool bScorer);
	// Pure: the scorer from team ids and positions (tests); OutMate = his nearest team mate, -1 when he has none.
	JAMCORE_API int32 PickScorer(const TArray<int32>& Teams, const TArray<FVector2D>& Pos, int32 ScoringTeam, int32 Who, const FVector2D& WhereM, int32& OutMate);
}
