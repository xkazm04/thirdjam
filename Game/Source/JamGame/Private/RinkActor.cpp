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
	Glass->SetCastShadow(false);
	// R-0002 (B-to-A-0005): stencil 1 = toon ramp + outline, 2 = banded ice without outline, 0 = untouched.
	for (UProceduralMeshComponent* P : { Boards.Get(), Nets.Get(), Obstacles.Get() }) { P->SetRenderCustomDepth(true); P->SetCustomDepthStencilValue(1); }
	Ice->SetRenderCustomDepth(true);
	Ice->SetCustomDepthStencilValue(2);
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
		FMeshBuf G, TrB, TrO;
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
			// R-0002: a 10 cm emissive strip on top of the board, along its inner edge (a box: top, inner and outer faces).
			const FVector2D I0 = P0 + Out * 1.0, I1 = P1 + Out * 1.0, O0 = P0 + Out * 11.0, O1 = P1 + Out * 11.0;
			FMeshBuf& Tr = ((P0.X + P1.X) * 0.5 < 0) ? TrB : TrO;
			Tr.Quad(FVector(I0, H + 10.0), FVector(I1, H + 10.0), FVector(O1, H + 10.0), FVector(O0, H + 10.0), FVector::UpVector);
			Tr.Quad(FVector(I0, H), FVector(I1, H), FVector(I1, H + 10.0), FVector(I0, H + 10.0), Nin);
			Tr.Quad(FVector(O0, H), FVector(O0, H + 10.0), FVector(O1, H + 10.0), FVector(O1, H), Nout);
		}
		B.Commit(Boards, 0);
		Boards->SetMaterial(0, LoadLook(TEXT("MI_Toon_Board"), BoardsMaterialPath));
		G.Commit(Glass, 0);
		Glass->SetMaterial(0, LoadMat(NetMaterialPath));
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
		const UMaterialInterface* BeaconMat = TrimMat(TEXT("MI_Trim_Beacon"), Game.Look.TrimColour);
		for (int32 S = -1; S <= 1; S += 2)
		{
			UStaticMeshComponent* Bc = S > 0 ? BeaconPos : BeaconNeg;
			Bc->SetRelativeLocation(FVector(S * (GL + Depth + 1.0) * UU, 0, 400.0));
			Bc->SetRelativeScale3D(FVector(0.12, 0.12, 8.0));
			Bc->SetMaterial(0, const_cast<UMaterialInterface*>(BeaconMat));
		}
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
			for (int32 Zs = -1; Zs <= 1; Zs += 2)
			{
				B.Ring(FVector2D(S * (GL - 6.0) * UU, Zs * FMath::Min(7.0, R.WidthM * 0.3) * UU), 0.0, 30.0, Z, 16);
			}
		}
		B.Ring(FVector2D::ZeroVector, 0.0, 15.0, Z + 0.1, 16);
		B.Commit(Markings, 0);
		FMeshBuf Blue;
		for (int32 S = -1; S <= 1; S += 2) Blue.Strip(FVector2D(S * R.LengthM / 6.0 * UU, -Hz), FVector2D(S * R.LengthM / 6.0 * UU, Hz), 15.0, Z);
		Blue.Ring(FVector2D::ZeroVector, 450.0, 460.0, Z, 64);
		Blue.Commit(Markings, 1);
		UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
		UMaterialInstanceDynamic* Red = UMaterialInstanceDynamic::Create(Base, this);
		Red->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.6f, 0.02f, 0.02f));
		UMaterialInstanceDynamic* BlueM = UMaterialInstanceDynamic::Create(Base, this);
		BlueM->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.02f, 0.08f, 0.6f));
		Markings->SetMaterial(0, Red);
		Markings->SetMaterial(1, BlueM);
		// B's ice material paints the lines itself from RinkHalfLength / RinkHalfWidth; ours would double them.
		Markings->SetVisibility(!bLookIce);
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
		FMeshBuf Mesh;
		const double PR = 5.0; // post half size, cm
		for (int32 S = -1; S <= 1; S += 2)
		{
			const double X0 = S * GL * UU, X1 = S * (GL + Depth) * UU;
			const double Y0 = -Hw * UU, Y1 = Hw * UU, Hh = NetH * UU;
			B.Box(FVector(X0 - PR, Y0 - PR, 0), FVector(X0 + PR, Y0 + PR, Hh));
			B.Box(FVector(X0 - PR, Y1 - PR, 0), FVector(X0 + PR, Y1 + PR, Hh));
			B.Box(FVector(X0 - PR, Y0 - PR, Hh - PR * 2), FVector(X0 + PR, Y1 + PR, Hh));
			B.Box(FVector(FMath::Min(X0, X1) - PR, Y0 - PR, 0), FVector(FMath::Max(X0, X1) + PR, Y0 + PR, 2 * PR));
			B.Box(FVector(FMath::Min(X0, X1) - PR, Y1 - PR, 0), FVector(FMath::Max(X0, X1) + PR, Y1 + PR, 2 * PR));
			B.Box(FVector(X1 - PR, Y0 - PR, 0), FVector(X1 + PR, Y1 + PR, 2 * PR));
			const FVector In(-S, 0, 0);
			Mesh.Quad(FVector(X1, Y0, 0), FVector(X1, Y1, 0), FVector(X1, Y1, Hh), FVector(X1, Y0, Hh), In, true);
			Mesh.Quad(FVector(X0, Y0, 0), FVector(X1, Y0, 0), FVector(X1, Y0, Hh), FVector(X0, Y0, Hh), FVector(0, 1, 0), true);
			Mesh.Quad(FVector(X0, Y1, 0), FVector(X1, Y1, 0), FVector(X1, Y1, Hh), FVector(X0, Y1, Hh), FVector(0, -1, 0), true);
			Mesh.Quad(FVector(X0, Y0, Hh), FVector(X1, Y0, Hh), FVector(X1, Y1, Hh), FVector(X0, Y1, Hh), FVector(0, 0, -1), true);
		}
		B.Commit(Nets, 0);
		Mesh.Commit(Nets, 1);
		bool bLookFrame = false;
		UMaterialInterface* FrameMat = LoadLook(TEXT("MI_Toon_NetFrame"), TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"), &bLookFrame);
		if (!bLookFrame)
		{
			UMaterialInstanceDynamic* Frame = UMaterialInstanceDynamic::Create(FrameMat, this);
			Frame->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.7f, 0.03f, 0.02f));
			FrameMat = Frame;
		}
		Nets->SetMaterial(0, FrameMat);
		Nets->SetMaterial(1, LoadLook(TEXT("MI_Toon_NetMesh"), NetMaterialPath));
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

	UE_LOG(LogJamSim, Log, TEXT("JAMRINK built %gx%g m corner %g goal %g m, %d obstacles, %d zones (%d on B's strips), outline %d points"), R.LengthM, R.WidthM, R.CornerRadiusM, R.GoalWidthM, R.Obstacles.Num(), ZonesDrawn, ZoneQuadsDrawn, Line.Num());
}

void ARinkActor::SetTrimEmissive(float Emissive)
{
	// Only B's trim instances carry the contract's Emissive scale (30 by default); the placeholder's 4 is its own and stays.
	if (!bLookTrim) return;
	if (TrimMid) TrimMid->SetScalarParameterValue(TEXT("Emissive"), Emissive);
	if (TrimOrangeMid) TrimOrangeMid->SetScalarParameterValue(TEXT("Emissive"), Emissive);
}
