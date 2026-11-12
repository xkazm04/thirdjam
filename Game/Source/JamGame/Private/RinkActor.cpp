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
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "UObject/UObjectIterator.h"

// Lane A25: the construction layer's switch. 0 hides the kick plate, glass rail, net frame extras, goal lights, benches and the extra markings on every rink
// actor (the "before" of the ProfileGPU pair); 1 shows them. Read at Build and on every change.
static TAutoConsoleVariable<int32> CVarRinkConstruction(TEXT("jam.rink.construction"), 1,
	TEXT("1: the rink's construction layer (kick plate, glass rail, net frame, goal lights, benches, faceoff markings) is visible; 0: hidden. Lane A25."));
// Default 0: at 40 m the eyes tool preferred no column over B's translucent one (0.95: "a massive, distracting in-world light beam"; the 14 cm net frame
// reads 4/5 as the far net on both rink sizes without it) and the translucent one only weakly over the opaque (0.6). docs/experiments/RINK.md has the pair.
static TAutoConsoleVariable<int32> CVarRinkBeacon(TEXT("jam.rink.beacon"), 0,
	TEXT("Far-net beacon column: 0 off, 1 B's MI_Trim_Beacon (translucent), 2 opaque emissive column. Lane A25, eyes finding 3."));
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
	PrimaryActorTick.bCanEverTick = false;
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
	const double Depth = UJamSim::NetDepth, NetH = UJamSim::NetHeight;
	FMeshBuf B;

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
		// Lane A25: B's glass instance when it exists (MI_Toon_Glass, stencil 0, translucent); the net placeholder stands in as before.
		Glass->SetMaterial(0, LoadLook(TEXT("MI_Toon_Glass"), NetMaterialPath));
		// Lane A25: stanchions, 5 cm square posts from the board top to the rail top, one every 2.4 m along the outline (the count rounds so the ring closes evenly).
		{
			double Perimeter = 0;
			for (int32 i = 0; i < Line.Num(); ++i) Perimeter += FVector2D::Distance(Line[i], Line[(i + 1) % Line.Num()]);
			const int32 Count = FMath::Max(4, FMath::RoundToInt32(Perimeter / StanchionSpacingM));
			const double Step = Perimeter / Count;
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
			const double Y0 = -Hw * UU, Y1 = Hw * UU, Hh = NetH * UU, Bh = NetBackHeightM * UU;
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
			const double GH = (BoardHeightM + GlassHeightM) * UU + 4.0;
			for (int32 S = -1; S <= 1; S += 2)
			{
				const double Mx = S * (R.LengthM * 0.5 * UU + BoardThicknessM * 0.5 * UU);
				Housing.Box(FVector(Mx - 20.0, -15.0, GH), FVector(Mx + 20.0, 15.0, GH + 30.0));
				const double Lx = Mx - S * 20.0;
				Lens.Box(FVector(FMath::Min(Lx, Lx - S * 4.0), -10.0, GH + 5.0), FVector(FMath::Max(Lx, Lx - S * 4.0), 10.0, GH + 25.0));
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

	// Obstacles (M3b): boxes and pillars, 1.2 m tall like the boards (section 0); C-0005 / A15 adds goal shields (section 0,
	// a wall Size wide across the rink, 0.3 m deep) and bumpers (a pillar in its own magenta section 1).
	{
		FMeshBuf Bump, Shield;
		auto Cylinder = [](FMeshBuf& Buf, const FVector& C, double Hs)
		{
			const int32 Seg = 20;
			for (int32 i = 0; i < Seg; ++i)
			{
				const double A0 = 2 * UE_DOUBLE_PI * i / Seg, A1 = 2 * UE_DOUBLE_PI * (i + 1) / Seg;
				const FVector D0(FMath::Cos(A0), FMath::Sin(A0), 0), D1(FMath::Cos(A1), FMath::Sin(A1), 0);
				// Lane A15: listed clockwise seen from outside so the front face points outward (the PIE proof showed pillars and bumpers inside out).
				Buf.Quad(C + D1 * Hs, C + D0 * Hs, C + D0 * Hs + FVector(0, 0, BoardHeightM * UU), C + D1 * Hs + FVector(0, 0, BoardHeightM * UU), (D0 + D1).GetSafeNormal());
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
				Cylinder(Bump, C, Hs);
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
			Obstacles->SetMaterial(1, BumperMat);
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

	SetConstructionVisible(IsConstructionEnabled());
	UE_LOG(LogJamSim, Log, TEXT("JAMRINK built %gx%g m corner %g goal %g m, %d obstacles, %d zones (%d on B's strips), outline %d points, construction %s (%d stanchions)"), R.LengthM, R.WidthM, R.CornerRadiusM, R.GoalWidthM, R.Obstacles.Num(), ZonesDrawn, ZoneQuadsDrawn, Line.Num(), IsConstructionEnabled() ? TEXT("on") : TEXT("off"), GlassRail->GetNumSections() > 1 ? GlassRail->GetProcMeshSection(1)->ProcVertexBuffer.Num() / 24 : 0);
}

bool ARinkActor::IsConstructionEnabled()
{
	return CVarRinkConstruction.GetValueOnGameThread() != 0;
}

void ARinkActor::SetConstructionVisible(bool bVisible)
{
	for (UProceduralMeshComponent* P : { KickPlate.Get(), GlassRail.Get(), GoalLights.Get(), Benches.Get() }) if (P) P->SetVisibility(bVisible);
	if (Nets && Nets->GetNumSections() > 2) Nets->SetMeshSectionVisible(2, bVisible);
	if (Markings && Markings->GetNumSections() > 3) { Markings->SetMeshSectionVisible(2, bVisible); Markings->SetMeshSectionVisible(3, bVisible); }
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

void ARinkActor::SetTrimEmissive(float Emissive)
{
	// Only B's trim instances carry the contract's Emissive scale (30 by default); the placeholder's 4 is its own and stays.
	if (!bLookTrim) return;
	if (TrimMid) TrimMid->SetScalarParameterValue(TEXT("Emissive"), Emissive);
	if (TrimOrangeMid) TrimOrangeMid->SetScalarParameterValue(TEXT("Emissive"), Emissive);
}
