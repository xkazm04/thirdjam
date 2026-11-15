#include "RinkActor.h"
#include "JamSim.h"
#include "ProceduralMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "LookTierSwitcher.h"
#include "NeonArena.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "UObject/UObjectIterator.h"
#include "Engine/GameInstance.h"
#include "Components/PointLightComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "JamPawns.h"
#include "Engine/StaticMeshSocket.h"
#include "HokejGameMode.h"   // lane A210: the sim, to tell a man checked into the boards from one who ran into them

// Lane A25: the construction layer's switch. 0 hides the kick plate, glass rail, net frame extras, goal lights, benches and the extra markings on every rink
// actor (the "before" of the ProfileGPU pair); 1 shows them. Read at Build and on every change.
static TAutoConsoleVariable<int32> CVarRinkConstruction(TEXT("jam.rink.construction"), 1,
	TEXT("1: the rink's construction layer (kick plate, glass rail, net frame, goal lights, benches, faceoff markings) is visible; 0: hidden. Lane A25."));
// Default 0: at 40 m the eyes tool preferred no column over B's translucent one (0.95: "a massive, distracting in-world light beam"; the 14 cm net frame
// reads 4/5 as the far net on both rink sizes without it) and the translucent one only weakly over the opaque (0.6). docs/experiments/RINK.md has the pair.
static TAutoConsoleVariable<int32> CVarRinkBeacon(TEXT("jam.rink.beacon"), 0,
	TEXT("Far-net beacon column: 0 off, 1 B's MI_Trim_Beacon (translucent), 2 opaque emissive column. Lane A25, eyes finding 3."));
// Lane A103 (track F3): the Blender-made benches and penalty boxes. 0 = A25's code benches on every tier (the rink as it was before the lane); 1 = the meshes on
// the dressed tiers. Read at Build and on every change through the construction sink below (SetConstructionVisible applies both switches).
static TAutoConsoleVariable<int32> CVarRinkArena(TEXT("jam.rink.arena"), 1,
	TEXT("1: the Blender-made team benches, penalty boxes, scorer's table and goals (/Game/Arena) stand on the dressed look tiers; 0: lane A25's code benches and code net frame. Lanes A103, A119."));

// Lane A127 (track F3, evolution 3): the arena's answers to the game (the goal lamp, the goal lights, the net's shake, the rattle of a bench, box or table,
// the boxed skater's figure in its box). 0 = the pieces stand still and the boxed skater is drawn on the ice, as before the lane.
static TAutoConsoleVariable<int32> CVarRinkReact(TEXT("jam.rink.react"), 1,
	TEXT("1: the arena answers the game (a goal lights the lamp and shakes the net, a hard board hit rocks the piece behind it, a boxed skater stands in the box); 0: the still arena. Lane A127."));

// Lane A136 (track F3, evolution 4): the board caps (one clay cap on the board top at every stanchion place, the piece a hard board hit rocks anywhere on the
// ring). 0 = no caps: the arena of evolution 3. Read through the construction sink like the other switches.
static TAutoConsoleVariable<int32> CVarRinkCaps(TEXT("jam.rink.caps"), 1,
	TEXT("1: clay caps stand on the board top round the whole ring on the dressed tiers and rock when a skater hits the boards hard; 0: no caps. Lane A136."));

// Lane A127: the lamp lights' intensity at the top of a flash, in candelas. 0 = the lamps flash without lighting anything around them.
// Lane A160 (evolution 6): device B measured the goal on both tiers (A-to-B-0024, B's answer to ask 4; B-to-A-0041 point 6): this light was what fought
// B's NS_goal (pink ice for 108 frames: 11 to 16 % of the ice pixels on clay). B's numbers are the default now: 1,600 here with `jam.rink.react.loud 0`
// = 800 cd at the scored end on p3_clay and 320 on l2_night, inside a 3.5 m radius beside the lamp behind the end boards: the light is off the ice and
// B's effect carries the moment. `jam.rink.react.loud 1` with `jam.rink.react.light 6000` gives back evolution 5's loud goal.
static TAutoConsoleVariable<float> CVarRinkReactLight(TEXT("jam.rink.react.light"), 1600.f,
	TEXT("Candelas of the red point light over the scored end's net at the top of a goal's flash (the scorer's table's takes an eighth, l2_night a fifth); 0 = no light. Lanes A127, A136."));
// Lane A136 (evolution 4): the loud goal. 1 = the scored end's light hangs over the net (it reaches the crease and the slot); 0 = evolution 3's light, beside
// the lamp behind the end boards at half the candelas.
static TAutoConsoleVariable<int32> CVarRinkReactLoud(TEXT("jam.rink.react.loud"), 0,
	TEXT("1: a goal's red light hangs over the scored net and lights the end zone; 0: the light beside the lamp behind the end boards at half the candelas (lane A127's). Lane A136."));

// Lane A145 (evolution 5): the hit flash. Candelas of the point light a hard board hit throws at the hit's place at Strength 1 (l2_night takes a fifth).
// 0 = no flash: the quiet hit of evolution 4. The tint: 0 warm white, 1 the half's team colour (blue on x < 0, orange on x > 0), 2 the lamps' red.
// 300 cd inside a 4.5 m radius from two sweeps of off-screen stills from the game camera 12 m off the boards (ARENA.md section 40): at a 9 m radius
// 1,500 cd washed 15 to 23 % of the picture; 300 cd in 4.5 m changes 4.0 % at Strength 0.45 and 5.0 % at 0.8 on p3_clay, a pool at the hit.
// Lane A210 (wave 4; the eye's EYE 12: "skating into the boards turns his whole figure pale blue for six frames"): 300 cd in the half's team colour stood
// 0.6 m from the man who hit the boards: from the game camera 7 m behind him his figure went blue-white over the hit's follow-through. 90 cd, warm white:
// the boards and the ice at the hit still answer, the figure keeps its colours (stills: docs/experiments/img/a210). 300 / tint 1 = package H34.
static TAutoConsoleVariable<float> CVarRinkReactHit(TEXT("jam.rink.react.hit"), 90.f,
	TEXT("Candelas of the flash a hard skater board hit throws on the boards and the ice at the hit, at Strength 1 (l2_night takes a fifth); 0 = no flash. Lane A145."));
// Lane A210 (wave 4): no flash at all for a plain run into the boards (still seen at 90 cd: the light stands 0.6 m from the man and his figure goes
// pale): the flash is a HIT's: a man checked into the boards, or going down there. 1 = every hard board hit flashes (package H34).
static TAutoConsoleVariable<int32> CVarRinkReactHitBump(TEXT("jam.rink.react.hit.bump"), 0,
	TEXT("The hit flash on a plain run into the boards: 0 (default) none (the cap still rocks); 1 = as for a hit (package H34). Lane A210."));
static TAutoConsoleVariable<int32> CVarRinkReactHitTint(TEXT("jam.rink.react.hit.tint"), 0,
	TEXT("The hit flash's colour: 0 warm white, 1 the team colour of the half (blue on x < 0, orange on x > 0), 2 the goal lamps' red. Lane A145."));
// Lane A160 (evolution 6): the box seen from the ice. Candelas of the light inside a penalty box while its penalty runs (p3_clay; every other tier takes
// a fifth: the same share as the hit flash). 0 = the dark box of evolution 5. The door hides behind the boards and the clock's hand is 20 pixels from the
// game camera (lane A145's finding): what is seen from the ice is the lit box with the boxed figure in it.
// Lane A172: 450 (it was 750): from the game camera's real pose beside the box on the clay rig 750 bleached the boxed figure's head and shoulders.
static TAutoConsoleVariable<float> CVarRinkReactBox(TEXT("jam.rink.react.box"), 450.f,
	TEXT("Candelas of the light inside a penalty box while its penalty runs (p3_clay; other tiers take a fifth); 0 = no light. Lane A160."));
// Lane A160: on the bare tiers (l1_day, l3_neon) the Blender goals and the board caps stand too: they are on the ice and on the boards, inside the rig's
// light, and the pond's own goal (SM_Arena_Goal_W600H061) was built for a cut that plays on l1_day and never showed. 0 = the code frame and no caps there.
static TAutoConsoleVariable<int32> CVarRinkArenaBare(TEXT("jam.rink.arena.bare"), 1,
	TEXT("1 (default): on the bare tiers (l1_day, l3_neon) the Blender goals and the board caps stand; 0: the code net frame and no caps there, as before lane A160."));
// Lane A172 (evolution 7): device B's lamp and cords (handoff A-to-B-0024, B's answer to ask 2). The lamps' slot `lamp` takes B's MI_Trim_Lamp (an
// instance of M_Trim, whose Emissive is a true emissive: on M_Toon it is a rim term and the lamp flattened): off at LampTrimRest, flashing to LampTrimPeak
// (B: 0.5 or under reads off, 8 to 12 flashes, from about 15 it loses its red). The goals' slot `net` takes B's plain off-white MI_Toon_NetCord.
// 0 = evolution 6's materials: the net frame's red on the lamps (rest 3, peak 17) and MI_Toon_NetMesh on the cords.
static TAutoConsoleVariable<int32> CVarRinkLampTrim(TEXT("jam.rink.lamp.trim"), 1,
	TEXT("1 (default): the goal lamps are device B's MI_Trim_Lamp, off at rest (Emissive 0.5) and flashing to 9; 0: MI_Toon_NetFrame as before lane A172 (rest 3, peak 17). Read at Build."));
static TAutoConsoleVariable<int32> CVarRinkNetCord(TEXT("jam.rink.net.cord"), 1,
	TEXT("1 (default): the Blender goals' cords are device B's MI_Toon_NetCord (plain off-white); 0: MI_Toon_NetMesh as before lane A172. Read at Build."));
// Lane A145: the goal wave. The share of the scored end's light that each of the two travelling lights reaches at the end of a pulse. 0 = no wave.
// 0.12 from a sweep (section 41): the game camera's picture of the scored end changes 14.8 % on the goal's frame, 16.2 % in the middle of a pulse and
// 14.6 % at its end (10.4 % and 6.2 % without the wave); at 0.5 the troughs were louder than the goal's own frame (26 %).
static TAutoConsoleVariable<float> CVarRinkReactWave(TEXT("jam.rink.react.wave"), 0.12f,
	TEXT("A goal runs round the scored end's caps: the two travelling lights' share of the end light's candelas at the end of each pulse; 0 = no wave (no cap hops on a goal). Lane A145."));

// Lane A119: the goal meshes arena.py builds (tools/blender/arena/arena.py GOAL_VARIANTS): the opening each was built for. Build takes the nearest and scales it.
namespace
{
	struct FArenaGoalVariant { double WidthM; double HeightM; const TCHAR* Name; const TCHAR* Path; const TCHAR* NetPath; };
	const FArenaGoalVariant GArenaGoalVariants[] = {
		{ 4.0, 0.61, TEXT("SM_Arena_Goal_W400H061"), TEXT("/Game/Arena/SM_Arena_Goal_W400H061.SM_Arena_Goal_W400H061"), TEXT("/Game/Arena/SM_Arena_GoalNet_W400H061.SM_Arena_GoalNet_W400H061") },   // the default goal
		{ 6.0, 0.61, TEXT("SM_Arena_Goal_W600H061"), TEXT("/Game/Arena/SM_Arena_Goal_W600H061.SM_Arena_Goal_W600H061"), TEXT("/Game/Arena/SM_Arena_GoalNet_W600H061.SM_Arena_GoalNet_W600H061") },   // hockey.mode.pond
		{ 4.0, 1.22, TEXT("SM_Arena_Goal_W400H122"), TEXT("/Game/Arena/SM_Arena_Goal_W400H122.SM_Arena_Goal_W400H122"), TEXT("/Game/Arena/SM_Arena_GoalNet_W400H122.SM_Arena_GoalNet_W400H122") },   // hockey.mode.sniper
	};
}
static FAutoConsoleVariableSink GRinkConstructionSink(FConsoleCommandDelegate::CreateLambda([]()
{
	const bool bOn = ARinkActor::IsConstructionEnabled();
	for (TObjectIterator<ARinkActor> It; It; ++It) if (IsValid(*It)) { It->SetConstructionVisible(bOn); It->ApplyBeaconMode(); }
}));

namespace
{
	constexpr double UU = 100.0; // metres -> Unreal units
	constexpr bool kFrontAlongCross = false; // engine convention: GenerateBoxMesh top face (0,1,3) has cross(B-A, C-A) opposite its normal

	struct FMeshBuf
	{
		TArray<FVector> V;
		TArray<int32> T;
		TArray<FVector> N;
		TArray<FVector2D> UV;
		TArray<FLinearColor> C;
		TArray<FProcMeshTangent> Tan;

		// Quad A B C D (in order around the face); Normal is the side it faces. Both windings are emitted only when bTwoSided.
		void Quad(const FVector& A, const FVector& B, const FVector& Cc, const FVector& D, const FVector& Normal, bool bTwoSided = false)
		{
			const int32 I = V.Num();
			V.Append({ A, B, Cc, D });
			for (int32 k = 0; k < 4; ++k) { N.Add(Normal); C.Add(FLinearColor::White); Tan.Add(FProcMeshTangent()); }
			UV.Append({ FVector2D(A.X, A.Y) / 1000.0 + FVector2D(0, A.Z / 1000.0), FVector2D(B.X, B.Y) / 1000.0 + FVector2D(0, B.Z / 1000.0),
				FVector2D(Cc.X, Cc.Y) / 1000.0 + FVector2D(0, Cc.Z / 1000.0), FVector2D(D.X, D.Y) / 1000.0 + FVector2D(0, D.Z / 1000.0) });
			// Front face: the triangle whose cross(B - A, C - A) points along Normal (kFrontAlongCross; flip it if faces vanish).
			const FVector Cross = FVector::CrossProduct(B - A, Cc - A);
			if ((FVector::DotProduct(Cross, Normal) > 0) == kFrontAlongCross) T.Append({ I, I + 1, I + 2, I, I + 2, I + 3 });
			else T.Append({ I, I + 2, I + 1, I, I + 3, I + 2 });
			if (bTwoSided)
			{
				const int32 J = V.Num();
				V.Append({ A, B, Cc, D });
				for (int32 k = 0; k < 4; ++k) { N.Add(-Normal); C.Add(FLinearColor::White); Tan.Add(FProcMeshTangent()); UV.Add(FVector2D::ZeroVector); }
				if ((FVector::DotProduct(Cross, Normal) > 0) == kFrontAlongCross) T.Append({ J, J + 2, J + 1, J, J + 3, J + 2 });
				else T.Append({ J, J + 1, J + 2, J, J + 2, J + 3 });
			}
		}

		// Lane A25: overwrites the UVs of the quad just emitted (one-sided quads only): the trim strips take U = distance along the outline, V = height,
		// so the pattern runs continuously around the corner arcs instead of stepping where the face turns from x to y (eyes finding 2, trim seam).
		void LastUV(const FVector2D& A, const FVector2D& B, const FVector2D& Cc, const FVector2D& D)
		{
			const int32 I = UV.Num() - 4;
			if (I < 0) return;
			UV[I] = A; UV[I + 1] = B; UV[I + 2] = Cc; UV[I + 3] = D;
		}

		// Axis-aligned box (all six faces, outward normals).
		void Box(const FVector& Min, const FVector& Max)
		{
			const FVector P[8] = {
				{ Min.X, Min.Y, Min.Z }, { Max.X, Min.Y, Min.Z }, { Max.X, Max.Y, Min.Z }, { Min.X, Max.Y, Min.Z },
				{ Min.X, Min.Y, Max.Z }, { Max.X, Min.Y, Max.Z }, { Max.X, Max.Y, Max.Z }, { Min.X, Max.Y, Max.Z } };
			Quad(P[4], P[5], P[6], P[7], FVector::UpVector);
			Quad(P[0], P[3], P[2], P[1], -FVector::UpVector);
			Quad(P[0], P[1], P[5], P[4], -FVector::RightVector);
			Quad(P[3], P[7], P[6], P[2], FVector::RightVector);
			Quad(P[0], P[4], P[7], P[3], -FVector::ForwardVector);
			Quad(P[1], P[2], P[6], P[5], FVector::ForwardVector);
		}

		// Flat ring or disc on the ice (Z = H), from radius R0 to R1, centre C (cm).
		void Ring(const FVector2D& Cc, double R0, double R1, double H, int32 Seg = 48)
		{
			for (int32 i = 0; i < Seg; ++i)
			{
				const double A0 = 2.0 * UE_DOUBLE_PI * i / Seg, A1 = 2.0 * UE_DOUBLE_PI * (i + 1) / Seg;
				const FVector2D D0(FMath::Cos(A0), FMath::Sin(A0)), D1(FMath::Cos(A1), FMath::Sin(A1));
				const FVector2D P0 = Cc + D0 * R0, P1 = Cc + D0 * R1, P2 = Cc + D1 * R1, P3 = Cc + D1 * R0;
				Quad(FVector(P0, H), FVector(P1, H), FVector(P2, H), FVector(P3, H), FVector::UpVector);
			}
		}

		void Strip(const FVector2D& A, const FVector2D& B, double HalfW, double H)
		{
			const FVector2D Dir = (B - A).GetSafeNormal();
			const FVector2D Side(-Dir.Y * HalfW, Dir.X * HalfW);
			Quad(FVector(A - Side, H), FVector(B - Side, H), FVector(B + Side, H), FVector(A + Side, H), FVector::UpVector);
		}

		// Lane A25: a flat ring sector on the ice from angle A0 to A1 (radians, counter-clockwise), for creases.
		void Arc(const FVector2D& Cc, double R0, double R1, double A0, double A1, double H, int32 Seg = 24)
		{
			for (int32 i = 0; i < Seg; ++i)
			{
				const double T0 = A0 + (A1 - A0) * i / Seg, T1 = A0 + (A1 - A0) * (i + 1) / Seg;
				const FVector2D D0(FMath::Cos(T0), FMath::Sin(T0)), D1(FMath::Cos(T1), FMath::Sin(T1));
				Quad(FVector(Cc + D0 * R0, H), FVector(Cc + D0 * R1, H), FVector(Cc + D1 * R1, H), FVector(Cc + D1 * R0, H), FVector::UpVector);
			}
		}

		// Lane A25: a box of half-width HalfW (sideways) and half-height HalfH (up) along the line A -> B, any direction (rails along the
		// rounded corners, the nets' diagonal braces). Up is the vertical-most direction perpendicular to the bar; a vertical bar takes +x as its side.
		void Bar(const FVector& A, const FVector& B, double HalfW, double HalfH)
		{
			const FVector Dir = (B - A).GetSafeNormal();
			FVector Side = FVector::CrossProduct(Dir, FVector::UpVector);
			if (!Side.Normalize()) Side = FVector::ForwardVector;
			const FVector Up = FVector::CrossProduct(Side, Dir).GetSafeNormal();
			const FVector S = Side * HalfW, U = Up * HalfH;
			const FVector P[8] = { A - S - U, A + S - U, A + S + U, A - S + U, B - S - U, B + S - U, B + S + U, B - S + U };
			Quad(P[3], P[2], P[6], P[7], Up);
			Quad(P[0], P[4], P[5], P[1], -Up);
			Quad(P[1], P[5], P[6], P[2], Side);
			Quad(P[0], P[3], P[7], P[4], -Side);
			Quad(P[0], P[1], P[2], P[3], -Dir);
			Quad(P[4], P[7], P[6], P[5], Dir);
		}

		void Commit(UProceduralMeshComponent* Pm, int32 Section)
		{
			Pm->CreateMeshSection_LinearColor(Section, V, T, N, UV, C, Tan, false);
			*this = FMeshBuf();
		}
	};
}

ARinkActor::ARinkActor()
{
	// Lane A127: the tick runs only while a reaction is live (ReactToEvent switches it on, Tick switches it off).
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	auto MakePm = [this, Root](const TCHAR* Name)
	{
		UProceduralMeshComponent* P = CreateDefaultSubobject<UProceduralMeshComponent>(Name);
		P->SetupAttachment(Root);
		P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		P->bUseAsyncCooking = true;
		return P;
	};
	Ice = MakePm(TEXT("Ice"));
	Markings = MakePm(TEXT("Markings"));
	Boards = MakePm(TEXT("Boards"));
	Glass = MakePm(TEXT("Glass"));
	Trim = MakePm(TEXT("Trim"));
	TrimOrange = MakePm(TEXT("TrimOrange"));
	Nets = MakePm(TEXT("Nets"));
	Obstacles = MakePm(TEXT("Obstacles"));
	Zones = MakePm(TEXT("Zones"));
	KickPlate = MakePm(TEXT("KickPlate"));
	GlassRail = MakePm(TEXT("GlassRail"));
	GoalLights = MakePm(TEXT("GoalLights"));
	Benches = MakePm(TEXT("Benches"));
	Glass->SetCastShadow(false);
	// Lane A25: the glass panel stays stencil 0 (no ramp, no outline: a translucent sheet with an outline reads as a wall) and casts nothing.
	Glass->SetRenderCustomDepth(false);
	Glass->SetCustomDepthStencilValue(0);
	// R-0002 (B-to-A-0005): stencil 1 = toon ramp + outline, 2 = banded ice without outline, 0 = untouched.
	for (UProceduralMeshComponent* P : { Boards.Get(), Nets.Get(), Obstacles.Get(), KickPlate.Get(), GlassRail.Get(), GoalLights.Get(), Benches.Get() }) { P->SetRenderCustomDepth(true); P->SetCustomDepthStencilValue(1); }
	Ice->SetRenderCustomDepth(true);
	Ice->SetCustomDepthStencilValue(2);
	// Lane A25: the painted markings lie on the ice and take its stencil (banded, no outline), like the zone decals; they cast nothing.
	Markings->SetRenderCustomDepth(true);
	Markings->SetCustomDepthStencilValue(2);
	Markings->SetCastShadow(false);
	// Lane A25: the thin rail and stanchions cast no shadow (a 5 cm post's VSM shadow is noise); the kick plate, benches and goal lights do.
	GlassRail->SetCastShadow(false);
	// C-0005 / A15: zone decals lie on the ice, so they take the ice's stencil (banded, no outline) and cast nothing.
	Zones->SetRenderCustomDepth(true);
	Zones->SetCustomDepthStencilValue(2);
	Zones->SetCastShadow(false);
	for (UProceduralMeshComponent* P : { Trim.Get(), TrimOrange.Get() }) { P->SetRenderCustomDepth(false); P->SetCustomDepthStencilValue(0); P->SetCastShadow(false); }
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cyl(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	for (int32 i = 0; i < 2; ++i)
	{
		UStaticMeshComponent* Bc = CreateDefaultSubobject<UStaticMeshComponent>(i == 0 ? TEXT("BeaconPos") : TEXT("BeaconNeg"));
		Bc->SetupAttachment(Root);
		if (Cyl.Succeeded()) Bc->SetStaticMesh(Cyl.Object);
		Bc->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Bc->SetCastShadow(false);
		Bc->SetRenderCustomDepth(false);
		Bc->SetCustomDepthStencilValue(0);
		(i == 0 ? BeaconPos : BeaconNeg) = Bc;
	}
	// Lane A103: the four Blender pieces. Empty and hidden until Build finds the meshes; stencil 1 (ramp and outline) like A25's benches; they cast shadows.
	{
		// Lane A119: three more of the same kind: the two goals and the scorer's table.
		// Lane A127: two more: the goal lights outside the end boards.
		static const TCHAR* const ArenaNames[11] = { TEXT("ArenaBenchNeg"), TEXT("ArenaBenchPos"), TEXT("ArenaBoxNeg"), TEXT("ArenaBoxPos"), TEXT("ArenaGoalNeg"), TEXT("ArenaGoalPos"), TEXT("ArenaTable"), TEXT("ArenaGoalNetNeg"), TEXT("ArenaGoalNetPos"), TEXT("ArenaGoalLightNeg"), TEXT("ArenaGoalLightPos") };
		TObjectPtr<UStaticMeshComponent>* const ArenaSlots[11] = { &ArenaBenchNeg, &ArenaBenchPos, &ArenaBoxNeg, &ArenaBoxPos, &ArenaGoalNeg, &ArenaGoalPos, &ArenaTable, &ArenaGoalNetNeg, &ArenaGoalNetPos, &ArenaGoalLightNeg, &ArenaGoalLightPos };
		for (int32 i = 0; i < 11; ++i)
		{
			UStaticMeshComponent* Ac = CreateDefaultSubobject<UStaticMeshComponent>(ArenaNames[i]);
			Ac->SetupAttachment(Root);
			Ac->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Ac->SetRenderCustomDepth(true);
			Ac->SetCustomDepthStencilValue(1);
			Ac->SetVisibility(false);
			if (i == 7 || i == 8) { Ac->SetRenderCustomDepth(false); Ac->SetCustomDepthStencilValue(0); }   // lane A119: the net's cords take no outline
			*ArenaSlots[i] = Ac;
		}
	}
	// Lane A127: the lamps' lights. Movable, shadowless, hidden at rest.
	{
		static const TCHAR* const LightNames[3] = { TEXT("ArenaLampLightTable"), TEXT("ArenaLampLightNeg"), TEXT("ArenaLampLightPos") };
		for (int32 i = 0; i < 3; ++i)
		{
			UPointLightComponent* L = CreateDefaultSubobject<UPointLightComponent>(LightNames[i]);
			L->SetupAttachment(Root);
			L->SetMobility(EComponentMobility::Movable);
			L->IntensityUnits = ELightUnits::Candelas;
			L->Intensity = 0.f;
			L->LightColor = FColor(255, 36, 16);
			L->AttenuationRadius = 1600.f;
			L->CastShadows = false;
			L->SetVisibility(false);
			ArenaLampLights[i] = L;
		}
	}
	// Lane A160: the light inside each penalty box. Movable, shadowless, hidden at rest; Build places it.
	{
		static const TCHAR* const BoxLightNames[2] = { TEXT("ArenaBoxLightNeg"), TEXT("ArenaBoxLightPos") };
		for (int32 i = 0; i < 2; ++i)
		{
			UPointLightComponent* L = CreateDefaultSubobject<UPointLightComponent>(BoxLightNames[i]);
			L->SetupAttachment(Root);
			L->SetMobility(EComponentMobility::Movable);
			L->IntensityUnits = ELightUnits::Candelas;
			L->Intensity = 0.f;
			L->LightColor = FColor(255, 236, 200);
			L->AttenuationRadius = BoxLightRadiusCm;
			L->CastShadows = false;
			L->SetVisibility(false);
			ArenaBoxLights[i] = L;
		}
	}
	// Lane A136: the board caps, one instanced component (one draw for the ring). Empty and hidden until Build finds the mesh.
	ArenaCaps = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ArenaCaps"));
	ArenaCaps->SetupAttachment(Root);
	ArenaCaps->SetMobility(EComponentMobility::Movable);
	ArenaCaps->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArenaCaps->SetRenderCustomDepth(true);
	ArenaCaps->SetCustomDepthStencilValue(1);
	ArenaCaps->SetCastShadow(false);
	ArenaCaps->SetVisibility(false);
	// Lane A145: the x >= 0 half's caps (orange), the hit and wave lights, the boxes' doors and clock hands.
	ArenaCapsPos = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ArenaCapsPos"));
	ArenaCapsPos->SetupAttachment(Root);
	ArenaCapsPos->SetMobility(EComponentMobility::Movable);
	ArenaCapsPos->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArenaCapsPos->SetRenderCustomDepth(true);
	ArenaCapsPos->SetCustomDepthStencilValue(1);
	ArenaCapsPos->SetCastShadow(false);
	ArenaCapsPos->SetVisibility(false);
	{
		static_assert(CapRipples == 4, "ArenaHitLights holds one light per ripple");
		static const TCHAR* const FlashNames[6] = { TEXT("ArenaHitLight0"), TEXT("ArenaHitLight1"), TEXT("ArenaHitLight2"), TEXT("ArenaHitLight3"), TEXT("ArenaWaveLightNeg"), TEXT("ArenaWaveLightPos") };
		for (int32 i = 0; i < 6; ++i)
		{
			UPointLightComponent* L = CreateDefaultSubobject<UPointLightComponent>(FlashNames[i]);
			L->SetupAttachment(Root);
			L->SetMobility(EComponentMobility::Movable);
			L->IntensityUnits = ELightUnits::Candelas;
			L->Intensity = 0.f;
			L->LightColor = i < 4 ? FColor(255, 236, 200) : FColor(255, 36, 16);
			L->AttenuationRadius = i < 4 ? HitLightRadiusCm : 1400.f;
			L->CastShadows = false;
			L->SetVisibility(false);
			if (i < 4) ArenaHitLights[i] = L; else ArenaWaveLights[i - 4] = L;
		}
		static const TCHAR* const UseNames[4] = { TEXT("ArenaBoxDoorNeg"), TEXT("ArenaBoxDoorPos"), TEXT("ArenaBoxHandNeg"), TEXT("ArenaBoxHandPos") };
		TObjectPtr<UStaticMeshComponent>* const UseSlots[4] = { &ArenaBoxDoorNeg, &ArenaBoxDoorPos, &ArenaBoxHandNeg, &ArenaBoxHandPos };
		for (int32 i = 0; i < 4; ++i)
		{
			UStaticMeshComponent* Ac = CreateDefaultSubobject<UStaticMeshComponent>(UseNames[i]);
			Ac->SetupAttachment(i % 2 == 0 ? ArenaBoxNeg.Get() : ArenaBoxPos.Get());
			Ac->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Ac->SetRenderCustomDepth(true);
			Ac->SetCustomDepthStencilValue(1);
			Ac->SetVisibility(false);
			*UseSlots[i] = Ac;
		}
	}
	// Lane A172: the rink's own boards, trim, kick plate, caps, boxes and benches also take lighting channel 1: the quiet goal's lights shine on that
	// channel alone (PlaceLampLights), so they light these and nothing else: not their own lamps, which stand 0.4 m from them, and not the ice.
	for (UPrimitiveComponent* P : TArray<UPrimitiveComponent*>{ Boards.Get(), Trim.Get(), TrimOrange.Get(), KickPlate.Get(), ArenaCaps.Get(), ArenaCapsPos.Get(), ArenaBoxNeg.Get(), ArenaBoxPos.Get(), ArenaBenchNeg.Get(), ArenaBenchPos.Get() })
	{
		if (P) P->LightingChannels.bChannel1 = true;
	}
	GoalTriggerPos = CreateDefaultSubobject<UBoxComponent>(TEXT("GoalTriggerPos"));
	GoalTriggerNeg = CreateDefaultSubobject<UBoxComponent>(TEXT("GoalTriggerNeg"));
	for (UBoxComponent* B : { GoalTriggerPos.Get(), GoalTriggerNeg.Get() })
	{
		B->SetupAttachment(Root);
		B->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		B->SetHiddenInGame(true);
	}
}

UMaterialInterface* ARinkActor::LoadMat(const TCHAR* Path) const
{
	if (UMaterialInterface* Mi = LoadObject<UMaterialInterface>(nullptr, Path, nullptr, LOAD_NoWarn | LOAD_Quiet)) return Mi;
	return LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
}

UMaterialInterface* ARinkActor::LoadLook(const TCHAR* LookName, const TCHAR* FallbackPath, bool* bOutFromLook) const
{
	// R-0002: device B's instance at /JamAssets/Look/<name>.<name> when it exists; else our placeholder; never fail.
	const FString Path = FString::Printf(TEXT("/JamAssets/Look/%s.%s"), LookName, LookName);
	UMaterialInterface* Mi = LoadObject<UMaterialInterface>(nullptr, *Path, nullptr, LOAD_NoWarn | LOAD_Quiet);
	if (bOutFromLook) *bOutFromLook = Mi != nullptr;
	return Mi ? Mi : LoadMat(FallbackPath);
}

void ARinkActor::Outline(double LengthM, double WidthM, double CornerM, int32 ArcSegments, TArray<FVector2D>& Out)
{
	Out.Reset();
	const double Hx = LengthM * 0.5, Hz = WidthM * 0.5;
	const double Rc = FMath::Clamp(CornerM, 0.0, FMath::Min(Hx, Hz));
	const FVector2D Centres[4] = { { Hx - Rc, Hz - Rc }, { -(Hx - Rc), Hz - Rc }, { -(Hx - Rc), -(Hz - Rc) }, { Hx - Rc, -(Hz - Rc) } };
	for (int32 c = 0; c < 4; ++c)
	{
		const double Start = c * UE_DOUBLE_PI * 0.5;
		const int32 Segs = Rc > 0 ? ArcSegments : 1;
		for (int32 i = 0; i <= Segs; ++i)
		{
			const double A = Start + (UE_DOUBLE_PI * 0.5) * i / Segs;
			Out.Add(Centres[c] + FVector2D(FMath::Cos(A), FMath::Sin(A)) * Rc);
		}
	}
}

void ARinkActor::Build(const FJamGameConfig& Game)
{
	const FJamRinkConfig& R = Game.Rink;
	const double GL = R.GoalLineX();
	const double Hw = R.GoalWidthM * 0.5;
	// R-0033 point 2 (lane A50): the frame, the netting and the goal triggers stand game.rink.goal_height_m tall (0.61 by default: the half-height net).
	const double Depth = UJamSim::NetDepth, NetH = R.GoalHeightM;
	FMeshBuf B;
	ResetReactions();   // lane A127: a rebuild in the middle of a flash or a shake starts from the still arena

	// Ice: a fan over the outline.
	TArray<FVector2D> Line;
	Outline(R.LengthM, R.WidthM, R.CornerRadiusM, 12, Line);
	{
		B.V.Add(FVector::ZeroVector); B.N.Add(FVector::UpVector); B.UV.Add(FVector2D::ZeroVector); B.C.Add(FLinearColor::White); B.Tan.Add(FProcMeshTangent());
		for (const FVector2D& P : Line) { B.V.Add(FVector(P * UU, 0)); B.N.Add(FVector::UpVector); B.UV.Add(P / 10.0); B.C.Add(FLinearColor::White); B.Tan.Add(FProcMeshTangent()); }
		for (int32 i = 0; i < Line.Num(); ++i)
		{
			const int32 A = 1 + i, Bn = 1 + (i + 1) % Line.Num();
			const FVector Cross = FVector::CrossProduct(B.V[A] - B.V[0], B.V[Bn] - B.V[0]);
			if ((Cross.Z > 0) == kFrontAlongCross) B.T.Append({ 0, A, Bn }); else B.T.Append({ 0, Bn, A });
		}
		B.Commit(Ice, 0);
		IceMid = UMaterialInstanceDynamic::Create(LoadLook(TEXT("MI_Ice_Night"), IceMaterialPath, &bLookIce), this);
		IceMid->SetScalarParameterValue(TEXT("Wet"), Game.Look.bWetIce ? 1.f : 0.f);
		IceMid->SetScalarParameterValue(TEXT("RinkHalfLength"), static_cast<float>(R.LengthM * 0.5 * UU));
		IceMid->SetScalarParameterValue(TEXT("RinkHalfWidth"), static_cast<float>(R.WidthM * 0.5 * UU));
		// R-0016 point 3 (5): B's paint and A's markings and goal triggers follow one rule on every rink size: blue lines at a third of the half-length,
		// goal lines at half-length - 4 m (FJamRinkConfig::GoalLineX). A's own markings below use the same numbers (LengthM / 6 and GL).
		IceMid->SetScalarParameterValue(TEXT("BlueLineFrac"), 0.3333f);
		IceMid->SetScalarParameterValue(TEXT("GoalLineOffsetCm"), 400.f);
		// B-to-A-0023 point 5 (lane A31): M_Ice's frost band follows the cut's corner radius (game.rink.corner_radius_m in cm; B's instance default 375 is today's regular rink).
		IceMid->SetScalarParameterValue(TEXT("CornerRadiusCm"), static_cast<float>(R.CornerRadiusM * UU));
		IceMid->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.80f, 0.86f, 0.92f));
		Ice->SetMaterial(0, IceMid);
	}

	// Boards: inner face, top cap, outer face; glass band above; trim strip along the inner top edge.
	{
		FMeshBuf G, TrB, TrO, Kick, Rail, Posts;
		double TrimS = 0;   // distance along the outline so far, cm (trim UVs)
		const double H = BoardHeightM * UU, T = BoardThicknessM * UU, GH = (BoardHeightM + GlassHeightM) * UU;
		// Split the outline at x = 0 so each trim strip is wholly on the blue (x < 0) or the orange (x > 0) half.
		TArray<FVector2D> Split;
		for (int32 i = 0; i < Line.Num(); ++i)
		{
			const FVector2D A = Line[i], Bp = Line[(i + 1) % Line.Num()];
			Split.Add(A);
			if ((A.X < 0 && Bp.X > 0) || (A.X > 0 && Bp.X < 0)) Split.Add(FMath::Lerp(A, Bp, A.X / (A.X - Bp.X)));
		}
		for (int32 i = 0; i < Split.Num(); ++i)
		{
			const FVector2D P0 = Split[i] * UU, P1 = Split[(i + 1) % Split.Num()] * UU;
			if (FVector2D::Distance(P0, P1) < 1.0) continue;
			const FVector2D Dir = (P1 - P0).GetSafeNormal();
			const FVector2D Out(Dir.Y, -Dir.X); // outward for a counter-clockwise outline
			const FVector2D Q0 = P0 + Out * T, Q1 = P1 + Out * T;
			const FVector Nin(-Out, 0), Nout(Out, 0);
			B.Quad(FVector(P0, 0), FVector(P1, 0), FVector(P1, H), FVector(P0, H), Nin);
			B.Quad(FVector(P0, H), FVector(P1, H), FVector(Q1, H), FVector(Q0, H), FVector::UpVector);
			B.Quad(FVector(Q0, 0), FVector(Q0, H), FVector(Q1, H), FVector(Q1, 0), Nout);
			const FVector2D M0 = P0 + Out * (T * 0.5), M1 = P1 + Out * (T * 0.5);
			G.Quad(FVector(M0, H), FVector(M1, H), FVector(M1, GH), FVector(M0, GH), Nin, true);
			// Lane A25: the kick plate, a 25 cm band standing 2 cm proud of the inner face at the board foot (front face and top lip; the ring closes itself).
			{
				const double KH = KickPlateHeightM * UU, KP = KickPlateProudM * UU;
				const FVector2D K0 = P0 - Out * KP, K1 = P1 - Out * KP;
				Kick.Quad(FVector(K0, 0), FVector(K1, 0), FVector(K1, KH), FVector(K0, KH), Nin);
				Kick.Quad(FVector(K0, KH), FVector(K1, KH), FVector(P1, KH), FVector(P0, KH), FVector::UpVector);
			}
			// Lane A25: the dark glass rail, 6 cm wide and 4 cm high, capping the glass along the board's middle line.
			Rail.Bar(FVector(M0, GH + 2.0), FVector(M1, GH + 2.0), 3.0, 2.0);
			// R-0002: a 10 cm emissive strip on top of the board, along its inner edge (a box: top, inner and outer faces).
			const FVector2D I0 = P0 + Out * 1.0, I1 = P1 + Out * 1.0, O0 = P0 + Out * 11.0, O1 = P1 + Out * 11.0;
			FMeshBuf& Tr = ((P0.X + P1.X) * 0.5 < 0) ? TrB : TrO;
			// Lane A25: U runs along the outline (cm / 1000, the scale the world-position UVs had on the straight sides), V across or up; no step at the corner seam.
			const double S0 = TrimS, S1 = TrimS + FVector2D::Distance(P0, P1);
			TrimS = S1;
			const double U0 = S0 / 1000.0, U1 = S1 / 1000.0;
			Tr.Quad(FVector(I0, H + 10.0), FVector(I1, H + 10.0), FVector(O1, H + 10.0), FVector(O0, H + 10.0), FVector::UpVector);
			Tr.LastUV(FVector2D(U0, (H + 10.0) / 1000.0), FVector2D(U1, (H + 10.0) / 1000.0), FVector2D(U1, (H + 20.0) / 1000.0), FVector2D(U0, (H + 20.0) / 1000.0));
			Tr.Quad(FVector(I0, H), FVector(I1, H), FVector(I1, H + 10.0), FVector(I0, H + 10.0), Nin);
			Tr.LastUV(FVector2D(U0, H / 1000.0), FVector2D(U1, H / 1000.0), FVector2D(U1, (H + 10.0) / 1000.0), FVector2D(U0, (H + 10.0) / 1000.0));
			Tr.Quad(FVector(O0, H), FVector(O0, H + 10.0), FVector(O1, H + 10.0), FVector(O1, H), Nout);
			Tr.LastUV(FVector2D(U0, H / 1000.0), FVector2D(U0, (H + 10.0) / 1000.0), FVector2D(U1, (H + 10.0) / 1000.0), FVector2D(U1, H / 1000.0));
		}
		B.Commit(Boards, 0);
		Boards->SetMaterial(0, LoadLook(TEXT("MI_Toon_Board"), BoardsMaterialPath));
		G.Commit(Glass, 0);
		// B-to-A-0023 point 2 (lane A31): B's MI_Glass (MI_Bcast_Glass on the broadcast tier); the net placeholder stands in only when B's instance is absent.
		SetGlassForTier(Game.Look.Tier);
		// Lane A25: stanchions, 5 cm square posts from the board top to the rail top (board 1.2 m + glass 0.9 m + the 4 cm rail), one every StanchionSpacingM
		// (3.6 m) along the outline (the count rounds so the ring closes evenly). B-to-A-0023 point 3: these are the glass stanchions; B removes its 66 Con_ actors.
		{
			double Perimeter = 0;
			for (int32 i = 0; i < Line.Num(); ++i) Perimeter += FVector2D::Distance(Line[i], Line[(i + 1) % Line.Num()]);
			const int32 Count = FMath::Max(4, FMath::RoundToInt32(Perimeter / StanchionSpacingM));
			const double Step = Perimeter / Count;
			CapAtM.Reset(Count);      // lane A136: a board cap stands at every stanchion's foot
			CapYawDeg.Reset(Count);
			int32 Seg = 0;
			double SegStart = 0, SegLen = FVector2D::Distance(Line[0], Line[1 % Line.Num()]);
			for (int32 k = 0; k < Count; ++k)
			{
				const double S = k * Step;
				while (S > SegStart + SegLen && Seg < Line.Num() - 1) { SegStart += SegLen; ++Seg; SegLen = FVector2D::Distance(Line[Seg], Line[(Seg + 1) % Line.Num()]); }
				const FVector2D A = Line[Seg], Bp = Line[(Seg + 1) % Line.Num()];
				const FVector2D Dir = (Bp - A).GetSafeNormal();
				const FVector2D Out(Dir.Y, -Dir.X);
				const FVector2D M = (A + Dir * FMath::Clamp(S - SegStart, 0.0, SegLen)) * UU + Out * (T * 0.5);
				Posts.Box(FVector(M.X - 2.5, M.Y - 2.5, H), FVector(M.X + 2.5, M.Y + 2.5, GH + 4.0));
				CapAtM.Add(M / UU);
				CapYawDeg.Add(FMath::RadiansToDegrees(FMath::Atan2(-Out.X, Out.Y)));   // the cap's +y (its `out`) along Out
			}
		}
		Kick.Commit(KickPlate, 0);
		KickPlate->SetMaterial(0, LoadConstructionMat(TEXT("MI_Toon_KickPlate"), TEXT("MI_LL_KickPlate"), FLinearColor(0.85f, 0.72f, 0.12f), 0.6f));
		Rail.Commit(GlassRail, 0);
		Posts.Commit(GlassRail, 1);
		{
			UMaterialInterface* RailMat = LoadConstructionMat(TEXT("MI_Toon_GlassRail"), TEXT("MI_LL_GlassRail"), FLinearColor(0.05f, 0.05f, 0.06f));
			GlassRail->SetMaterial(0, RailMat);
			GlassRail->SetMaterial(1, RailMat);
		}
		TrB.Commit(Trim, 0);
		TrO.Commit(TrimOrange, 0);
		const FLinearColor Blue = Game.Teams.Num() == 2 ? Game.Teams[0].Colour : FLinearColor(0.012f, 0.07f, 0.69f);
		const FLinearColor Orange = Game.Teams.Num() == 2 ? Game.Teams[1].Colour : FLinearColor(0.80f, 0.13f, 0.01f);
		auto TrimMat = [this](const TCHAR* LookName, const FLinearColor& Fallback) -> UMaterialInterface*
		{
			bool bLook = false;
			UMaterialInterface* Mi = LoadLook(LookName, TrimMaterialPath, &bLook);
			if (bLook) return Mi;
			UMaterialInstanceDynamic* D = UMaterialInstanceDynamic::Create(Mi, const_cast<ARinkActor*>(this));
			D->SetVectorParameterValue(TEXT("TeamColour"), Fallback);
			D->SetVectorParameterValue(TEXT("Color"), Fallback);
			D->SetScalarParameterValue(TEXT("Emissive"), 4.f);
			return D;
		};
		// B-to-A-0018 point 5 (lane A21): the two strips are always a dynamic instance so the per-tier Emissive can be set (B's
		// MI_Trim_Blue / MI_Trim_Orange: night 30, clay 12, from the switcher); the placeholder keeps its own 4.
		{
			bool bLookBlue = false, bLookOrange = false;
			UMaterialInterface* BlueBase = LoadLook(TEXT("MI_Trim_Blue"), TrimMaterialPath, &bLookBlue);
			UMaterialInterface* OrangeBase = LoadLook(TEXT("MI_Trim_Orange"), TrimMaterialPath, &bLookOrange);
			bLookTrim = bLookBlue && bLookOrange;
			TrimMid = Cast<UMaterialInstanceDynamic>(bLookBlue ? UMaterialInstanceDynamic::Create(BlueBase, this) : TrimMat(TEXT("MI_Trim_Blue"), Blue));
			TrimOrangeMid = Cast<UMaterialInstanceDynamic>(bLookOrange ? UMaterialInstanceDynamic::Create(OrangeBase, this) : TrimMat(TEXT("MI_Trim_Orange"), Orange));
			Trim->SetMaterial(0, TrimMid);
			TrimOrange->SetMaterial(0, TrimOrangeMid);
			const ULookTierSwitcher* Look = GetWorld() ? GetWorld()->GetSubsystem<ULookTierSwitcher>() : nullptr;
			SetTrimEmissive(Look ? Look->GetTrimEmissiveFor(Game.Look.Tier) : ULookTierSwitcher::DefaultTrimEmissiveFor(Game.Look.Tier));
		}

		// R-0002: far-net beacon posts, d 12 cm, 8 m tall, 1 m behind each net (basic cylinder: 100 cm across and tall).
		BeaconColour = Game.Look.TrimColour;
		for (int32 S = -1; S <= 1; S += 2)
		{
			UStaticMeshComponent* Bc = S > 0 ? BeaconPos : BeaconNeg;
			Bc->SetRelativeLocation(FVector(S * (GL + Depth + 1.0) * UU, 0, 400.0));
			Bc->SetRelativeScale3D(FVector(0.12, 0.12, 8.0));
		}
		ApplyBeaconMode();   // lane A25: material and visibility from jam.rink.beacon
	}

	// Markings: section 0 red (centre line, goal lines, faceoff dots), section 1 blue (blue lines, centre circle).
	{
		const double Hz = R.WidthM * 0.5 * UU, Z = 0.6;
		const double Inset = R.CornerRadiusM; // goal lines stop where the corner starts curving (approximate)
		B.Strip(FVector2D(0, -Hz), FVector2D(0, Hz), 15.0, Z);
		for (int32 S = -1; S <= 1; S += 2)
		{
			const double X = S * GL * UU;
			const double Ext = FMath::Max(Hz - FMath::Max(0.0, Inset - (R.LengthM * 0.5 - GL)) * UU, Hz * 0.6);
			B.Strip(FVector2D(X, -Ext), FVector2D(X, Ext), 3.0, Z);
		}
		B.Commit(Markings, 0);
		FMeshBuf Blue;
		for (int32 S = -1; S <= 1; S += 2) Blue.Strip(FVector2D(S * R.LengthM / 6.0 * UU, -Hz), FVector2D(S * R.LengthM / 6.0 * UU, Hz), 15.0, Z);
		Blue.Ring(FVector2D::ZeroVector, 450.0, 460.0, Z, 64);
		Blue.Commit(Markings, 1);
		// Lane A25: the marks B's painted ice does not carry, always visible: section 2 red (faceoff circles r 4.5 m with four hash marks each, the eight
		// faceoff dots, the crease outlines, the trapezoids behind the nets), section 3 blue (crease fills, the centre dot). Line width 8 cm (the goal line's
		// class, HalfW 4), the blue and centre lines stay at 30 cm. The end-zone dots sit where the old section-0 dots were (x = goal line - 6 m, z = +-min(7, 0.3 w));
		// the neutral-zone dots 1.5 m inside the blue lines; the crease is a 1.8 m half-disc on the goal line; the trapezoid runs from goal width/2 + 1.35 m at the
		// goal line to + 2.25 m at the end boards (clamped before the corner arc).
		FMeshBuf Red2, Blue2;
		{
			const double LW = 6.0, CircleR = 450.0, DotR = 30.0, Hash = 60.0, HashX = 85.0, CreaseR = 180.0;   // 12 cm lines: 8 cm aliased into dots at 24 m (first proof)
			const double DotY = FMath::Min(7.0, R.WidthM * 0.3) * UU;
			for (int32 S = -1; S <= 1; S += 2)
			{
				const double CX = S * (GL - 6.0) * UU, NX = S * (R.LengthM / 6.0 - 1.5) * UU;
				for (int32 Zs = -1; Zs <= 1; Zs += 2)
				{
					const FVector2D C(CX, Zs * DotY);
					Red2.Ring(C, 0.0, DotR, Z, 16);
					Red2.Ring(C, CircleR - LW, CircleR + LW, Z, 64);
					for (int32 Hx = -1; Hx <= 1; Hx += 2) for (int32 Hy = -1; Hy <= 1; Hy += 2)
						Red2.Strip(FVector2D(C.X + Hx * HashX, C.Y + Hy * CircleR), FVector2D(C.X + Hx * HashX, C.Y + Hy * (CircleR + Hash)), LW, Z);
					Red2.Ring(FVector2D(NX, Zs * DotY), 0.0, DotR, Z, 16);
				}
				// Crease: the half-disc faces the centre (towards -x for the +x goal).
				const FVector2D GC(S * GL * UU, 0);
				const double A0 = S > 0 ? UE_DOUBLE_PI * 0.5 : -UE_DOUBLE_PI * 0.5;
				Blue2.Arc(GC, 0.0, CreaseR - LW, A0, A0 + UE_DOUBLE_PI, Z - 0.1, 24);
				Red2.Arc(GC, CreaseR - LW, CreaseR + LW, A0, A0 + UE_DOUBLE_PI, Z, 24);
				// Trapezoid: two lines from the goal line to the end boards, outside the net.
				const double TrapNear = (Hw + 1.35) * UU, TrapFar = FMath::Min((Hw + 2.25) * UU, FMath::Max(TrapNear, Hz - Inset * UU));
				for (int32 Zs = -1; Zs <= 1; Zs += 2)
					Red2.Strip(FVector2D(S * GL * UU, Zs * TrapNear), FVector2D(S * R.LengthM * 0.5 * UU, Zs * TrapFar), LW, Z);
			}
			Blue2.Ring(FVector2D::ZeroVector, 0.0, 15.0, Z, 16);
		}
		Red2.Commit(Markings, 2);
		Blue2.Commit(Markings, 3);
		UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
		UMaterialInstanceDynamic* Red = UMaterialInstanceDynamic::Create(Base, this);
		Red->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.6f, 0.02f, 0.02f));
		UMaterialInstanceDynamic* BlueM = UMaterialInstanceDynamic::Create(Base, this);
		BlueM->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.02f, 0.08f, 0.6f));
		UMaterialInstanceDynamic* PaleBlue = UMaterialInstanceDynamic::Create(Base, this);
		PaleBlue->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.35f, 0.55f, 0.85f));
		Markings->SetMaterial(0, Red);
		Markings->SetMaterial(1, BlueM);
		Markings->SetMaterial(2, Red);
		Markings->SetMaterial(3, PaleBlue);
		// B's ice material paints the centre, blue and goal lines and the centre circle itself from RinkHalfLength / RinkHalfWidth; ours would double them.
		// The construction sections (2, 3) stay, and follow jam.rink.construction.
		Markings->SetVisibility(true);
		Markings->SetMeshSectionVisible(0, !bLookIce);
		Markings->SetMeshSectionVisible(1, !bLookIce);
	}

	// Zones (C-0005 / A15): flat decals at 0.8 cm (above the markings at 0.6), one fill section and one rim section per zone kind,
	// in the order the kinds first appear. Circle = disc + 25 cm rim ring; rect = quad + 25 cm border of four strips.
	int32 ZonesDrawn = 0;
	int32 ZoneQuadsDrawn = 0;
	{
		// B-to-A-0016 (lane A21): a zone whose MI_Zone_<kind> exists becomes one engine plane (100 x 100 cm, UVs 0..1) scaled to the
		// zone's bounding box, 0.2 cm above the ice, with a dynamic instance carrying Circle (1 circle / 0 rect) and Strength. No shadow,
		// no collision, stencil 0. A kind without its instance on this machine falls through to A's decal below, so play never waits on an asset.
		for (UStaticMeshComponent* Q : ZoneQuads) if (Q) Q->DestroyComponent();
		ZoneQuads.Reset();
		for (UProceduralMeshComponent* Rg : ZoneRings) if (Rg) Rg->DestroyComponent();
		ZoneRings.Reset();
		UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));   // engine content, cached after the first load; not held across GC
		TMap<FName, UMaterialInterface*> ZoneLooks;   // kind -> B's instance (null = missing, looked up once per Build)
		auto ZoneLookFor = [this, &ZoneLooks](const FName& Kind) -> UMaterialInterface*
		{
			if (UMaterialInterface** Found = ZoneLooks.Find(Kind)) return *Found;
			bool bLook = false;
			UMaterialInterface* Mi = LoadLook(*FString::Printf(TEXT("MI_Zone_%s"), *Kind.ToString()), TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"), &bLook);
			if (!bLook)
			{
				static TSet<FName> Logged;
				if (!Logged.Contains(Kind)) { Logged.Add(Kind); UE_LOG(LogJamSim, Log, TEXT("JAMRINK zone look /JamAssets/Look/MI_Zone_%s missing (logged once): A's decal stands in"), *Kind.ToString()); }
			}
			ZoneLooks.Add(Kind, bLook ? Mi : nullptr);
			return bLook ? Mi : nullptr;
		};

		struct FZoneKindBuf { FName Kind; FMeshBuf Fill, Rim; double StrengthSum = 0; int32 Count = 0; };
		TArray<FZoneKindBuf> Kinds;
		const double ZH = 0.8, RimW = 25.0;
		for (const FJamZoneConfig& Zn : R.Zones)
		{
			if (!Zn.bVisible) continue;
			const bool bCircle = Zn.Shape != FName(TEXT("rect"));
			if (UMaterialInterface* Look = PlaneMesh ? ZoneLookFor(Zn.Kind) : nullptr)
			{
				UStaticMeshComponent* Q = NewObject<UStaticMeshComponent>(this);   // unique auto name: a rebuild never clashes with a quad that is still being destroyed
				Q->SetupAttachment(RootComponent);
				Q->SetMobility(EComponentMobility::Movable);
				Q->SetStaticMesh(PlaneMesh);
				Q->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				Q->SetCastShadow(false);
				Q->SetRenderCustomDepth(false);
				Q->SetCustomDepthStencilValue(0);
				const double SizeX = bCircle ? Zn.R * 2.0 : Zn.W, SizeY = bCircle ? Zn.R * 2.0 : Zn.D;   // the plane is 1 m, so metres are the scale
				Q->SetRelativeLocation(FVector(Zn.X * UU, Zn.Z * UU, 0.2));
				Q->SetRelativeScale3D(FVector(FMath::Max(SizeX, 0.01), FMath::Max(SizeY, 0.01), 1.0));
				UMaterialInstanceDynamic* D = UMaterialInstanceDynamic::Create(Look, this);
				D->SetScalarParameterValue(TEXT("Circle"), bCircle ? 1.f : 0.f);
				D->SetScalarParameterValue(TEXT("Strength"), static_cast<float>(FMath::Clamp(Zn.Strength, 0.0, 1.0)));
				Q->SetMaterial(0, D);
				Q->RegisterComponent();
				ZoneQuads.Add(Q);
				// Lane A69: the clay tier's ring for this zone (same index as the quad). A flat ring 6 cm high and 12 cm wide centred on the outline; a circle is
				// 64 segments with an outer wall, an inner wall and a top, a rect is four boxes. UVs are the quad's (0..1 over the zone's bounding box) so the
				// material reads the same mask. Its own dynamic instance of the same MI_Zone_<kind>; hidden until the tier says clay.
				{
					constexpr double RingH = 6.0, RingHalfW = 15.0;   // A69 measurement: 12 cm read as a 2 px wire at 10 m (eyes), 30 cm reads as a ring
					FMeshBuf RB;
					const double Cx = Zn.X * UU, Cy = Zn.Z * UU;
					if (bCircle)
					{
						const double Ro = Zn.R * UU + RingHalfW, Ri = FMath::Max(0.0, Zn.R * UU - RingHalfW);
						constexpr int32 Seg = 64;
						for (int32 i = 0; i < Seg; ++i)
						{
							const double A0 = 2.0 * UE_DOUBLE_PI * i / Seg, A1 = 2.0 * UE_DOUBLE_PI * (i + 1) / Seg;
							const FVector2D D0(FMath::Cos(A0), FMath::Sin(A0)), D1(FMath::Cos(A1), FMath::Sin(A1));
							const FVector2D C(Cx, Cy);
							RB.Quad(FVector(C + D0 * Ri, RingH), FVector(C + D0 * Ro, RingH), FVector(C + D1 * Ro, RingH), FVector(C + D1 * Ri, RingH), FVector::UpVector);
							RB.Quad(FVector(C + D0 * Ro, 0), FVector(C + D1 * Ro, 0), FVector(C + D1 * Ro, RingH), FVector(C + D0 * Ro, RingH), FVector((D0 + D1).GetSafeNormal(), 0));
							RB.Quad(FVector(C + D1 * Ri, 0), FVector(C + D0 * Ri, 0), FVector(C + D0 * Ri, RingH), FVector(C + D1 * Ri, RingH), FVector(-(D0 + D1).GetSafeNormal(), 0));
						}
					}
					else
					{
						const double Hx = Zn.W * 0.5 * UU, Hy = Zn.D * 0.5 * UU, RHw = RingHalfW;
						RB.Box(FVector(Cx - Hx - RHw, Cy - Hy - RHw, 0), FVector(Cx + Hx + RHw, Cy - Hy + RHw, RingH));
						RB.Box(FVector(Cx - Hx - RHw, Cy + Hy - RHw, 0), FVector(Cx + Hx + RHw, Cy + Hy + RHw, RingH));
						RB.Box(FVector(Cx - Hx - RHw, Cy - Hy + RHw, 0), FVector(Cx - Hx + RHw, Cy + Hy - RHw, RingH));
						RB.Box(FVector(Cx + Hx - RHw, Cy - Hy + RHw, 0), FVector(Cx + Hx + RHw, Cy + Hy - RHw, RingH));
					}
					const double MX = FMath::Max(SizeX, 0.01), MY = FMath::Max(SizeY, 0.01);
					// The material reads the zone's size from the component's scale as well as from the UVs (the decal quad is scaled to (w, d, 1) m), so the ring is
					// built in the quad's local frame: centred, divided by the scale, and given the same scale below.
					for (int32 u = 0; u < RB.V.Num(); ++u)
					{
						RB.UV[u] = FVector2D((RB.V[u].X - Cx) / (MX * UU) + 0.5, (RB.V[u].Y - Cy) / (MY * UU) + 0.5);
						RB.V[u] = FVector((RB.V[u].X - Cx) / MX, (RB.V[u].Y - Cy) / MY, RB.V[u].Z);
					}
					UProceduralMeshComponent* Rg = NewObject<UProceduralMeshComponent>(this);
					Rg->SetupAttachment(RootComponent);
					Rg->SetMobility(EComponentMobility::Movable);
					Rg->SetRelativeLocation(FVector(Cx, Cy, 0.0));
					Rg->SetRelativeScale3D(FVector(MX, MY, 1.0));
					Rg->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					Rg->SetCastShadow(false);
					Rg->SetRenderCustomDepth(false);
					Rg->SetCustomDepthStencilValue(0);
					RB.Commit(Rg, 0);
					// A-to-B-0015 (B29): B's opaque lit ring instance MI_Zone_<kind>_Ring (matte clay body, soft emissive; reads neither scale nor UVs) when it exists;
					// else the zone's own MI_Zone_<kind> as A69 shipped it. Circle / Strength do not exist on the ring master: setting them is harmless.
					bool bRingLook = false;
					UMaterialInterface* RingBase = LoadLook(*FString::Printf(TEXT("MI_Zone_%s_Ring"), *Zn.Kind.ToString()), TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"), &bRingLook);
					if (!bRingLook || !RingBase) RingBase = Look;
					UMaterialInstanceDynamic* RD = UMaterialInstanceDynamic::Create(RingBase, this);
					RD->SetScalarParameterValue(TEXT("Circle"), bCircle ? 1.f : 0.f);
					RD->SetScalarParameterValue(TEXT("Strength"), static_cast<float>(FMath::Clamp(Zn.Strength, 0.0, 1.0)));
					Rg->SetMaterial(0, RD);
					Rg->SetVisibility(false);
					Rg->RegisterComponent();
					ZoneRings.Add(Rg);
				}
				++ZoneQuadsDrawn; ++ZonesDrawn;
				continue;
			}
			FZoneKindBuf* K = Kinds.FindByPredicate([&Zn](const FZoneKindBuf& E) { return E.Kind == Zn.Kind; });
			if (!K) { K = &Kinds.AddDefaulted_GetRef(); K->Kind = Zn.Kind; }
			K->StrengthSum += Zn.Strength; ++K->Count; ++ZonesDrawn;
			const FVector2D C(Zn.X * UU, Zn.Z * UU);
			if (Zn.Shape == FName(TEXT("rect")))
			{
				const double Hx = Zn.W * 0.5 * UU, Hy = Zn.D * 0.5 * UU, Hb = RimW * 0.5;
				const double X0 = C.X - Hx, X1 = C.X + Hx, Y0 = C.Y - Hy, Y1 = C.Y + Hy;
				K->Fill.Quad(FVector(X0, Y0, ZH), FVector(X1, Y0, ZH), FVector(X1, Y1, ZH), FVector(X0, Y1, ZH), FVector::UpVector);
				// Border strips run the full edge length, centred Hb inside it, so the corners are covered.
				K->Rim.Strip(FVector2D(X0, Y0 + Hb), FVector2D(X1, Y0 + Hb), Hb, ZH + 0.1);
				K->Rim.Strip(FVector2D(X0, Y1 - Hb), FVector2D(X1, Y1 - Hb), Hb, ZH + 0.1);
				K->Rim.Strip(FVector2D(X0 + Hb, Y0), FVector2D(X0 + Hb, Y1), Hb, ZH + 0.1);
				K->Rim.Strip(FVector2D(X1 - Hb, Y0), FVector2D(X1 - Hb, Y1), Hb, ZH + 0.1);
			}
			else
			{
				const double Rad = Zn.R * UU;
				K->Fill.Ring(C, 0.0, Rad, ZH, 64);
				K->Rim.Ring(C, FMath::Max(0.0, Rad - RimW), Rad, ZH + 0.1, 64);
			}
		}
		auto ZoneTint = [](const FName& Kind) -> FLinearColor
		{
			if (Kind == FName(TEXT("slow_ice"))) return FLinearColor(0.35f, 0.55f, 0.95f);
			if (Kind == FName(TEXT("fast_ice"))) return FLinearColor(0.95f, 0.45f, 0.15f);
			if (Kind == FName(TEXT("bumper"))) return FLinearColor(0.9f, 0.2f, 0.6f);
			if (Kind == FName(TEXT("no_shoot"))) return FLinearColor(0.85f, 0.15f, 0.15f);
			if (Kind == FName(TEXT("boost_only"))) return FLinearColor(0.2f, 0.85f, 0.4f);
			return FLinearColor(0.6f, 0.6f, 0.6f); // unknown kind: neutral grey rather than nothing
		};
		Zones->ClearAllMeshSections();
		for (int32 k = 0; k < Kinds.Num(); ++k)
		{
			FZoneKindBuf& K = Kinds[k];
			// Brighter for a stronger zone: tint * (0.5 + 0.5 * Strength), Strength averaged over this kind's zones (one material per section).
			const float Scale = static_cast<float>(0.5 + 0.5 * (K.Count ? K.StrengthSum / K.Count : 0.5));
			const FLinearColor Base = ZoneTint(K.Kind);
			const FLinearColor FillTint(Base.R * Scale, Base.G * Scale, Base.B * Scale);
			const FLinearColor RimTint(FillTint.R * 1.35f, FillTint.G * 1.35f, FillTint.B * 1.35f);
			bool bLook = false;
			UMaterialInterface* LookMat = LoadLook(*FString::Printf(TEXT("MI_Zone_%s"), *K.Kind.ToString()), TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"), &bLook);
			auto Tinted = [this, bLook, LookMat](const FLinearColor& Tint) -> UMaterialInterface*
			{
				if (bLook) return LookMat;
				UMaterialInstanceDynamic* D = UMaterialInstanceDynamic::Create(LookMat, this);
				D->SetVectorParameterValue(TEXT("Color"), Tint);
				return D;
			};
			K.Fill.Commit(Zones, 2 * k);
			Zones->SetMaterial(2 * k, Tinted(FillTint));
			K.Rim.Commit(Zones, 2 * k + 1);
			Zones->SetMaterial(2 * k + 1, Tinted(RimTint));
		}
	}

	// Nets: posts and crossbar (frame, section 0) and the mesh (section 1), open towards the centre.
	{
		// Lane A25 (LOOK-LAB.md recommendation 6): 14 cm tubular frame (posts, crossbar, side and back bottom bars: section 0, the box the sim's goal volume
		// matches) plus the back frame (two back posts to 0.9 m, a back bar, two braces from the crossbar ends: section 2, hidden with the construction);
		// the netting (section 1) drapes over it: sloped top from the crossbar to the back bar, vertical back below it, two side panels.
		FMeshBuf Mesh, Extra;
		const double PR = NetBarM * 0.5 * UU; // post half size, cm
		const double BR = 5.0;                // brace half size, cm
		for (int32 S = -1; S <= 1; S += 2)
		{
			const double X0 = S * GL * UU, X1 = S * (GL + Depth) * UU;
			// The back frame stands at 0.9 m under a 1.5 m crossbar; with a lower opening it keeps that proportion (never above the crossbar).
			const double Y0 = -Hw * UU, Y1 = Hw * UU, Hh = NetH * UU, Bh = FMath::Min(NetBackHeightM, NetH * (NetBackHeightM / 1.5)) * UU;
			B.Box(FVector(X0 - PR, Y0 - PR, 0), FVector(X0 + PR, Y0 + PR, Hh));
			B.Box(FVector(X0 - PR, Y1 - PR, 0), FVector(X0 + PR, Y1 + PR, Hh));
			B.Box(FVector(X0 - PR, Y0 - PR, Hh - PR * 2), FVector(X0 + PR, Y1 + PR, Hh));
			B.Box(FVector(FMath::Min(X0, X1) - PR, Y0 - PR, 0), FVector(FMath::Max(X0, X1) + PR, Y0 + PR, 2 * PR));
			B.Box(FVector(FMath::Min(X0, X1) - PR, Y1 - PR, 0), FVector(FMath::Max(X0, X1) + PR, Y1 + PR, 2 * PR));
			B.Box(FVector(X1 - PR, Y0 - PR, 0), FVector(X1 + PR, Y1 + PR, 2 * PR));
			Extra.Box(FVector(X1 - PR, Y0 - PR, 0), FVector(X1 + PR, Y0 + PR, Bh));
			Extra.Box(FVector(X1 - PR, Y1 - PR, 0), FVector(X1 + PR, Y1 + PR, Bh));
			Extra.Box(FVector(X1 - PR, Y0 - PR, Bh - PR * 2), FVector(X1 + PR, Y1 + PR, Bh));
			Extra.Bar(FVector(X0, Y0, Hh - PR), FVector(X1, Y0, Bh - PR), BR, BR);
			Extra.Bar(FVector(X0, Y1, Hh - PR), FVector(X1, Y1, Bh - PR), BR, BR);
			const FVector In(-S, 0, 0);
			Mesh.Quad(FVector(X1, Y0, 0), FVector(X1, Y1, 0), FVector(X1, Y1, Bh), FVector(X1, Y0, Bh), In, true);
			Mesh.Quad(FVector(X0, Y0, 0), FVector(X1, Y0, 0), FVector(X1, Y0, Bh), FVector(X0, Y0, Hh), FVector(0, 1, 0), true);
			Mesh.Quad(FVector(X0, Y1, 0), FVector(X1, Y1, 0), FVector(X1, Y1, Bh), FVector(X0, Y1, Hh), FVector(0, -1, 0), true);
			Mesh.Quad(FVector(X0, Y0, Hh), FVector(X1, Y0, Bh), FVector(X1, Y1, Bh), FVector(X0, Y1, Hh), FVector(0, 0, -1), true);
		}
		B.Commit(Nets, 0);
		Mesh.Commit(Nets, 1);
		Extra.Commit(Nets, 2);
		UMaterialInterface* FrameMat = LoadConstructionMat(TEXT("MI_Toon_NetFrame"), TEXT("MI_LL_NetFrame"), FLinearColor(0.7f, 0.03f, 0.02f));
		Nets->SetMaterial(0, FrameMat);
		Nets->SetMaterial(1, LoadLook(TEXT("MI_Toon_NetMesh"), NetMaterialPath));
		Nets->SetMaterial(2, FrameMat);

		// Lane A25: a goal light on the end-board rail behind each net: a dark housing 30 x 24 x 20 cm on the rail, a red lens on its rink face.
		{
			FMeshBuf Housing, Lens;
			// Lane A47 (B-to-A-0030 fault 10): the housing sat 4 cm above the rail and read as floating over the glass on every tier; it now straddles the
			// rail top with its base 6 cm below the glass line, clamped onto the glass.
			// Lane A64 (eyes grid h19 finding 5, 10 stills): the 40 x 30 x 30 cm box on the rail still read as a floating cube. The housing is now a low
			// clamp that wraps the glass top rail (the rail bar is 6 x 4 cm at GH + 2), and the light a red bar lying along the rail on top of it:
			// one line with the glass, no cube.
			const double GH = (BoardHeightM + GlassHeightM) * UU;
			for (int32 S = -1; S <= 1; S += 2)
			{
				const double Mx = S * (R.LengthM * 0.5 * UU + BoardThicknessM * 0.5 * UU);
				Housing.Box(FVector(Mx - 6.0, -40.0, GH - 4.0), FVector(Mx + 6.0, 40.0, GH + 8.0));
				Lens.Box(FVector(Mx - 4.0, -34.0, GH + 8.0), FVector(Mx + 4.0, 34.0, GH + 16.0));
			}
			Housing.Commit(GoalLights, 0);
			Lens.Commit(GoalLights, 1);
			// Mid-grey lifted housing: a black box on the black backdrop behind the end boards is invisible (first proof); B's MI_Toon_GoalLight replaces it.
			GoalLights->SetMaterial(0, LoadConstructionMat(TEXT("MI_Toon_GoalLight"), TEXT("MI_LL_GlassRail"), FLinearColor(0.45f, 0.45f, 0.48f), 1.2f, true));
			GoalLights->SetMaterial(1, FrameMat);
		}
	}

	// Lane A25: benches outside the +y boards (Unreal +Y is the rink's z axis), crowdless: two team benches and a shorter penalty bench between them, each a
	// board-coloured enclosure (back wall 1.1 m, two end walls; section 0) with an ochre seat slab (section 1). Lengths scale with the rink: team bench 0.1 L
	// centred at +-0.0833 L, penalty bench 0.0533 L at the centre with a divider. 90 x 45: 9 m benches at +-7.5 m, 4.8 m penalty bench.
	{
		FMeshBuf Walls, Seats;
		const double Y0 = (R.WidthM * 0.5 + BoardThicknessM) * UU + 5.0, Depth1 = 110.0, Wall = 10.0, Hgt = 110.0;
		auto Bench = [&](double XC, double Len, bool bDivider)
		{
			const double X0 = XC - Len * 0.5, X1 = XC + Len * 0.5;
			Walls.Box(FVector(X0, Y0 + Depth1 - Wall, 0), FVector(X1, Y0 + Depth1, Hgt));
			Walls.Box(FVector(X0, Y0, 0), FVector(X0 + Wall, Y0 + Depth1, Hgt));
			Walls.Box(FVector(X1 - Wall, Y0, 0), FVector(X1, Y0 + Depth1, Hgt));
			if (bDivider) Walls.Box(FVector(XC - Wall * 0.5, Y0, 0), FVector(XC + Wall * 0.5, Y0 + Depth1, Hgt));
			Seats.Box(FVector(X0 + Wall, Y0 + 40.0, 40.0), FVector(X1 - Wall, Y0 + Depth1 - Wall, 48.0));
			Seats.Box(FVector(X0 + Wall, Y0 + Depth1 - Wall - 8.0, 48.0), FVector(X1 - Wall, Y0 + Depth1 - Wall, 85.0));
		};
		const double L = R.LengthM * UU;
		Bench(-L * 0.0833, L * 0.1, false);
		Bench(L * 0.0833, L * 0.1, false);
		Bench(0, L * 0.0533, true);
		Walls.Commit(Benches, 0);
		Seats.Commit(Benches, 1);
		// B's bench instance when it ships (MI_Toon_Bench); else the lab flat master in a warm grey with a lift (the outer side of the boards is unlit at night).
		// Lift 1.2: at 0.5 an unlit stencil-1 surface quantises into the ramp's floor and reads black (second proof); the net frame at 2 reads everywhere.
		Benches->SetMaterial(0, LoadConstructionMat(TEXT("MI_Toon_Bench"), TEXT("MI_LL_KickPlate"), FLinearColor(0.62f, 0.60f, 0.56f), 1.2f, true));
		Benches->SetMaterial(1, LoadConstructionMat(TEXT("MI_Toon_KickPlate"), TEXT("MI_LL_KickPlate"), FLinearColor(0.85f, 0.72f, 0.12f), 1.2f));
	}

	// Lane A103 (track F3): the Blender-made dressing. Two team benches outside the +y boards at A25's places (centres +-0.0833 L, the 12 m mesh scaled along the
	// boards to 0.1 L), two penalty boxes outside the -y boards at x = -3 and +3 (UJamSim::BoxPos), turned 180 degrees so they open onto the rink. Each piece's
	// origin is the middle of its rink-side edge on the ice, 5 cm clear of the board's outer face. Materials are device B's instances by reference, through the
	// same loaders as the code construction (B's instance, else the LookLab stand-in, else the engine's basic material tinted); the `team` slot follows the half:
	// blue for x < 0 (team 0), orange for x > 0. A missing mesh leaves the component empty: ApplyArenaDressing then keeps A25's code benches.
	{
		UStaticMesh* BenchMesh = LoadObject<UStaticMesh>(nullptr, TeamBenchMeshPath, nullptr, LOAD_NoWarn | LOAD_Quiet);
		UStaticMesh* BoxMesh = LoadObject<UStaticMesh>(nullptr, PenaltyBoxMeshPath, nullptr, LOAD_NoWarn | LOAD_Quiet);
		const double Y0 = (R.WidthM * 0.5 + BoardThicknessM + ArenaGapM) * UU;
		const double BenchScale = R.LengthM * ArenaBenchLengthFactor / ArenaBenchBuiltLengthM;
		const double BenchX = R.LengthM * ArenaBenchCentreFactor * UU;
		auto Dress = [this](UStaticMeshComponent* C, UStaticMesh* Mesh, const FVector& Loc, double Yaw, double ScaleX, bool bOrange)
		{
			if (!C) return;
			C->SetStaticMesh(Mesh);
			if (!Mesh) return;
			C->SetRelativeLocationAndRotation(Loc, FRotator(0.0, Yaw, 0.0));
			C->SetRelativeScale3D(FVector(ScaleX, 1.0, 1.0));
			auto Bind = [C](const TCHAR* Slot, UMaterialInterface* Mat) { const int32 I = C->GetMaterialIndex(FName(Slot)); if (I != INDEX_NONE && Mat) C->SetMaterial(I, Mat); };
			Bind(TEXT("board"), LoadLook(TEXT("MI_Toon_Board"), BoardsMaterialPath));
			Bind(TEXT("kick"), LoadConstructionMat(TEXT("MI_Toon_KickPlate"), TEXT("MI_LL_KickPlate"), FLinearColor(0.85f, 0.72f, 0.12f), 1.2f));
			Bind(TEXT("bench"), LoadConstructionMat(TEXT("MI_Toon_Bench"), TEXT("MI_LL_KickPlate"), FLinearColor(0.62f, 0.60f, 0.56f), 1.2f, true));
			Bind(TEXT("team"), bOrange
				? LoadConstructionMat(TEXT("MI_Toon_Orange"), TEXT("MI_LL_NetFrame"), FLinearColor(0.95f, 0.45f, 0.08f), 1.2f, true)
				: LoadConstructionMat(TEXT("MI_Toon_Blue"), TEXT("MI_LL_NetFrame"), FLinearColor(0.05f, 0.25f, 0.85f), 1.2f, true));
			Bind(TEXT("dark"), LoadConstructionMat(TEXT("MI_Toon_Dark"), TEXT("MI_LL_GlassRail"), FLinearColor(0.06f, 0.06f, 0.07f)));
		};
		Dress(ArenaBenchNeg, BenchMesh, FVector(-BenchX, Y0, 0.0), 0.0, BenchScale, false);
		Dress(ArenaBenchPos, BenchMesh, FVector(BenchX, Y0, 0.0), 0.0, BenchScale, true);
		Dress(ArenaBoxNeg, BoxMesh, FVector(-PenaltyBoxCentreXM * UU, -Y0, 0.0), 180.0, 1.0, false);
		Dress(ArenaBoxPos, BoxMesh, FVector(PenaltyBoxCentreXM * UU, -Y0, 0.0), 180.0, 1.0, true);
		// Lane A145 (evolution 5): the box's door and its clock's long hand, meshes of their own hung on the box mesh's sockets `Door` (the hinge's foot) and
		// `Hand` (the clock's middle). A box mesh without the sockets (the one before the lane, door and hand built in) leaves them empty: nothing shows twice.
		{
			UStaticMesh* DoorMesh = LoadObject<UStaticMesh>(nullptr, BoxDoorMeshPath, nullptr, LOAD_NoWarn | LOAD_Quiet);
			UStaticMesh* HandMesh = LoadObject<UStaticMesh>(nullptr, BoxHandMeshPath, nullptr, LOAD_NoWarn | LOAD_Quiet);
			UStaticMeshComponent* const Use[4] = { ArenaBoxDoorNeg.Get(), ArenaBoxDoorPos.Get(), ArenaBoxHandNeg.Get(), ArenaBoxHandPos.Get() };
			int32 Hung = 0;
			for (int32 i = 0; i < 4; ++i)
			{
				const UStaticMeshSocket* Sock = BoxMesh ? BoxMesh->FindSocket(FName(i < 2 ? TEXT("Door") : TEXT("Hand"))) : nullptr;
				UStaticMesh* Mesh = Sock ? (i < 2 ? DoorMesh : HandMesh) : nullptr;
				Dress(Use[i], Mesh, Sock ? Sock->RelativeLocation : FVector::ZeroVector, 0.0, 1.0, i % 2 == 1);
				if (Mesh && Use[i]) ++Hung;
			}
			PenaltyFramesCfg = Game.Rules.PenaltyFrames > 0 ? Game.Rules.PenaltyFrames : 300;
			TeamSplit = Game.Teams.Num() > 0 ? Game.Teams[0].Skaters.Num() : 0;
			// Lane A160: the cut's whistle (the boxes stand on a bare tier only with it), and each box's light: over the seat, a step towards the boards.
			bWhistle = Game.Rules.bPenalties;
			{
				const UStaticMeshSocket* SeatSock = BoxMesh ? BoxMesh->FindSocket(FName(TEXT("Seat"))) : nullptr;
				const FVector SeatLocal = SeatSock ? SeatSock->RelativeLocation : FVector(0.0, 90.0, 0.0);
				UStaticMeshComponent* const Boxes[2] = { ArenaBoxNeg.Get(), ArenaBoxPos.Get() };
				for (int32 t = 0; t < 2; ++t)
				{
					if (!ArenaBoxLights[t] || !Boxes[t]) continue;
					// the box's own +y points away from the rink (the piece is turned 180 degrees): towards the boards is its -y
					ArenaBoxLights[t]->SetRelativeLocation(Boxes[t]->GetRelativeTransform().TransformPosition(SeatLocal + FVector(0.0, -BoxLightTowardsRinkM * UU, BoxLightHeightM * UU)));
					ArenaBoxLights[t]->SetAttenuationRadius(BoxLightRadiusCm);
					ArenaBoxLights[t]->SetIntensity(0.f);
					ArenaBoxLights[t]->SetVisibility(false);
				}
			}
			for (FBoxUse& U : BoxUse) U = FBoxUse();
			BoxSeenAt[0] = BoxSeenAt[1] = -1;
			UE_LOG(LogJamSim, Log, TEXT("JAMRINK arena box use: door mesh %s, hand mesh %s, %d of 4 hung on the boxes' sockets; a penalty is %d frames, team 0 is skaters 0..%d"),
				DoorMesh ? TEXT("loaded") : TEXT("missing"), HandMesh ? TEXT("loaded") : TEXT("missing"), Hung, PenaltyFramesCfg, TeamSplit - 1);
		}
		// Lane A119: the scorer's table between the two boxes (their inner ends stand 1.48 m apart on every rink), and the goals on the goal lines.
		UStaticMesh* TableMesh = LoadObject<UStaticMesh>(nullptr, ScorerTableMeshPath, nullptr, LOAD_NoWarn | LOAD_Quiet);
		UMaterialInterface* FrameMat = LoadConstructionMat(TEXT("MI_Toon_NetFrame"), TEXT("MI_LL_NetFrame"), FLinearColor(0.7f, 0.03f, 0.02f));
		auto BindSlot = [](UStaticMeshComponent* C, const TCHAR* Slot, UMaterialInterface* Mat) { if (!C || !C->GetStaticMesh() || !Mat) return; const int32 I = C->GetMaterialIndex(FName(Slot)); if (I != INDEX_NONE) C->SetMaterial(I, Mat); };
		Dress(ArenaTable, TableMesh, FVector(0.0, -Y0, 0.0), 180.0, 1.0, false);
		BindSlot(ArenaTable, TEXT("frame"), FrameMat);
		// Lane A127 (evolution 3): the goal lights, one outside the end boards behind each net (the piece's +y points away from the rink, like the benches'),
		// and the lamps. A lamp is the mesh's slot `lamp` on a dynamic instance of the net frame's red (device B's MI_Toon_NetFrame: its master M_Toon exposes
		// the scalar Emissive, read here as the lamp's resting value). A material without Emissive is bound as it is and that lamp never lights.
		UStaticMesh* LightMesh = LoadObject<UStaticMesh>(nullptr, GoalLightMeshPath, nullptr, LOAD_NoWarn | LOAD_Quiet);
		const double EndX = (R.LengthM * 0.5 + BoardThicknessM + ArenaGapM) * UU;
		Dress(ArenaGoalLightNeg, LightMesh, FVector(-EndX, 0.0, 0.0), 90.0, 1.0, false);
		Dress(ArenaGoalLightPos, LightMesh, FVector(EndX, 0.0, 0.0), -90.0, 1.0, true);
		// Lane A136 (evolution 4): the board caps. One instance at every stanchion place on the boards' middle line, its origin on the board top, turned so its
		// overhang points away from the rink; each a little different in size and turn (from its index: no random source). B's MI_Toon_KickPlate by reference.
		{
			// Lane A145 (evolution 5): the cap is a dome (40 triangles) in the team colour of its half; a rink whose caps would pass CapTriangleBudget as
			// domes takes the far level, lane A136's 24-triangle lump (one level for the whole ring: an instanced component draws one).
			UStaticMesh* RoundMesh = LoadObject<UStaticMesh>(nullptr, BoardCapMeshPath, nullptr, LOAD_NoWarn | LOAD_Quiet);
			UStaticMesh* FarMesh = LoadObject<UStaticMesh>(nullptr, BoardCapFarMeshPath, nullptr, LOAD_NoWarn | LOAD_Quiet);
			bCapsRound = RoundMesh && (!FarMesh || CapAtM.Num() * RoundMesh->GetNumTriangles(0) <= CapTriangleBudget);
			UStaticMesh* CapMesh = bCapsRound ? RoundMesh : (FarMesh ? FarMesh : RoundMesh);
			for (FCapRipple& Rp : CapRipple) Rp = FCapRipple();
			CapNow.Reset();
			CapRest.Reset();
			CapSide.Reset();
			CapInst.Reset();
			bCapsLive = false;
			UInstancedStaticMeshComponent* const CapComps[2] = { ArenaCaps.Get(), ArenaCapsPos.Get() };
			if (CapComps[0] && CapComps[1])
			{
				TArray<FTransform> Half[2];
				for (int32 s = 0; s < 2; ++s)
				{
					CapComps[s]->ClearInstances();
					CapComps[s]->SetStaticMesh(CapMesh);
				}
				if (CapMesh)
				{
					UMaterialInterface* const TeamMat[2] = { LoadConstructionMat(TEXT("MI_Toon_Blue"), TEXT("MI_LL_NetFrame"), FLinearColor(0.05f, 0.25f, 0.85f), 1.2f, true),
						LoadConstructionMat(TEXT("MI_Toon_Orange"), TEXT("MI_LL_NetFrame"), FLinearColor(0.95f, 0.45f, 0.08f), 1.2f, true) };
					for (int32 s = 0; s < 2; ++s)
					{
						const int32 KickSlot = CapComps[s]->GetMaterialIndex(FName(TEXT("kick")));
						if (KickSlot != INDEX_NONE && TeamMat[s]) CapComps[s]->SetMaterial(KickSlot, TeamMat[s]);
					}
					for (int32 k = 0; k < CapAtM.Num(); ++k)
					{
						uint32 Hh = static_cast<uint32>(k + 1) * 2654435761u;
						Hh ^= Hh >> 15;
						const double Size = 0.90 + 0.16 * ((Hh & 0xffu) / 255.0);   // at 1.06 the cap still ends at the board's inner face
						const double Turn = -9.0 + 18.0 * (((Hh >> 8) & 0xffu) / 255.0);
						const int32 Side = CapAtM[k].X >= 0.0 ? 1 : 0;
						CapRest.Add(FTransform(FRotator(0.0, CapYawDeg[k] + Turn, 0.0), FVector(CapAtM[k].X * UU, CapAtM[k].Y * UU, BoardHeightM * UU), FVector(Size)));
						CapSide.Add(static_cast<uint8>(Side));
						CapInst.Add(Half[Side].Num());
						Half[Side].Add(CapRest.Last());
					}
					for (int32 s = 0; s < 2; ++s) if (Half[s].Num() > 0) CapComps[s]->AddInstances(Half[s], false, false);
				}
			}
			UE_LOG(LogJamSim, Log, TEXT("JAMRINK arena caps: mesh %s, %d caps on the board top (one every %.2f m of the ring), %d triangles in all, %s in the team colours; jam.rink.caps %d"),
				CapMesh ? TEXT("loaded") : TEXT("missing"), CapRest.Num(), CapRest.Num() > 0 ? 2.0 * (R.LengthM + R.WidthM - (4.0 - UE_DOUBLE_PI) * FMath::Clamp(R.CornerRadiusM, 0.0, FMath::Min(R.LengthM, R.WidthM) * 0.5)) / CapRest.Num() : 0.0,
				GetCapTriangles(), bCapsRound ? TEXT("the dome") : TEXT("the far level"), AreCapsEnabled() ? 1 : 0);
		}
		{
			// Lane A172: device B's lamp instance on the slot `lamp` (the net frame's red stays on the table's slot `frame` and on the goals).
			UMaterialInterface* LampMat = FrameMat;
			bLampTrim = false;
			if (CVarRinkLampTrim.GetValueOnGameThread() != 0)
			{
				if (UMaterialInterface* TrimLamp = LoadObject<UMaterialInterface>(nullptr, LampTrimPath, nullptr, LOAD_NoWarn | LOAD_Quiet)) { LampMat = TrimLamp; bLampTrim = true; }
			}
			LampPeakAdd = bLampTrim ? LampTrimPeak - LampTrimRest : LampPeakEmissive;
			UStaticMeshComponent* const LampOn[3] = { ArenaTable.Get(), ArenaGoalLightNeg.Get(), ArenaGoalLightPos.Get() };
			for (int32 i = 0; i < 3; ++i)
			{
				LampMids[i] = nullptr;
				LampRest[i] = -1.f;
				UStaticMeshComponent* C = LampOn[i];
				if (!C || !C->GetStaticMesh() || !LampMat) continue;
				const int32 Slot = C->GetMaterialIndex(FName(TEXT("lamp")));
				if (Slot == INDEX_NONE) continue;
				float Rest = 0.f;
				if (LampMat->IsA<UMaterialInstanceDynamic>() || !LampMat->GetScalarParameterValue(FHashedMaterialParameterInfo(TEXT("Emissive")), Rest)) { C->SetMaterial(Slot, LampMat); continue; }
				UMaterialInstanceDynamic* D = UMaterialInstanceDynamic::Create(LampMat, this);
				if (bLampTrim) { Rest = LampTrimRest; D->SetScalarParameterValue(TEXT("Emissive"), Rest); }   // lane A172: B's instance ships lit (3): a goal lamp rests off
				C->SetMaterial(Slot, D);
				LampMids[i] = D;
				LampRest[i] = Rest;
			}
			// The lamps' lights (lane A136: PlaceLampLights; the end lights hang over the nets unless `jam.rink.react.loud 0`).
			GoalLineXM = GL;
			PlaceLampLights();
			// The rattle's pieces and where each stands along its long side (rink metres): bench -x, bench +x (the +y side), box -x, box +x, table (the -y side).
			RinkHalfWidthM = R.WidthM * 0.5;
			const double Cx[5] = { -BenchX / UU, BenchX / UU, -PenaltyBoxCentreXM, PenaltyBoxCentreXM, 0.0 };
			const double Hl[5] = { R.LengthM * ArenaBenchLengthFactor * 0.5, R.LengthM * ArenaBenchLengthFactor * 0.5, 2.26, 2.26, 0.70 };
			for (int32 i = 0; i < 5; ++i)
			{
				RattleCentreX[i] = Cx[i];
				RattleHalfLen[i] = Hl[i];
				if (const UStaticMeshComponent* C = RattlePiece(i)) RattleRestRot[i] = C->GetRelativeRotation().Quaternion();
			}
			UE_LOG(LogJamSim, Log, TEXT("JAMRINK arena reactions: goal light mesh %s at x +-%.2f m; lamps table %s, -x %s, +x %s (Emissive at rest %.2f, peak +%.1f for %d frames); jam.rink.react %d"),
				LightMesh ? TEXT("loaded") : TEXT("missing"), EndX / UU, LampMids[0] ? TEXT("yes") : TEXT("no"), LampMids[1] ? TEXT("yes") : TEXT("no"), LampMids[2] ? TEXT("yes") : TEXT("no"),
				LampRest[0], LampPeakAdd, LampFlashFrames, AreReactionsEnabled() ? 1 : 0);
			UE_LOG(LogJamSim, Log, TEXT("JAMRINK arena lamps: slot lamp on %s (rest %.2f, peak %.2f); jam.rink.lamp.trim %d"), bLampTrim ? TEXT("MI_Trim_Lamp") : TEXT("MI_Toon_NetFrame"),
				LampRest[0], LampRest[0] + LampPeakAdd, CVarRinkLampTrim.GetValueOnGameThread());
		}
		// The goal: the variant nearest the cartridge's opening (by the ratio of widths and heights), scaled to it. The mesh's +x runs along the goal line and
		// its +y into the net, so the +x end takes yaw -90 and the -x end yaw +90. Frame on B's MI_Toon_NetFrame, cords on B's MI_Toon_NetMesh.
		const FArenaGoalVariant* Variant = nullptr;
		double BestDist = TNumericLimits<double>::Max();
		if (R.GoalWidthM > 0.0 && NetH > 0.0)
		{
			for (const FArenaGoalVariant& V : GArenaGoalVariants)
			{
				const double Dist = FMath::Abs(FMath::Loge(R.GoalWidthM / V.WidthM)) + FMath::Abs(FMath::Loge(NetH / V.HeightM));
				if (Dist < BestDist) { BestDist = Dist; Variant = &V; }
			}
		}
		UStaticMesh* GoalMesh = Variant ? LoadObject<UStaticMesh>(nullptr, Variant->Path, nullptr, LOAD_NoWarn | LOAD_Quiet) : nullptr;
		UStaticMesh* GoalNetMesh = GoalMesh ? LoadObject<UStaticMesh>(nullptr, Variant->NetPath, nullptr, LOAD_NoWarn | LOAD_Quiet) : nullptr;
		if (!GoalNetMesh) GoalMesh = nullptr;   // a frame without its net is not a goal: the code frame stays
		const FVector GoalScale = Variant ? FVector(R.GoalWidthM / Variant->WidthM, 1.0, NetH / Variant->HeightM) : FVector::OneVector;
		NetRestScale = GoalScale;   // lane A127: the net's shake is a wobble of this scale
		UMaterialInterface* CordMat = LoadLook(TEXT("MI_Toon_NetMesh"), NetMaterialPath);
		CordLookName = TEXT("MI_Toon_NetMesh");
		if (CVarRinkNetCord.GetValueOnGameThread() != 0)   // lane A172: device B's plain off-white cords
		{
			if (UMaterialInterface* Cord = LoadObject<UMaterialInterface>(nullptr, NetCordPath, nullptr, LOAD_NoWarn | LOAD_Quiet)) { CordMat = Cord; CordLookName = TEXT("MI_Toon_NetCord"); }
		}
		for (int32 S = -1; S <= 1; S += 2)
		{
			UStaticMeshComponent* const Pair[2] = { S > 0 ? ArenaGoalPos.Get() : ArenaGoalNeg.Get(), S > 0 ? ArenaGoalNetPos.Get() : ArenaGoalNetNeg.Get() };
			for (int32 k = 0; k < 2; ++k)
			{
				UStaticMeshComponent* C = Pair[k];
				if (!C) continue;
				C->SetStaticMesh(k == 0 ? GoalMesh : GoalNetMesh);
				if (!GoalMesh) continue;
				C->SetRelativeLocationAndRotation(FVector(S * GL * UU, 0.0, 0.0), FRotator(0.0, S > 0 ? -90.0 : 90.0, 0.0));
				C->SetRelativeScale3D(GoalScale);
				BindSlot(C, TEXT("frame"), FrameMat);
				BindSlot(C, TEXT("net"), CordMat);
			}
		}
		UE_LOG(LogJamSim, Log, TEXT("JAMRINK arena goals: mesh %s (%s) for an opening %.2f x %.2f m, scale %.3f along the line, %.3f up, at x +-%.2f m; scorer's table %s at x 0 y -%.2f m"),
			GoalMesh ? TEXT("loaded") : TEXT("missing"), Variant ? Variant->Name : TEXT("none"), R.GoalWidthM, NetH, GoalScale.X, GoalScale.Z, GL,
			TableMesh ? TEXT("loaded") : TEXT("missing"), Y0 / UU);
		ArenaTier = Game.Look.Tier;
		UE_LOG(LogJamSim, Log, TEXT("JAMRINK arena pieces: bench mesh %s, box mesh %s; benches at x +-%.2f m y +%.2f m scale %.3f (%.1f m long), boxes at x +-%.1f m y -%.2f m yaw 180"),
			BenchMesh ? TEXT("loaded") : TEXT("missing"), BoxMesh ? TEXT("loaded") : TEXT("missing"), BenchX / UU, Y0 / UU, BenchScale, R.LengthM * ArenaBenchLengthFactor, PenaltyBoxCentreXM, Y0 / UU);
	}

	// Obstacles (M3b): boxes and pillars, 1.2 m tall like the boards (section 0); C-0005 / A15 adds goal shields (section 0,
	// a wall Size wide across the rink, 0.3 m deep) and bumpers (a pillar in its own magenta section 1).
	{
		FMeshBuf Bump, Shield;
		FMeshBuf BumpTrim;   // lane A64: the bumper's rubber base band, top band and lid (section 3, B's MI_Toon_Dark)
		auto Cylinder = [](FMeshBuf& Buf, const FVector& C, double Hs, double Z0 = 0.0, double Z1 = BoardHeightM * UU)
		{
			const int32 Seg = 20;
			for (int32 i = 0; i < Seg; ++i)
			{
				const double A0 = 2 * UE_DOUBLE_PI * i / Seg, A1 = 2 * UE_DOUBLE_PI * (i + 1) / Seg;
				const FVector D0(FMath::Cos(A0), FMath::Sin(A0), 0), D1(FMath::Cos(A1), FMath::Sin(A1), 0);
				// Lane A15: listed clockwise seen from outside so the front face points outward (the PIE proof showed pillars and bumpers inside out).
				Buf.Quad(C + D1 * Hs + FVector(0, 0, Z0), C + D0 * Hs + FVector(0, 0, Z0), C + D0 * Hs + FVector(0, 0, Z1), C + D1 * Hs + FVector(0, 0, Z1), (D0 + D1).GetSafeNormal());
			}
		};
		for (const FJamObstacle& O : R.Obstacles)
		{
			const double Hs = O.Size * 0.5 * UU;
			const FVector C(O.X * UU, O.Z * UU, 0);
			if (O.Kind == FName(TEXT("pillar")))
			{
				Cylinder(B, C, Hs);
			}
			else if (O.Kind == FName(TEXT("bumper")))
			{
				// Lane A64 (eyes grid h19 finding 1, severity 5): an open magenta tube with no lid read as a flat magenta box. It is now a rubber bollard:
				// a dark base band (0..14 cm) and top band (106..120 cm) 3 cm proud, a dark lid with a magenta boss, the magenta body between.
				const double H = BoardHeightM * UU, Band = 14.0, Proud = 3.0;
				Cylinder(Bump, C, Hs, Band, H - Band);
				Cylinder(BumpTrim, C, Hs + Proud, 0.0, Band);
				Cylinder(BumpTrim, C, Hs + Proud, H - Band, H);
				BumpTrim.Ring(FVector2D(C.X, C.Y), Hs * 0.62, Hs + Proud, H, 20);
				Bump.Ring(FVector2D(C.X, C.Y), 0.0, Hs * 0.62, H + 2.0, 20);
				Cylinder(Bump, C, Hs * 0.62, H, H + 2.0);
			}
			else if (O.Kind == FName(TEXT("goal_shield")))
			{
				// Size metres wide along the rink's z (Unreal Y), 0.3 m deep along x, standing across the rink. Own section 2 (B-to-A-0016: MI_Toon_GoalShield).
				const double Hd = 0.15 * UU;
				Shield.Box(C - FVector(Hd, Hs, 0), C + FVector(Hd, Hs, BoardHeightM * UU));
			}
			else
			{
				B.Box(C - FVector(Hs, Hs, 0), C + FVector(Hs, Hs, BoardHeightM * UU));
			}
		}
		Obstacles->ClearAllMeshSections();
		if (B.V.Num()) { B.Commit(Obstacles, 0); Obstacles->SetMaterial(0, LoadLook(TEXT("MI_Toon_Board"), BoardsMaterialPath)); }
		if (Bump.V.Num())
		{
			Bump.Commit(Obstacles, 1);
			bool bLookBumper = false;
			UMaterialInterface* BumperMat = LoadLook(TEXT("MI_Toon_Bumper"), TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"), &bLookBumper);
			if (!bLookBumper)
			{
				UMaterialInstanceDynamic* D = UMaterialInstanceDynamic::Create(BumperMat, this);
				D->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.9f, 0.2f, 0.6f));
				BumperMat = D;
			}
			else
			{
				// Lane A64: B's MI_Toon_Bumper (M_Toon, TeamColour hot magenta) carries an emissive fill that flattens the body into one tone; the toon
				// ramp needs the light to show the round. The fill is capped at BumperEmissiveCap here until B's instance carries a lower value.
				float BEmissive = 0.f;
				BumperMat->GetScalarParameterValue(FHashedMaterialParameterInfo(TEXT("Emissive")), BEmissive);
				if (BEmissive > BumperEmissiveCap)
				{
					UMaterialInstanceDynamic* D = UMaterialInstanceDynamic::Create(BumperMat, this);
					D->SetScalarParameterValue(TEXT("Emissive"), BumperEmissiveCap);
					BumperMat = D;
				}
				UE_LOG(LogJamSim, Log, TEXT("JAMRINK bumper MI_Toon_Bumper Emissive %.2f -> %.2f (lane A64 cap), lid and bands MI_Toon_Dark"), BEmissive, FMath::Min(BEmissive, BumperEmissiveCap));
			}
			Obstacles->SetMaterial(1, BumperMat);
			if (BumpTrim.V.Num())
			{
				BumpTrim.Commit(Obstacles, 3);
				Obstacles->SetMaterial(3, LoadConstructionMat(TEXT("MI_Toon_Dark"), TEXT("MI_LL_GlassRail"), FLinearColor(0.06f, 0.06f, 0.07f)));
			}
		}
		if (Shield.V.Num())
		{
			// B-to-A-0016: goal shields in pale grey-white (MI_Toon_GoalShield), stencil 1 like the boards; a tinted basic material stands in.
			Shield.Commit(Obstacles, 2);
			bool bLookShield = false;
			UMaterialInterface* ShieldMat = LoadLook(TEXT("MI_Toon_GoalShield"), TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"), &bLookShield);
			if (!bLookShield)
			{
				UMaterialInstanceDynamic* D = UMaterialInstanceDynamic::Create(ShieldMat, this);
				D->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.85f, 0.87f, 0.9f));
				ShieldMat = D;
			}
			Obstacles->SetMaterial(2, ShieldMat);
		}
	}

	// Goal triggers behind each line (cosmetic markers of the goal volume the simulation uses).
	GoalTriggerPos->SetRelativeLocation(FVector((GL + Depth * 0.5) * UU, 0, NetH * 0.5 * UU));
	GoalTriggerPos->SetBoxExtent(FVector(Depth * 0.5 * UU, Hw * UU, NetH * 0.5 * UU));
	GoalTriggerNeg->SetRelativeLocation(FVector(-(GL + Depth * 0.5) * UU, 0, NetH * 0.5 * UU));
	GoalTriggerNeg->SetBoxExtent(FVector(Depth * 0.5 * UU, Hw * UU, NetH * 0.5 * UU));

	SetZoneRingsForTier(Game.Look.Tier);
	SetConstructionVisible(IsConstructionEnabled());
	UJamNeonArena::ApplyFor(this, Game);   // lane A197: the grid arena on a neon tier (NeonArena.h), empty on every other tier
	UE_LOG(LogJamSim, Log, TEXT("JAMRINK built %gx%g m corner %g goal %g m, %d obstacles, %d zones (%d on B's strips), outline %d points, construction %s (%d stanchions)"), R.LengthM, R.WidthM, R.CornerRadiusM, R.GoalWidthM, R.Obstacles.Num(), ZonesDrawn, ZoneQuadsDrawn, Line.Num(), IsConstructionEnabled() ? TEXT("on") : TEXT("off"), GlassRail->GetNumSections() > 1 ? GlassRail->GetProcMeshSection(1)->ProcVertexBuffer.Num() / 24 : 0);
}

bool ARinkActor::IsConstructionEnabled()
{
	return CVarRinkConstruction.GetValueOnGameThread() != 0;
}

void ARinkActor::SetConstructionVisible(bool bVisible)
{
	for (UProceduralMeshComponent* P : { KickPlate.Get(), GlassRail.Get(), GoalLights.Get(), Benches.Get() }) if (P) P->SetVisibility(bVisible);
	if (bClayTier && GlassRail) GlassRail->SetVisibility(false);   // lane A69: B's clay boards are open (the rail and posts are clipped by M_Toon_Rail anyway)
	if (Nets && Nets->GetNumSections() > 2) Nets->SetMeshSectionVisible(2, bVisible);
	if (Markings && Markings->GetNumSections() > 3) { Markings->SetMeshSectionVisible(2, bVisible); Markings->SetMeshSectionVisible(3, bVisible); }
	ApplyArenaDressing();   // lane A103: the Blender pieces replace A25's code benches where they show
}

bool ARinkActor::IsArenaDressingEnabled()
{
	return CVarRinkArena.GetValueOnGameThread() != 0;
}

bool ARinkActor::IsBareTier(const FString& Tier)
{
	// C-0002 (contracts/look-tiers.md): LR_L1_Day and LR_L3_Neon are the contract's empty placeholder rigs: no dressing and no light outside the boards.
	return Tier == TEXT("l1_day") || Tier == TEXT("l3_neon");
}

void ARinkActor::ApplyArenaDressing()
{
	UStaticMeshComponent* const Pieces[4] = { ArenaBenchNeg.Get(), ArenaBenchPos.Get(), ArenaBoxNeg.Get(), ArenaBoxPos.Get() };
	bool bHave = true;
	for (const UStaticMeshComponent* C : Pieces) bHave = bHave && C && C->GetStaticMesh();
	// The construction layer's state is what SetConstructionVisible last applied, not the cvar: a direct call with false must hide the pieces too. The kick
	// plate is the witness (only SetConstructionVisible ever sets its visibility).
	const bool bConstruction = KickPlate ? KickPlate->IsVisible() : IsConstructionEnabled();
	const bool bShow = bHave && bConstruction && IsArenaDressingEnabled() && !IsBareTier(ArenaTier);
	for (UStaticMeshComponent* C : Pieces) if (C) C->SetVisibility(bShow);
	// Lane A160: in a cut that whistles the two penalty boxes stand on a bare tier too (Glass House plays on l3_neon: the one mode with a penalty had no
	// box, and its boxed figure stood on the ice at the boards). Without the whistle a bare tier shows no box, as before.
	const bool bHaveBoxes = ArenaBoxNeg && ArenaBoxPos && ArenaBoxNeg->GetStaticMesh() && ArenaBoxPos->GetStaticMesh();
	const bool bShowBoxes = bShow || (bHaveBoxes && bConstruction && IsArenaDressingEnabled() && bWhistle);
	if (ArenaBoxNeg) ArenaBoxNeg->SetVisibility(bShowBoxes);
	if (ArenaBoxPos) ArenaBoxPos->SetVisibility(bShowBoxes);
	bBoxesShown = bShowBoxes;
	// Lane A160: the goals and the caps stand on a bare tier too (`jam.rink.arena.bare`); the pieces outside the boards do not.
	const bool bInside = bConstruction && IsArenaDressingEnabled() && (!IsBareTier(ArenaTier) || IsBareDressed());
	// A25's code benches stand where the Blender benches go: one or the other, never both.
	if (Benches) Benches->SetVisibility(bConstruction && !bShow);
	// Lane A119: the scorer's table stands with the boxes; the goals follow the same rule on their own meshes, and the code frame with its sheet netting
	// (`Nets`) shows exactly when they do not: one goal on each line, never both, never none.
	if (ArenaTable) ArenaTable->SetVisibility(bShow && ArenaTable->GetStaticMesh() != nullptr);
	UStaticMeshComponent* const Goals[4] = { ArenaGoalNeg.Get(), ArenaGoalPos.Get(), ArenaGoalNetNeg.Get(), ArenaGoalNetPos.Get() };
	bool bHaveGoals = true;
	for (const UStaticMeshComponent* C : Goals) bHaveGoals = bHaveGoals && C && C->GetStaticMesh();
	const bool bShowGoals = bHaveGoals && bInside;
	for (UStaticMeshComponent* C : Goals) if (C) C->SetVisibility(bShowGoals);
	if (Nets) Nets->SetVisibility(!bShowGoals);
	// Lane A127: the Blender goal lights stand with the boxes and the table; A25's code housings on the glass rail show exactly when they do not.
	UStaticMeshComponent* const Lights[2] = { ArenaGoalLightNeg.Get(), ArenaGoalLightPos.Get() };
	bool bHaveLights = true;
	for (const UStaticMeshComponent* C : Lights) bHaveLights = bHaveLights && C && C->GetStaticMesh();
	const bool bShowLights = bHaveLights && bShow;
	for (UStaticMeshComponent* C : Lights) if (C) C->SetVisibility(bShowLights);
	if (GoalLights) GoalLights->SetVisibility(bConstruction && !bShowLights);
	PlaceLampLights();   // lane A136: `jam.rink.react.loud` may have changed
	// Lane A136: the board caps stand with the pieces; `jam.rink.caps 0` takes them away alone.
	const bool bShowCaps = (bShow || (bHave && bInside)) && AreCapsEnabled() && ArenaCaps && ArenaCapsPos && ArenaCaps->GetStaticMesh() != nullptr && CapRest.Num() > 0;
	if (ArenaCaps) ArenaCaps->SetVisibility(bShowCaps);
	if (ArenaCapsPos) ArenaCapsPos->SetVisibility(bShowCaps);
	// Lane A145: the boxes' doors and clock hands stand with their boxes.
	for (UStaticMeshComponent* C : { ArenaBoxDoorNeg.Get(), ArenaBoxDoorPos.Get(), ArenaBoxHandNeg.Get(), ArenaBoxHandPos.Get() }) if (C) C->SetVisibility(bShowBoxes && C->GetStaticMesh() != nullptr);
	if (!bShowCaps) ResetCaps();
	bCapsShown = bShowCaps;
	// The still arena: with the pieces hidden or `jam.rink.react 0` every lamp is at rest and every piece where Build put it.
	if ((!bShow && !bShowBoxes && !bShowCaps) || !bShowGoals || !AreReactionsEnabled()) ResetReactions();
	ApplyBox(LastBoxFrame);   // lane A145: the door's resting place follows `jam.rink.react` (closed while the arena answers, open as it was built before with 0)
	if (bShowGoals != bArenaGoalShown)
	{
		bArenaGoalShown = bShowGoals;
		UE_LOG(LogJamSim, Log, TEXT("JAMRINK arena goals %s for tier %s (meshes %s): %s"), bShowGoals ? TEXT("shown") : TEXT("hidden"), *ArenaTier,
			bHaveGoals ? TEXT("loaded") : TEXT("missing"), bShowGoals ? TEXT("code net frame hidden") : TEXT("code net frame as before"));
	}
	if (bShow != bArenaShown)
	{
		bArenaShown = bShow;
		UE_LOG(LogJamSim, Log, TEXT("JAMRINK arena dressing %s for tier %s (meshes %s, jam.rink.arena %d, construction %s): %s"), bShow ? TEXT("shown") : TEXT("hidden"), *ArenaTier,
			bHave ? TEXT("loaded") : TEXT("missing"), IsArenaDressingEnabled() ? 1 : 0, bConstruction ? TEXT("on") : TEXT("off"), bShow ? TEXT("code benches hidden") : TEXT("code benches as before"));
	}
}

// ---- Lane A127 (track F3, evolution 3): the arena answers the game ----

namespace
{
	// The rinks that have begun play, for the boxed skater's seat (AJamSkater::BoxSeat is one static hook in JamCore; the rink of the skater's world answers).
	TArray<TWeakObjectPtr<ARinkActor>>& SeatRinks()
	{
		static TArray<TWeakObjectPtr<ARinkActor>> A;
		return A;
	}
}

bool ARinkActor::AreReactionsEnabled()
{
	return CVarRinkReact.GetValueOnGameThread() != 0 && IsArenaDressingEnabled();
}

float ARinkActor::LampAt(int64 FramesSince)
{
	if (FramesSince < 0 || FramesSince >= LampFlashFrames) return 0.f;
	// A sawtooth: full on the pulse's first frame, down to 0.35 at its end; the last LampFadeFrames fade the whole pattern out.
	const float Pulse = 1.f - 0.65f * static_cast<float>(FramesSince % LampPulseFrames) / static_cast<float>(LampPulseFrames);
	const int64 Left = LampFlashFrames - FramesSince;
	const float Envelope = Left >= LampFadeFrames ? 1.f : static_cast<float>(Left) / static_cast<float>(LampFadeFrames);
	return Pulse * Envelope;
}

float ARinkActor::NetShakeAt(int64 FramesSince)
{
	if (FramesSince < 0 || FramesSince >= NetShakeFrames) return 0.f;
	// Lane A160: one frame ahead (NetShakeLeadFrames), so the cords are already driven back on the goal's own frame (0.55 of the deepest) and deepest on
	// the next two: the bulge starts with the lamp and device B's NS_goal instead of a frame after them.
	const float T = static_cast<float>(FramesSince + NetShakeLeadFrames);
	return FMath::Exp(-T / 16.f) * FMath::Sin(2.f * UE_PI * T / 10.f);
}

float ARinkActor::RattleAt(int64 FramesSince, float Strength)
{
	if (FramesSince < 0 || FramesSince >= RattleFrames) return 0.f;
	const float T = static_cast<float>(FramesSince);
	return FMath::Clamp(Strength, 0.f, 1.f) * FMath::Exp(-T / 6.f) * FMath::Cos(2.f * UE_PI * T / 8.f);
}

bool ARinkActor::AreCapsEnabled()
{
	return CVarRinkCaps.GetValueOnGameThread() != 0;
}

float ARinkActor::CapRockAt(int64 FramesSince, float Strength)
{
	if (FramesSince < 0 || FramesSince >= CapRockFrames) return 0.f;
	const float T = static_cast<float>(FramesSince);
	return FMath::Clamp(Strength, 0.f, 1.f) * FMath::Exp(-T / 4.f) * FMath::Cos(2.f * UE_PI * T / 8.f);
}

float ARinkActor::HitFlashAt(int64 FramesSince, float Strength)
{
	if (FramesSince < 0 || FramesSince >= HitFlashFrames) return 0.f;
	const float T = 1.f - static_cast<float>(FramesSince) / static_cast<float>(HitFlashFrames);
	return FMath::Clamp(Strength, 0.f, 1.f) * T * T;
}

float ARinkActor::HitLightPeak()
{
	return FMath::Max(0.f, CVarRinkReactHit.GetValueOnGameThread());
}

float ARinkActor::HitLightPeakHere() const
{
	return HitLightPeak() * (ArenaTier == TEXT("l2_night") || IsBareTier(ArenaTier) ? NightLightShare : 1.f);   // lane A160: the bare tiers take night's share (the caps stand there now)
}

float ARinkActor::GetHitLightIntensity(int32 Index) const
{
	const UPointLightComponent* Light = Index >= 0 && Index < CapRipples ? ArenaHitLights[Index].Get() : nullptr;
	return Light && Light->IsVisible() ? Light->Intensity : 0.f;
}

float ARinkActor::GetWaveLightIntensity(int32 Index) const
{
	const UPointLightComponent* Light = Index >= 0 && Index < 2 ? ArenaWaveLights[Index].Get() : nullptr;
	return Light && Light->IsVisible() ? Light->Intensity : 0.f;
}

float ARinkActor::WaveShare()
{
	return FMath::Clamp(CVarRinkReactWave.GetValueOnGameThread(), 0.f, 4.f);
}

float ARinkActor::BoxLightPeak()
{
	return FMath::Max(0.f, CVarRinkReactBox.GetValueOnGameThread());
}

float ARinkActor::BoxLightPeakHere() const
{
	return BoxLightPeak() * (ArenaTier == TEXT("p3_clay") ? 1.f : NightLightShare);
}

float ARinkActor::GetBoxLightIntensity(int32 Team) const
{
	const UPointLightComponent* Light = ArenaBoxLights[Team == 0 ? 0 : 1].Get();
	return Light && Light->IsVisible() ? Light->Intensity : 0.f;
}

bool ARinkActor::IsBareDressed()
{
	return CVarRinkArenaBare.GetValueOnGameThread() != 0;
}

// Lane A160: what a goal's reactions really did, frame by frame after the bus's goal frame (the frame the sim scored: AHokejGameMode::DeliverEvents
// broadcasts the sim's events in the tick that stepped it, and UJamEffectsBinder starts device B's NS_goal on that same broadcast). Each reaction's first
// frame off rest is read from the components themselves, and one line is logged when all four have shown or the first pulse is over.
void ARinkActor::NoteGoalTiming(int64 Since)
{
	if (bGoalTimingLogged || Since < 0) return;
	const int32 F = static_cast<int32>(Since);
	const int32 EndLamp = GoalEnd > 0 ? 2 : 1;
	if (GoalSeenAt[0] < 0 && ((LampMids[0] && GetLampEmissive() > LampRest[0]) || (LampMids[EndLamp] && GetGoalLightEmissive(GoalEnd) > LampRest[EndLamp])
		|| GetLampLightIntensity(0) > 0.f || GetLampLightIntensity(EndLamp) > 0.f)) GoalSeenAt[0] = F;
	if (GoalSeenAt[1] < 0 && !GetNetShakeScale(GoalEnd).Equals(FVector::OneVector, 1e-6)) GoalSeenAt[1] = F;
	if (GoalSeenAt[2] < 0)
	{
		for (const TPair<int32, FVector2f>& P : CapNow)
		{
			if (P.Value.Y > 0.f && CapAtM.IsValidIndex(P.Key) && CapAtM[P.Key].X * GoalEnd > GoalLineXM) { GoalSeenAt[2] = F; break; }
		}
	}
	if (GoalSeenAt[3] < 0 && (GetWaveLightIntensity(0) > 0.f || GetWaveLightIntensity(1) > 0.f)) GoalSeenAt[3] = F;
	const bool bAll = GoalSeenAt[0] >= 0 && GoalSeenAt[1] >= 0 && GoalSeenAt[2] >= 0 && GoalSeenAt[3] >= 0;
	if (!bAll && Since < LampPulseFrames) return;
	bGoalTimingLogged = true;
	UE_LOG(LogJamSim, Log, TEXT("JAMRINK goal timing end=%s frame=%lld: first frame off rest after the goal's bus frame: lamp +%d net +%d caps +%d wave_light +%d (-1 = not in the first %d frames)"),
		GoalEnd > 0 ? TEXT("+x") : TEXT("-x"), static_cast<long long>(GoalFrame), GoalSeenAt[0], GoalSeenAt[1], GoalSeenAt[2], GoalSeenAt[3], LampPulseFrames);
}

float ARinkActor::DoorAt(int64 FramesSince)
{
	if (FramesSince < 0 || FramesSince >= DoorCycleFrames) return 0.f;
	if (FramesSince < DoorOpenFrames)
	{
		const float T = static_cast<float>(FramesSince + 1) / static_cast<float>(DoorOpenFrames);   // open on frame DoorOpenFrames - 1
		return T * T * (3.f - 2.f * T);
	}
	if (FramesSince < DoorOpenFrames + DoorHoldFrames) return 1.f;
	const float T = 1.f - static_cast<float>(FramesSince - DoorOpenFrames - DoorHoldFrames + 1) / static_cast<float>(DoorCloseFrames);
	return T * T * (3.f - 2.f * T);
}

bool ARinkActor::IsBoxUsable() const
{
	return ArenaBoxDoorNeg && ArenaBoxDoorPos && ArenaBoxHandNeg && ArenaBoxHandPos && ArenaBoxDoorNeg->GetStaticMesh() && ArenaBoxDoorPos->GetStaticMesh()
		&& ArenaBoxHandNeg->GetStaticMesh() && ArenaBoxHandPos->GetStaticMesh();
}

void ARinkActor::NoteBoxed(int32 Team) const
{
	if (Team == 0 || Team == 1) BoxSeenAt[Team] = ReactBus ? ReactBus->GetSimFrame() : -1;
}

FTransform ARinkActor::GetCapTransform(int32 Index) const
{
	FTransform X = FTransform::Identity;
	if (!CapSide.IsValidIndex(Index) || !CapInst.IsValidIndex(Index)) return X;
	const UInstancedStaticMeshComponent* Comp = CapSide[Index] ? ArenaCapsPos.Get() : ArenaCaps.Get();
	if (Comp) Comp->GetInstanceTransform(CapInst[Index], X, false);
	return X;
}

// The boxes in use at SimFrame: a foul swings the fouler's team's door open (in), the clock's hand turns once in the penalty's frames, the door swings
// again at the end (out). The sim serves the first penalty frame on the frame after the foul's and releases the skater on foul + penalty_frames
// (Jam.Rink.HitSeenBoxUsed measures both on a real foul), so the penalty's frame k is bus frame foul + k.
void ARinkActor::ApplyBox(int64 SimFrame)
{
	LastBoxFrame = SimFrame;
	const bool bReact = AreReactionsEnabled();
	bool bLive = false;
	for (int32 t = 0; t < 2; ++t)
	{
		FBoxUse& U = BoxUse[t];
		float Door = bReact ? 0.f : 1.f;   // at rest: closed while the arena answers the game; with `jam.rink.react 0` open, as the box mesh had it before the lane
		float Hand = 0.f;
		float Lit = 0.f;     // lane A160: the box's light, on while the penalty runs
		if (U.FoulFrame >= 0)
		{
			const int64 Since = SimFrame - U.FoulFrame;
			const int64 EndSince = static_cast<int64>(U.Frames);         // the frame the skater is back on the ice
			if (!bReact || Since < 0 || Since >= EndSince + DoorCycleFrames) U = FBoxUse();   // over, or a frame before its start: a new match began
			else
			{
				bLive = true;
				Door = FMath::Max(DoorAt(Since), DoorAt(Since - EndSince));
				Hand = Since < EndSince ? 360.f * static_cast<float>(Since) / static_cast<float>(FMath::Max(1, U.Frames)) : 0.f;
				if (Since < EndSince) Lit = FMath::Min(1.f, static_cast<float>(Since + 1) / BoxLightFadeFrames) * FMath::Min(1.f, static_cast<float>(EndSince - Since) / BoxLightFadeFrames);
				if (Since >= EndSince && !U.bEndLogged)
				{
					U.bEndLogged = true;
					UE_LOG(LogJamSim, Log, TEXT("JAMRINK react box end team=%d frame=%lld: the penalty's %d frames are over, the door opens again, the hand is back at the top"), t,
						static_cast<long long>(SimFrame), U.Frames);
				}
			}
		}
		BoxDoorDegNow[t] = DoorOpenDeg * Door;
		BoxHandDegNow[t] = Hand;
		if (UPointLightComponent* Light = ArenaBoxLights[t])
		{
			const float Candelas = bBoxesShown ? BoxLightPeakHere() * Lit : 0.f;
			Light->SetIntensity(Candelas);
			Light->SetVisibility(Candelas > 0.f);
		}
		// The door turns about its hinge (the component's origin on the socket) from along the boards into the box; the hand about the box's `out` axis:
		// a positive pitch takes the hand's top towards the box's -x, clockwise for somebody on the ice.
		if (UStaticMeshComponent* D = t == 0 ? ArenaBoxDoorNeg.Get() : ArenaBoxDoorPos.Get()) D->SetRelativeRotation(FRotator(0.0, BoxDoorDegNow[t], 0.0));
		if (UStaticMeshComponent* Hd = t == 0 ? ArenaBoxHandNeg.Get() : ArenaBoxHandPos.Get()) Hd->SetRelativeRotation(FRotator(BoxHandDegNow[t], 0.0, 0.0));
	}
	bBoxLive = bLive;
}

int32 ARinkActor::GetCapTriangles() const
{
	const UStaticMesh* Mesh = nullptr;
	if (ArenaCaps) Mesh = ArenaCaps->GetStaticMesh();
	return Mesh ? CapRest.Num() * Mesh->GetNumTriangles(0) : 0;
}

// Writes every cap that a live ripple moves at SimFrame and puts back the ones that were moving and are not any more.
void ARinkActor::ApplyCaps(int64 SimFrame)
{
	static const float Share[CapRippleReach + 1] = { 1.f, 0.55f, 0.25f };
	const int32 N = CapRest.Num();
	TMap<int32, FVector2f> Next;
	bool bLive = false;
	for (FCapRipple& Rp : CapRipple)
	{
		if (Rp.Frame < 0) continue;
		const int64 Since = SimFrame - Rp.Frame;
		if (N <= 0 || Since < 0 || Since >= CapRippleFrames) { Rp.Frame = -1; continue; }   // over, or a frame before its start: a new match began
		bLive = true;
		for (int32 D = -CapRippleReach; D <= CapRippleReach; ++D)
		{
			const int32 Ring = FMath::Abs(D);
			const float Cv = CapRockAt(Since - Ring * CapRippleDelay, Rp.Strength) * Share[Ring];
			if (Cv == 0.f) continue;
			FVector2f& V = Next.FindOrAdd(((Rp.Index + D) % N + N) % N, FVector2f::ZeroVector);
			V.X += CapRockDeg * Cv;                 // the tilt swings both ways
			V.Y += CapHopCm * FMath::Abs(Cv);       // the hop is always up
		}
	}
	// Lane A145: the hit flash, one light per ripple (it stands where ReactToEvent put it), Strength x a square fall over HitFlashFrames.
	for (int32 i = 0; i < CapRipples; ++i)
	{
		UPointLightComponent* Light = ArenaHitLights[i];
		if (!Light) continue;
		const float Candelas = CapRipple[i].Frame >= 0 ? HitLightPeakHere() * HitFlashAt(SimFrame - CapRipple[i].Frame, CapRipple[i].Strength) : 0.f;
		Light->SetIntensity(Candelas);
		Light->SetVisibility(Candelas > 0.f);
	}
	// Lane A145: the goal wave. In every pulse of the lamp a front leaves the goal light and runs WaveReachM along the end boards both ways; the caps behind
	// the scored goal line hop as it passes, and a light rides each front, growing while the lamp's own pulse falls.
	float WaveCandelas = 0.f;
	double WaveAtM = 0.0;
	if (GoalFrame >= 0 && bCapsShown && N > 0 && CapAtM.Num() == N && WaveShare() > 0.f)
	{
		const int64 Since = SimFrame - GoalFrame;
		if (Since >= 0 && Since < LampFlashFrames)
		{
			bLive = true;
			const int64 Left = LampFlashFrames - Since;
			const float Envelope = Left >= LampFadeFrames ? 1.f : static_cast<float>(Left) / static_cast<float>(LampFadeFrames);
			const int32 InPulse = static_cast<int32>(Since % LampPulseFrames);
			for (int32 k = 0; k < N; ++k)
			{
				if (CapAtM[k].X * GoalEnd <= GoalLineXM) continue;
				const double Along = FMath::Abs(CapAtM[k].Y);
				if (Along > WaveReachM) continue;
				// Lane A160: the front leaves from the posts (WaveStartHalfM off the middle), so the nearest caps hop on the goal's own frame on every rink.
				int64 T = InPulse - FMath::RoundToInt32(FMath::Max(0.0, Along - WaveStartHalfM) / WaveReachM * (LampPulseFrames - 1));   // frames since the front passed this cap
				if (T < 0)
				{
					if (Since < LampPulseFrames) continue;   // the first front has not reached it yet
					T += LampPulseFrames;                    // still swinging from the pulse before
				}
				const float Cv = CapRockAt(T, 1.f) * WaveCapShare * Envelope;
				if (Cv == 0.f) continue;
				FVector2f& V = Next.FindOrAdd(k, FVector2f::ZeroVector);
				V.X += CapRockDeg * Cv;
				V.Y += CapHopCm * FMath::Abs(Cv);
			}
			const float Phase = static_cast<float>(InPulse) / static_cast<float>(LampPulseFrames);
			WaveAtM = Phase * WaveReachM;
			WaveCandelas = LampLightPeakAt(GoalEnd > 0 ? 2 : 1) * WaveShare() * Phase * Envelope;
		}
	}
	for (int32 s = 0; s < 2; ++s)
	{
		UPointLightComponent* Light = ArenaWaveLights[s];
		if (!Light) continue;
		if (WaveCandelas > 0.f) Light->SetRelativeLocation(FVector(GoalEnd * (GoalLineXM + 4.0 - (IsGoalLoud() ? WaveLightInM : WaveLightInQuietM)) * UU, (s == 0 ? -1.0 : 1.0) * WaveAtM * UU, WaveLightHeightM * UU));
		Light->SetIntensity(WaveCandelas);
		Light->SetVisibility(WaveCandelas > 0.f);
	}
	if (ArenaCaps && ArenaCapsPos && CapSide.Num() == N && CapInst.Num() == N && ArenaCaps->GetInstanceCount() + ArenaCapsPos->GetInstanceCount() == N)
	{
		auto Write = [this](int32 K, const FTransform& X) { (CapSide[K] ? ArenaCapsPos : ArenaCaps)->UpdateInstanceTransform(CapInst[K], X, false, true, true); };
		for (const TPair<int32, FVector2f>& Was : CapNow) if (!Next.Contains(Was.Key) && CapRest.IsValidIndex(Was.Key)) Write(Was.Key, CapRest[Was.Key]);
		for (const TPair<int32, FVector2f>& P : Next)
		{
			// A roll about the cap's own x axis (along the boards): positive tips its top towards its +y, away from the rink.
			FTransform X = CapRest[P.Key];
			X.SetRotation(X.GetRotation() * FRotator(0.0, 0.0, P.Value.X).Quaternion());
			X.AddToTranslation(FVector(0.0, 0.0, P.Value.Y));
			Write(P.Key, X);
		}
	}
	CapNow = MoveTemp(Next);
	bCapsLive = bLive;
}

void ARinkActor::ResetCaps()
{
	for (FCapRipple& Rp : CapRipple) Rp.Frame = -1;
	ApplyCaps(0);
}

UStaticMeshComponent* ARinkActor::RattlePiece(int32 Index) const
{
	switch (Index)
	{
	case 0: return ArenaBenchNeg.Get();
	case 1: return ArenaBenchPos.Get();
	case 2: return ArenaBoxNeg.Get();
	case 3: return ArenaBoxPos.Get();
	case 4: return ArenaTable.Get();
	default: return nullptr;
	}
}

void ARinkActor::ApplyGoalReaction(int64 Since)
{
	const float L = LampAt(Since);
	const int32 EndLamp = GoalEnd > 0 ? 2 : 1;
	for (const int32 i : { 0, EndLamp })
	{
		if (!LampMids[i]) continue;
		LampMids[i]->SetScalarParameterValue(TEXT("Emissive"), LampRest[i] + L * LampPeakAdd);   // L = 0 writes the resting value exactly
		if (UPointLightComponent* Light = ArenaLampLights[i])
		{
			const float Candelas = L * LampLightPeakAt(i);
			Light->SetIntensity(Candelas);
			Light->SetVisibility(Candelas > 0.f);
		}
	}
	// The cords: the net's own component (the frame stands still), deeper and lower as the puck drives it back, then swinging out. Scale only: the mouth
	// stays on the goal line (the mesh's origin).
	if (UStaticMeshComponent* Net = GoalEnd > 0 ? ArenaGoalNetPos.Get() : ArenaGoalNetNeg.Get())
	{
		const float W = NetShakeAt(Since);
		Net->SetRelativeScale3D(FVector(NetRestScale.X, NetRestScale.Y * (1.0 + NetShakeDepth * W), NetRestScale.Z * (1.0 - NetShakeHeight * W)));
	}
}

void ARinkActor::ApplyRattle(int64 Since)
{
	UStaticMeshComponent* C = RattlePiece(RattleIndex);
	if (!C) { RattleDegNow = 0.f; return; }
	RattleDegNow = RattleMaxDeg * RattleAt(Since, RattleStrength);
	// A roll about the piece's own x axis (along the boards) through its origin, the rink-side foot: the piece rocks away from the boards and back.
	C->SetRelativeRotation(RattleRestRot[RattleIndex] * FRotator(0.0, 0.0, RattleDegNow).Quaternion());
}

void ARinkActor::ResetReactions()
{
	if (GoalFrame >= 0) ApplyGoalReaction(MAX_int32);
	if (RattleFrame >= 0) ApplyRattle(MAX_int32);
	ResetCaps();
	GoalFrame = -1;
	RattleFrame = -1;
	RattleDegNow = 0.f;
	for (FBoxUse& U : BoxUse) U = FBoxUse();   // lane A145
	ApplyBox(LastBoxFrame);
}

void ARinkActor::ReactToEvent(FName Event, const FJamEventPayload& Payload, int64 SimFrame)
{
	if (!AreReactionsEnabled()) return;
	const FVector Local = GetActorTransform().InverseTransformPosition(Payload.Where);
	if (Event == JamEvents::Goal && (bArenaShown || bArenaGoalShown))
	{
		if (GoalFrame >= 0) ApplyGoalReaction(MAX_int32);   // a second goal inside the first one's flash: the first end goes back to rest
		GoalFrame = SimFrame;
		GoalEnd = Local.X > 0.0 ? 1 : -1;   // the side of the net the puck is in (the binder's rule for the burst's colour)
		++GoalReactions;
		for (int32& Seen : GoalSeenAt) Seen = -1;   // lane A160: the measured first frame of each reaction, logged once (NoteGoalTiming)
		bGoalTimingLogged = false;
		ApplyGoalReaction(0);
		ApplyCaps(SimFrame);   // lane A145: the wave's first front leaves the goal light
		NoteGoalTiming(0);
		UE_LOG(LogJamSim, Log, TEXT("JAMRINK react goal end=%s frame=%lld lamp %.2f -> %.2f net shake %d frames"), GoalEnd > 0 ? TEXT("+x") : TEXT("-x"),
			static_cast<long long>(SimFrame), LampRest[0], GetLampEmissive(), NetShakeFrames);
	}
	else if (Event == JamEvents::Board && Payload.Kind == FName(TEXT("skater")) && Payload.Strength >= RattleMinStrength && (bArenaShown || bCapsShown))
	{
		const double X = Local.X / UU, Y = Local.Y / UU;
		bool bAnswered = false;
		// Lane A136: the nearest board cap answers anywhere on the ring (the caps stand every 3.6 m at most, so one is always within 2 m of the hit).
		if (bCapsShown && CapRest.Num() > 0 && CapAtM.Num() == CapRest.Num())
		{
			int32 Cap = 0;
			double BestSq = TNumericLimits<double>::Max();
			for (int32 k = 0; k < CapAtM.Num(); ++k)
			{
				const double Sq = FVector2D::DistSquared(CapAtM[k], FVector2D(X, Y));
				if (Sq < BestSq) { BestSq = Sq; Cap = k; }
			}
			int32 Slot = 0;   // a free ripple (Frame -1) first, else the oldest
			for (int32 i = 1; i < CapRipples; ++i) if (CapRipple[i].Frame < CapRipple[Slot].Frame) Slot = i;
			CapRipple[Slot].Frame = SimFrame;
			CapRipple[Slot].Index = Cap;
			CapRipple[Slot].Strength = Payload.Strength;
			// Lane A145: the flash stands at the hit: the event's place moved HitLightInM into the rink along the board's normal (the payload's Dir; without
			// one, towards the rink's middle), HitLightHeightM up, in the colour `jam.rink.react.hit.tint` names.
			if (UPointLightComponent* Light = ArenaHitLights[Slot])
			{
				FVector2D In(GetActorTransform().InverseTransformVectorNoScale(Payload.Dir));
				if (In.IsNearlyZero()) In = FVector2D(-X, -Y);
				In = In.GetSafeNormal();
				Light->SetRelativeLocation(FVector((X + In.X * HitLightInM) * UU, (Y + In.Y * HitLightInM) * UU, HitLightHeightM * UU));
				const int32 Tint = CVarRinkReactHitTint.GetValueOnGameThread();
				Light->SetLightColor(Tint == 0 ? FLinearColor(1.f, 0.84f, 0.58f) : Tint == 2 ? FLinearColor(1.f, 0.017f, 0.005f) : X < 0.0 ? FLinearColor(0.05f, 0.30f, 1.f) : FLinearColor(1.f, 0.36f, 0.03f));
				// Lane A210: a plain bump lights nothing (the light's colour goes black: its candelas, the cap's rock and the tests' read-outs stay). Without
				// a sim to ask (a test, a preview) it is a hit.
				if (CVarRinkReactHitBump.GetValueOnGameThread() == 0)
				{
					const AHokejGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AHokejGameMode>() : nullptr;
					const UJamSim* Sim = GM ? GM->GetSim() : nullptr;
					if (Sim && Sim->GetSkaters().IsValidIndex(Payload.Who) && !Sim->GetSkaters()[Payload.Who].bContactBoard && !Sim->GetSkaters()[Payload.Who].IsDown()) Light->SetLightColor(FLinearColor::Black);
				}
			}
			LastCapIndex = Cap;
			LastCapDistM = static_cast<float>(FMath::Sqrt(BestSq));
			++CapRockReactions;
			ApplyCaps(SimFrame);
			bAnswered = true;
			UE_LOG(LogJamSim, Log, TEXT("JAMRINK react board cap=%d of %d strength=%.2f frame=%lld tilt %.1f deg hop %.1f cm, %.2f m from the hit, flash %.0f cd"), Cap, CapRest.Num(),
				Payload.Strength, static_cast<long long>(SimFrame), GetCapTiltDeg(Cap), GetCapHopCm(Cap), LastCapDistM, GetHitLightIntensity(Slot));
		}
		// Lane A127: the bench, box or table behind a long side's hit rocks too (an end board or a corner has none).
		if (bArenaShown && FMath::Abs(Y) >= RinkHalfWidthM - 1.5)
		{
			int32 Best = -1;
			double BestD = RattleReachM + 1e-6;
			for (int32 i = 0; i < 5; ++i)
			{
				if ((i < 2) != (Y > 0.0)) continue;   // benches on +y, boxes and table on -y
				const UStaticMeshComponent* C = RattlePiece(i);
				if (!C || !C->GetStaticMesh() || !C->IsVisible()) continue;
				const double D = FMath::Max(0.0, FMath::Abs(X - RattleCentreX[i]) - RattleHalfLen[i]);
				if (D < BestD) { BestD = D; Best = i; }
			}
			if (Best >= 0)
			{
				if (RattleFrame >= 0 && RattleIndex != Best) ApplyRattle(MAX_int32);   // the piece that was rocking goes back to rest
				RattleFrame = SimFrame;
				RattleIndex = Best;
				RattleStrength = Payload.Strength;
				++RattleReactions;
				ApplyRattle(0);
				bAnswered = true;
				UE_LOG(LogJamSim, Log, TEXT("JAMRINK react rattle piece=%s strength=%.2f frame=%lld roll %.2f deg, %.2f m along the boards from it"), *RattlePiece(Best)->GetName(),
					Payload.Strength, static_cast<long long>(SimFrame), RattleDegNow, BestD);
			}
		}
		if (!bAnswered) return;
	}
	else if (Event == JamEvents::Foul && bBoxesShown)
	{
		// Lane A145: the fouler's team's box is used. The bus names the fouler by its index (team 0's skaters first); the penalty is game.rules.penalty_frames.
		const int32 Team = Payload.Who >= 0 && Payload.Who < TeamSplit ? 0 : 1;
		BoxUse[Team] = FBoxUse();
		BoxUse[Team].FoulFrame = SimFrame;
		BoxUse[Team].Frames = PenaltyFramesCfg;
		++BoxReactions;
		ApplyBox(SimFrame);
		UE_LOG(LogJamSim, Log, TEXT("JAMRINK react box team=%d who=%d frame=%lld penalty=%d frames: the door %.1f deg on its way to %.0f (open by frame +%d), the hand %.1f deg; meshes %s"), Team, Payload.Who,
			static_cast<long long>(SimFrame), PenaltyFramesCfg, BoxDoorDegNow[Team], DoorOpenDeg, DoorOpenFrames - 1, BoxHandDegNow[Team], IsBoxUsable() ? TEXT("hung") : TEXT("missing"));
	}
	else return;
	if (HasActorBegunPlay()) SetActorTickEnabled(true);
}

void ARinkActor::AdvanceReactions(int64 SimFrame)
{
	if (GoalFrame >= 0)
	{
		const int64 Since = SimFrame - GoalFrame;
		const int64 End = FMath::Max(LampFlashFrames, NetShakeFrames);
		if (Since < 0 || Since >= End) { ApplyGoalReaction(MAX_int32); GoalFrame = -1; }   // a frame before the start: a new match began
		else ApplyGoalReaction(Since);
	}
	if (RattleFrame >= 0)
	{
		const int64 Since = SimFrame - RattleFrame;
		if (Since < 0 || Since >= RattleFrames) { ApplyRattle(MAX_int32); RattleFrame = -1; }
		else ApplyRattle(Since);
	}
	if (bCapsLive) ApplyCaps(SimFrame);
	if (GoalFrame >= 0 && !bGoalTimingLogged) NoteGoalTiming(SimFrame - GoalFrame);   // lane A160
	if (bBoxLive) ApplyBox(SimFrame);   // lane A145
}

float ARinkActor::GetLampEmissive() const
{
	return LampMids[0] ? LampMids[0]->K2_GetScalarParameterValue(TEXT("Emissive")) : -1.f;
}

float ARinkActor::LampLightPeak()
{
	return FMath::Max(0.f, CVarRinkReactLight.GetValueOnGameThread());
}

float ARinkActor::LampLightPeakAt(int32 Index) const
{
	const bool bNight = ArenaTier == TEXT("l2_night") || IsBareTier(ArenaTier);   // lane A160: the bare tiers take night's share (the goals stand there now)
	if (!IsGoalLoud()) return LampLightPeak() * 0.5f * (Index == 0 ? TableLightShareE3 : 1.f) * (bNight ? NightLightShareE3 : 1.f);   // evolution 3's light
	// The table's light keeps what it had on both tiers (750 cd, 300 on l2_night): its night share is evolution 3's.
	if (Index == 0) return LampLightPeak() * TableLightShare * (bNight ? NightLightShareE3 : 1.f);
	return LampLightPeak() * (bNight ? NightLightShare : 1.f);
}

bool ARinkActor::IsGoalLoud()
{
	return CVarRinkReactLoud.GetValueOnGameThread() != 0;
}

void ARinkActor::PlaceLampLights()
{
	// Lane A160: device B's radius (3.5 m: the lamp, the board top and the caps, not the ice) unless the goal is loud (evolution 4's 16 m and 14 m).
	for (UPointLightComponent* Light : { ArenaLampLights[0].Get(), ArenaLampLights[1].Get(), ArenaLampLights[2].Get() }) if (Light) Light->SetAttenuationRadius(IsGoalLoud() ? LampLightRadiusLoudCm : LampLightRadiusCm);
	for (UPointLightComponent* Light : { ArenaWaveLights[0].Get(), ArenaWaveLights[1].Get() }) if (Light) Light->SetAttenuationRadius(IsGoalLoud() ? WaveLightRadiusLoudCm : LampLightRadiusCm);
	// Lane A172: quiet, the goal's lights shine on lighting channel 1 only (the rink's boards, trim, kick plate, caps, boxes and benches): the lamp's own
	// light bleached the lamp it stands beside (its body read R/G 1.37 on a goal's frame, 2.05 unlit at the same Emissive), and it cannot reach the ice.
	// Loud (`jam.rink.react.loud 1`) they light everything, as in evolution 5.
	for (UPointLightComponent* Light : { ArenaLampLights[0].Get(), ArenaLampLights[1].Get(), ArenaLampLights[2].Get(), ArenaWaveLights[0].Get(), ArenaWaveLights[1].Get() })
	{
		if (Light) Light->SetLightingChannels(IsGoalLoud(), !IsGoalLoud(), false);
	}
	// Evolution 3's places, in each piece's own space: 3 m up, a little to the rink's side of the lamp (the piece's -y; the table's lamp stands at its back).
	const FVector LightLocal[3] = { FVector(0.0, 105.0, 305.0), FVector(0.0, -15.0, 300.0), FVector(0.0, -15.0, 300.0) };
	UStaticMeshComponent* const LampOn[3] = { ArenaTable.Get(), ArenaGoalLightNeg.Get(), ArenaGoalLightPos.Get() };
	const bool bLoud = IsGoalLoud();
	for (int32 i = 0; i < 3; ++i)
	{
		if (!ArenaLampLights[i] || !LampOn[i]) continue;
		// Lane A136: an end's light hangs over its net, EndLightBehindLineM behind the goal line and EndLightHeightM up (1 m inside the end boards on every rink).
		if (bLoud && i > 0) ArenaLampLights[i]->SetRelativeLocation(FVector((i == 1 ? -1.0 : 1.0) * (GoalLineXM + EndLightBehindLineM) * UU, 0.0, EndLightHeightM * UU));
		else ArenaLampLights[i]->SetRelativeLocation(LampOn[i]->GetRelativeTransform().TransformPosition(LightLocal[i]));
	}
}

float ARinkActor::GetLampLightIntensity(int32 Index) const
{
	const UPointLightComponent* Light = Index >= 0 && Index < 3 ? ArenaLampLights[Index].Get() : nullptr;
	return Light && Light->IsVisible() ? Light->Intensity : 0.f;
}

float ARinkActor::GetGoalLightEmissive(int32 End) const
{
	const UMaterialInstanceDynamic* D = LampMids[End > 0 ? 2 : 1];
	return D ? const_cast<UMaterialInstanceDynamic*>(D)->K2_GetScalarParameterValue(TEXT("Emissive")) : -1.f;
}

FVector ARinkActor::GetNetShakeScale(int32 End) const
{
	const UStaticMeshComponent* Net = End > 0 ? ArenaGoalNetPos.Get() : ArenaGoalNetNeg.Get();
	if (!Net) return FVector::OneVector;
	const FVector S = Net->GetRelativeScale3D();
	return FVector(S.X / NetRestScale.X, S.Y / NetRestScale.Y, S.Z / NetRestScale.Z);
}

FString ARinkActor::GetRattledPieceName() const
{
	const UStaticMeshComponent* C = RattlePiece(RattleIndex);
	return C ? C->GetName() : FString();
}

void ARinkActor::BindBus(UJamEventBus* InBus)
{
	if (ReactBus == InBus) return;
	if (ReactBus) ReactBus->OnJamEvent.RemoveDynamic(this, &ARinkActor::OnJamEvent);
	ReactBus = InBus;
	if (ReactBus) ReactBus->OnJamEvent.AddDynamic(this, &ARinkActor::OnJamEvent);
}

void ARinkActor::OnJamEvent(FName Event, const FJamEventPayload& Payload)
{
	if (Event != JamEvents::Goal && Event != JamEvents::Board && Event != JamEvents::Foul) return;
	ReactToEvent(Event, Payload, ReactBus ? ReactBus->GetSimFrame() : -1);
}

void ARinkActor::ServeBoxSeat()
{
	if (AJamSkater::BoxSeat) return;
	AJamSkater::BoxSeat = [](const AJamSkater& Skater, FVector& Location, FRotator& Rotation) -> bool
	{
		if (!AreReactionsEnabled()) return false;
		const UWorld* World = Skater.GetWorld();
		for (const TWeakObjectPtr<ARinkActor>& W : SeatRinks())
		{
			const ARinkActor* Rk = W.Get();
			FVector L;
			FRotator R;
			if (Rk && Rk->GetWorld() == World && Rk->GetPenaltyBoxSeat(Skater.GetTeam(), L, R)) { Rk->NoteBoxed(Skater.GetTeam()); Location = L; Rotation = R; return true; }
		}
		return false;
	};
}

void ARinkActor::BeginPlay()
{
	Super::BeginPlay();
	SeatRinks().AddUnique(this);
	ServeBoxSeat();
	if (UGameInstance* GI = GetGameInstance()) BindBus(GI->GetSubsystem<UJamEventBus>());
}

void ARinkActor::EndPlay(const EEndPlayReason::Type Reason)
{
	BindBus(nullptr);
	SeatRinks().Remove(this);
	Super::EndPlay(Reason);
}

void ARinkActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const int64 Now = ReactBus ? ReactBus->GetSimFrame() : -1;
	AdvanceReactions(Now);
	// Lane A145: the frame a boxed figure is no longer drawn in its box (the BoxSeat hook noted it last on BoxSeenAt), beside what the box shows then.
	for (int32 t = 0; t < 2; ++t)
	{
		if (BoxSeenAt[t] < 0 || Now <= BoxSeenAt[t] + 1) continue;
		const int64 End = BoxUse[t].FoulFrame >= 0 ? BoxUse[t].FoulFrame + BoxUse[t].Frames : -1;
		UE_LOG(LogJamSim, Log, TEXT("JAMRINK box left team=%d frame=%lld last_seen=%lld counted_end=%lld door %.1f deg hand %.1f deg"), t, static_cast<long long>(Now),
			static_cast<long long>(BoxSeenAt[t]), static_cast<long long>(End), BoxDoorDegNow[t], BoxHandDegNow[t]);
		BoxSeenAt[t] = -1;
	}
	if (!IsReacting() && BoxSeenAt[0] < 0 && BoxSeenAt[1] < 0) SetActorTickEnabled(false);
}

bool ARinkActor::GetPenaltyBoxSeat(int32 Team, FVector& OutLocation, FRotator& OutRotation) const
{
	const UStaticMeshComponent* C = Team == 0 ? ArenaBoxNeg.Get() : ArenaBoxPos.Get();
	static const FName SeatSocket(TEXT("Seat"));
	if (!bBoxesShown || !C || !C->GetStaticMesh() || !C->DoesSocketExist(SeatSocket)) return false;   // lane A160: the boxes, not the whole dressing (a bare tier that whistles)
	OutLocation = C->GetSocketLocation(SeatSocket);
	// Facing the rink: the box stands on the -y side, so the rink is along the actor's +y.
	OutRotation = GetActorTransform().TransformVectorNoScale(FVector::YAxisVector).Rotation();
	return true;
}

int32 ARinkActor::GetBeaconMode()
{
	return FMath::Clamp(CVarRinkBeacon.GetValueOnGameThread(), 0, 2);
}

void ARinkActor::ApplyBeaconMode()
{
	const int32 Mode = GetBeaconMode();
	UMaterialInterface* Mat = nullptr;
	if (Mode == 1)
	{
		// R-0002: B's translucent beacon instance; the placeholder trim tinted with the trim colour stands in.
		bool bLook = false;
		Mat = LoadLook(TEXT("MI_Trim_Beacon"), TrimMaterialPath, &bLook);
		if (!bLook)
		{
			UMaterialInstanceDynamic* D = UMaterialInstanceDynamic::Create(Mat, this);
			D->SetVectorParameterValue(TEXT("TeamColour"), BeaconColour);
			D->SetVectorParameterValue(TEXT("Color"), BeaconColour);
			D->SetScalarParameterValue(TEXT("Emissive"), 4.f);
			Mat = D;
		}
	}
	else if (Mode == 2)
	{
		// Opaque emissive column in the trim colour (B's MI_Trim_BeaconOpaque if it ever ships; the lab flat instance tinted and lifted stands in).
		Mat = LoadConstructionMat(TEXT("MI_Trim_BeaconOpaque"), TEXT("MI_LL_NetFrame"), BeaconColour, 6.f, true);
	}
	for (UStaticMeshComponent* Bc : { BeaconPos.Get(), BeaconNeg.Get() })
	{
		if (!Bc) continue;
		Bc->SetVisibility(Mode != 0);
		if (Mat) Bc->SetMaterial(0, Mat);
	}
}

UMaterialInterface* ARinkActor::LoadConstructionMat(const TCHAR* LookName, const TCHAR* LabName, const FLinearColor& Tint, float LabEmissive, bool bLabTint)
{
	// B's shipped instance first (/JamAssets/Look/MI_Toon_*), then lane A23's proposal (/Game/LookLab/MI_LL_*, one log line per name), then the basic shape material tinted.
	bool bLook = false;
	UMaterialInterface* Mi = LoadLook(LookName, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"), &bLook);
	if (bLook) return Mi;
	const FString LabPath = FString::Printf(TEXT("/Game/LookLab/%s.%s"), LabName, LabName);
	if (UMaterialInterface* Lab = LoadObject<UMaterialInterface>(nullptr, *LabPath, nullptr, LOAD_NoWarn | LOAD_Quiet))
	{
		static TSet<FString> Logged;
		if (!Logged.Contains(LookName)) { Logged.Add(LookName); UE_LOG(LogJamSim, Log, TEXT("JAMRINK look /JamAssets/Look/%s missing: /Game/LookLab/%s stands in (logged once)"), LookName, LabName); }
		if (LabEmissive >= 0.f || bLabTint)
		{
			// The lab's flat master (M_LL_Flat: Tint, Emissive, Roughness) has no ambient floor, so a vertical face under the near-vertical pool spots falls into
			// the ramp's darkest band and reads black (first proof, 17:08); a small emissive lift keeps the kick plate ochre. B's toon master carries its own fill.
			// Outside the boards there is no light at all in LR_L2_Night, so the benches and goal-light housings are tinted and lifted copies too.
			UMaterialInstanceDynamic* D = UMaterialInstanceDynamic::Create(Lab, this);
			if (LabEmissive >= 0.f) D->SetScalarParameterValue(TEXT("Emissive"), LabEmissive);
			if (bLabTint) D->SetVectorParameterValue(TEXT("Tint"), Tint);
			return D;
		}
		return Lab;
	}
	UMaterialInstanceDynamic* D = UMaterialInstanceDynamic::Create(Mi, this);
	D->SetVectorParameterValue(TEXT("Color"), Tint);
	return D;
}

void ARinkActor::SetZoneRingsForTier(const FString& Tier)
{
	const bool bClay = Tier == TEXT("p3_clay");
	const bool bChanged = bClay != bClayTier;
	bClayTier = bClay;
	for (UProceduralMeshComponent* Rg : ZoneRings) if (Rg) Rg->SetVisibility(bClay);
	for (UStaticMeshComponent* Q : ZoneQuads) if (Q) Q->SetVisibility(!bClay);
	// B-to-A-0035 point 1 (optional): on clay the pane and the rail clip every pixel, so their draws go; back on the other tiers they return.
	if (Glass) Glass->SetVisibility(!bClay);
	if (GlassRail) GlassRail->SetVisibility(!bClay && IsConstructionEnabled());
	// Lane A103: the arena dressing follows the tier through the same call (Build and ULookTierSwitcher::PushGlass both come here).
	ArenaTier = Tier;
	ApplyArenaDressing();
	UJamNeonArena::OnTierChanged(this, Tier);   // lane A197: a tier switch without a rebuild draws or clears the grid arena
	if (bChanged || bClay) UE_LOG(LogJamSim, Log, TEXT("JAMRINK zone rings %s for tier %s: %d ring components, %d decal quads (glass and rail %s)"), bClay ? TEXT("shown") : TEXT("hidden"), *Tier, ZoneRings.Num(), ZoneQuads.Num(), bClay ? TEXT("hidden") : TEXT("shown"));
}

void ARinkActor::SetGlassForTier(const FString& Tier)
{
	if (!Glass) return;
	const TCHAR* LookName = Tier == TEXT("p4_broadcast") ? TEXT("MI_Bcast_Glass") : TEXT("MI_Glass");
	bool bLook = false;
	UMaterialInterface* Mi = LoadLook(LookName, NetMaterialPath, &bLook);
	const FString NewName = bLook ? FString(LookName) : FString(TEXT("placeholder"));
	if (NewName == GlassLookName && Glass->GetMaterial(0) == Mi) return;
	Glass->SetMaterial(0, Mi);
	GlassLookName = NewName;
	UE_LOG(LogJamSim, Log, TEXT("JAMRINK glass %s for tier %s%s"), *GlassLookName, *Tier, bLook ? TEXT("") : TEXT(" (B's instance missing: the net placeholder stands in)"));
}

void ARinkActor::SetTrimEmissive(float Emissive)
{
	// Only B's trim instances carry the contract's Emissive scale (30 by default); the placeholder's 4 is its own and stays.
	if (!bLookTrim) return;
	if (TrimMid) TrimMid->SetScalarParameterValue(TEXT("Emissive"), Emissive);
	if (TrimOrangeMid) TrimOrangeMid->SetScalarParameterValue(TEXT("Emissive"), Emissive);
}
