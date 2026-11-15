// Lane A197: the grid arena (NeonArena.h). Presentation only.
#include "NeonArena.h"

#include "RinkActor.h"
#include "JamPawns.h"
#include "JamSim.h"
#include "ProceduralMeshComponent.h"
#include "Components/MeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "LookTierSwitcher.h"

static TAutoConsoleVariable<int32> CVarJamNeon(TEXT("jam.neon"), 1,
	TEXT("Lane A197: the grid arena on a neon tier (l3_neon, n1_*): 1 on (default), 0 off (the tier as before: the rink's own lines and colours). Presentation only."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamNeonTrails(TEXT("jam.neon.trails"), 1,
	TEXT("Lane A197: 1 = light trails behind skaters and the puck on the grid arena (default), 0 = none (the trails' cost A/B)."), ECVF_Default);

// Lane A197 (B's LR_L3_Neon rig in, 16:55): the parts of A's arena, each switchable, so the rig's own grid and A's do not fight.
static TAutoConsoleVariable<int32> CVarJamNeonSheet(TEXT("jam.neon.sheet"), 1,
	TEXT("Lane A197: 1 = A darkens the ice and the boards and hides the painted marks on the grid arena (default), 0 = the rig's own sheet."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamNeonGrid(TEXT("jam.neon.grid"), 0,
	TEXT("Lane A197: 0 = the rig's diamond grid alone (default since B's LR_L3_Neon: two grids fought in the stills), 1 = A's square grid that brightens near the puck on top."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamNeonLines(TEXT("jam.neon.lines"), 1,
	TEXT("Lane A197: 1 = A's rink lines, board-foot band and goal frames in light by half (default), 0 = none."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamNeonFigures(TEXT("jam.neon.figures"), 1,
	TEXT("Lane A197 (B's ask with LR_L3_Neon): 1 = the figures' trim (slot 2) and helmet (slot 4) glow in the team colour on the grid arena (default), 0 = as on every tier."), ECVF_Default);

// Lane A223 (track L1; B-to-A-0046 point 2): the penalty boxes on the grid arena. Lane A160 stood them on this tier for the cut that whistles (Glow Ice) and
// they kept their daylight materials: a cream board, a grey bench and an ochre kick plate on the dark sheet, the brightest and biggest thing in the frame.
static TAutoConsoleVariable<int32> CVarLookNeonBoxes(TEXT("jam.look.NeonBoxes"), 1,
	TEXT("Lane A223: 1 (default) = on the grid arena the penalty boxes' board, bench and kick plate take the dark sheet's material (the team's seats and the boxed figure stay as they are); 0 = the cream boxes of before the lane."), ECVF_Default);

namespace
{
	constexpr double UU = 100.0;
	constexpr const TCHAR* LightMaterialPath = TEXT("/Game/Placeholders/M_Trim.M_Trim");   // ARinkActor::TrimMaterialPath: the rink's own trim fallback
	constexpr double TileM = 7.5;               // grid tiles: three grid cells a side, one instance each (the glow near the puck is per tile)
	constexpr double GridHalfW = 1.2;           // cm
	constexpr double GridZ = 0.4, LineZ = 0.9, TrailZ = 1.4, PuckZ = 1.8;   // cm above the ice; the rink's own markings sit at 0.6
	constexpr float LineEmissive = 10.f, OrangeLineEmissive = 4.f, CentreEmissive = 8.f, FrameEmissive = 8.f;
	constexpr float TrailEmissive[3] = { 16.f, 6.f, 45.f };   // blue reads darker than orange at one value, and orange past ~5 blooms to yellow; the puck is the brightest thing
	const FLinearColor White(0.85f, 0.95f, 1.f);
	const FLinearColor DarkIce(0.018f, 0.022f, 0.045f);
	const FLinearColor DarkBoard(0.012f, 0.014f, 0.026f);

	struct FBuf
	{
		TArray<FVector> V; TArray<int32> T; TArray<FVector> N; TArray<FVector2D> UV; TArray<FLinearColor> C; TArray<FProcMeshTangent> Tan;
		// The rink's convention (RinkActor.cpp kFrontAlongCross = false): the front triangle's cross(B - A, C - A) points against Normal.
		void Quad(const FVector& A, const FVector& B, const FVector& Cc, const FVector& D, const FVector& Normal, bool bTwoSided = false)
		{
			for (int32 Side = 0; Side < (bTwoSided ? 2 : 1); ++Side)
			{
				const FVector Nn = Side ? -Normal : Normal;
				const int32 I = V.Num();
				V.Append({ A, B, Cc, D });
				for (int32 k = 0; k < 4; ++k) { N.Add(Nn); C.Add(FLinearColor::White); Tan.Add(FProcMeshTangent()); }
				UV.Append({ FVector2D(0, 0), FVector2D(1, 0), FVector2D(1, 1), FVector2D(0, 1) });
				const FVector Cross = FVector::CrossProduct(B - A, Cc - A);
				if (FVector::DotProduct(Cross, Nn) > 0) T.Append({ I, I + 2, I + 1, I, I + 3, I + 2 });
				else T.Append({ I, I + 1, I + 2, I, I + 2, I + 3 });
			}
		}
		void Strip(const FVector2D& A, const FVector2D& B, double HalfW, double Z)
		{
			const FVector2D Dir = (B - A).GetSafeNormal();
			const FVector2D S(-Dir.Y * HalfW, Dir.X * HalfW);
			Quad(FVector(A - S, Z), FVector(B - S, Z), FVector(B + S, Z), FVector(A + S, Z), FVector::UpVector);
		}
		// A strip cut into dashes (On cm lit, Off cm dark); a solid strip when Off <= 0.
		void Dashed(const FVector2D& A, const FVector2D& B, double HalfW, double Z, double On, double Off)
		{
			const double L = FVector2D::Distance(A, B);
			if (Off <= 0 || L <= On) { Strip(A, B, HalfW, Z); return; }
			const FVector2D Dir = (B - A) / L;
			for (double S = 0; S < L; S += On + Off) Strip(A + Dir * S, A + Dir * FMath::Min(L, S + On), HalfW, Z);
		}
		void Ring(const FVector2D& Cc, double R, double HalfW, double Z, int32 Seg, bool bDashed)
		{
			for (int32 i = 0; i < Seg; ++i)
			{
				if (bDashed && (i % 3) == 2) continue;
				const double A0 = 2.0 * UE_DOUBLE_PI * i / Seg, A1 = 2.0 * UE_DOUBLE_PI * (i + 1) / Seg;
				const FVector2D D0(FMath::Cos(A0), FMath::Sin(A0)), D1(FMath::Cos(A1), FMath::Sin(A1));
				Quad(FVector(Cc + D0 * (R - HalfW), Z), FVector(Cc + D0 * (R + HalfW), Z), FVector(Cc + D1 * (R + HalfW), Z), FVector(Cc + D1 * (R - HalfW), Z), FVector::UpVector);
			}
		}
		// A square bar from A to B (the goal frames), half-size H, all four long faces.
		void Bar(const FVector& A, const FVector& B, double H)
		{
			const FVector Dir = (B - A).GetSafeNormal();
			FVector Side = FVector::CrossProduct(Dir, FVector::UpVector);
			if (!Side.Normalize()) Side = FVector::ForwardVector;
			const FVector Up = FVector::CrossProduct(Side, Dir).GetSafeNormal();
			const FVector S = Side * H, U = Up * H;
			Quad(A + S + U, B + S + U, B - S + U, A - S + U, Up);
			Quad(A - S - U, B - S - U, B + S - U, A + S - U, -Up);
			Quad(A + S - U, B + S - U, B + S + U, A + S + U, Side);
			Quad(A - S + U, B - S + U, B - S - U, A - S - U, -Side);
		}
		void Commit(UProceduralMeshComponent* M, int32 Section)
		{
			M->CreateMeshSection_LinearColor(Section, V, T, N, UV, C, Tan, false);
			V.Reset(); T.Reset(); N.Reset(); UV.Reset(); C.Reset(); Tan.Reset();
		}
	};

	// Half the rink's width (m) at x on its rounded outline (the corner arcs), so grid lines stop at the boards.
	double HalfWidthAt(const FJamRinkConfig& R, double X)
	{
		const double Hl = R.LengthM * 0.5, Hw = R.WidthM * 0.5, Cr = FMath::Min(R.CornerRadiusM, FMath::Min(Hl, Hw));
		const double Dx = FMath::Abs(X) - (Hl - Cr);
		if (Dx <= 0) return Hw;
		if (Dx >= Cr) return 0;
		return Hw - Cr + FMath::Sqrt(Cr * Cr - Dx * Dx);
	}
}

UJamNeonArena::UJamNeonArena()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;   // after the pawns and the puck took this frame's render transforms
}

bool UJamNeonArena::IsNeonTier(const FString& Tier)
{
	return Tier.Equals(TEXT("l3_neon"), ESearchCase::CaseSensitive) || Tier.StartsWith(TEXT("n1_"), ESearchCase::CaseSensitive);
}

float UJamNeonArena::GridEmissiveAt(double DistanceM)
{
	const double T = FMath::Clamp(1.0 - DistanceM / GridGlowRadiusM, 0.0, 1.0);
	return GridRestEmissive + (GridNearEmissive - GridRestEmissive) * static_cast<float>(T * T);
}

float UJamNeonArena::GoalPulseAt(int64 FramesSince)
{
	if (FramesSince < 0 || FramesSince >= GoalPulseFrames) return 0.f;
	const float T = static_cast<float>(FramesSince) / GoalPulseFrames;
	return (1.f - T) * (1.f - T);
}

UJamNeonArena* UJamNeonArena::ApplyFor(ARinkActor* Rink, const FJamGameConfig& Game)
{
	if (!Rink) return nullptr;
	UJamNeonArena* A = Rink->FindComponentByClass<UJamNeonArena>();
	if (!A)
	{
		A = NewObject<UJamNeonArena>(Rink, TEXT("NeonArena"));
		A->SetupAttachment(Rink->GetRootComponent());
		A->RegisterComponent();
	}
	A->Config = Game;
	A->bHasConfig = true;
	A->bIceSaved = false;    // Build made a fresh ice instance and set the boards' material again
	A->SavedBoards = nullptr;
	A->SavedBoxes.Reset();   // lane A223: and bound the boxes' materials again
	if (Game.Teams.Num() == 2) { A->TeamColour[0] = Game.Teams[0].Colour; A->TeamColour[1] = Game.Teams[1].Colour; }
	if (IsNeonTier(Game.Look.Tier) && CVarJamNeon.GetValueOnGameThread() != 0) A->Draw(Rink); else A->Clear(Rink);
	return A;
}

void UJamNeonArena::OnTierChanged(ARinkActor* Rink, const FString& Tier)
{
	UJamNeonArena* A = Rink ? Rink->FindComponentByClass<UJamNeonArena>() : nullptr;
	if (!A || !A->bHasConfig || A->Config.Look.Tier == Tier) return;
	A->Config.Look.Tier = Tier;
	if (IsNeonTier(Tier) && CVarJamNeon.GetValueOnGameThread() != 0) A->Draw(Rink); else A->Clear(Rink);
}

UProceduralMeshComponent* UJamNeonArena::MakeMesh(const TCHAR* Name)
{
	UProceduralMeshComponent* M = NewObject<UProceduralMeshComponent>(GetOwner(), Name);
	M->SetupAttachment(this);
	M->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	M->SetCastShadow(false);
	M->bUseAsyncCooking = false;
	M->RegisterComponent();
	return M;
}

UMaterialInstanceDynamic* UJamNeonArena::MakeLight(const FLinearColor& Colour, float Emissive)
{
	UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, LightMaterialPath);
	if (!Base) return nullptr;
	UMaterialInstanceDynamic* D = UMaterialInstanceDynamic::Create(Base, this);
	D->SetVectorParameterValue(TEXT("TeamColour"), Colour);
	D->SetVectorParameterValue(TEXT("Color"), Colour);
	D->SetScalarParameterValue(TEXT("Emissive"), Emissive);
	return D;
}

void UJamNeonArena::Clear(ARinkActor* Rink)
{
	const bool bWas = bActive;
	bActive = false;
	SetComponentTickEnabled(false);
	for (UProceduralMeshComponent* M : { GridMesh.Get(), LineMesh.Get(), TrailMesh.Get() }) if (M) M->ClearAllMeshSections();
	GridTiles.Reset(); TileMids.Reset(); Trails.Reset(); TrailVerts = 0;
	RestoreFigures();
	BindBus(nullptr);
	if (!Rink || !bWas) return;
	// Give the rink back what Draw changed (a tier switch away from neon without a rebuild).
	if (Rink->Markings) Rink->Markings->SetVisibility(true);
	if (Rink->KickPlate) Rink->KickPlate->SetVisibility(ARinkActor::IsConstructionEnabled());
	if (Rink->IceMid && bIceSaved)
	{
		for (const TPair<FName, FLinearColor>& P : SavedIceVectors) Rink->IceMid->SetVectorParameterValue(P.Key, P.Value);
		for (const TPair<FName, float>& P : SavedIceScalars) Rink->IceMid->SetScalarParameterValue(P.Key, P.Value);
	}
	if (Rink->Boards && SavedBoards) Rink->Boards->SetMaterial(0, SavedBoards);
	RestoreBoxes();
	bIceSaved = false;
	SavedBoards = nullptr;
}

void UJamNeonArena::Draw(ARinkActor* Rink)
{
	if (!Rink) return;
	if (!GridMesh) GridMesh = MakeMesh(TEXT("NeonGrid"));
	if (!LineMesh) LineMesh = MakeMesh(TEXT("NeonLines"));
	if (!TrailMesh) TrailMesh = MakeMesh(TEXT("NeonTrails"));
	for (UProceduralMeshComponent* M : { GridMesh.Get(), LineMesh.Get(), TrailMesh.Get() }) M->ClearAllMeshSections();
	const FJamRinkConfig& R = Config.Rink;

	// The dark sheet: the ice's tint near black, the boards dark, the painted marks and the ochre kick plate hidden (the light lines replace them).
	const bool bSheet = CVarJamNeonSheet.GetValueOnGameThread() != 0;
	if (bSheet && Rink->IceMid)
	{
		// Device B's M_Ice takes its tint from these vectors (the placeholder from Color): each is saved once and set near black; the painted lines go dark too.
		static const TCHAR* IceVectors[] = { TEXT("Color"), TEXT("IceColour"), TEXT("FrostColour"), TEXT("BoardBandColour"), TEXT("BandColour"), TEXT("LineBlue"), TEXT("LineRed"),
			TEXT("ScuffColour"), TEXT("SheenColour"), TEXT("LampSheenColour"), TEXT("AmbientColour") };
		if (!bIceSaved)
		{
			SavedIceVectors.Reset();
			for (const TCHAR* Name : IceVectors)
			{
				FLinearColor Was;
				if (Rink->IceMid->GetVectorParameterValue(FHashedMaterialParameterInfo(Name), Was)) SavedIceVectors.Add(Name, Was);
			}
			// The painted bands at the board foot (B's M_Ice: BoardBandCm, BandCm) and the frost band go to zero width: the light band replaces them.
			SavedIceScalars.Reset();
			for (const TCHAR* Name : { TEXT("BoardBandCm"), TEXT("BandCm"), TEXT("FrostWidthCm") })
			{
				float Was = 0.f;
				if (Rink->IceMid->GetScalarParameterValue(FHashedMaterialParameterInfo(Name), Was)) SavedIceScalars.Add(Name, Was);
			}
			bIceSaved = true;
		}
		for (const TPair<FName, float>& P : SavedIceScalars) Rink->IceMid->SetScalarParameterValue(P.Key, 0.f);
		for (const TPair<FName, FLinearColor>& P : SavedIceVectors)
		{
			const FName& N = P.Key;
			const bool bLine = N == FName(TEXT("LineBlue")) || N == FName(TEXT("LineRed")) || N == FName(TEXT("BoardBandColour")) || N == FName(TEXT("BandColour"));
			Rink->IceMid->SetVectorParameterValue(N, bLine ? DarkBoard : (N == FName(TEXT("AmbientColour")) ? P.Value * 0.15f : DarkIce));
		}
	}
	if (bSheet && Rink->Boards)
	{
		if (!SavedBoards) SavedBoards = Rink->Boards->GetMaterial(0);
		if (!DarkBoards) DarkBoards = MakeLight(DarkBoard, 0.f);
		if (DarkBoards) Rink->Boards->SetMaterial(0, DarkBoards);
	}
	DarkenBoxes(Rink, bSheet);   // lane A223
	if (bSheet && Rink->Markings) Rink->Markings->SetVisibility(false);
	if (bSheet && Rink->KickPlate) Rink->KickPlate->SetVisibility(false);

	GridTiles.Reset(); TileMids.Reset(); LineMids.Reset();
	if (CVarJamNeonGrid.GetValueOnGameThread() != 0) BuildGrid(R);
	if (CVarJamNeonLines.GetValueOnGameThread() != 0) BuildLines(Rink, R);
	Trails.Reset();
	FiguresLit = 0;
	FigureMeshes = 0;
	LastSampleFrame = -1;
	GoalFrame = -1;
	bActive = true;
	SetComponentTickEnabled(true);
	if (UGameInstance* GI = Rink->GetGameInstance()) BindBus(GI->GetSubsystem<UJamEventBus>());
	UE_LOG(LogJamLook, Log, TEXT("JAMNEON grid arena on tier %s: %d grid tiles, rink %.0f x %.0f m"), *Config.Look.Tier, GridTiles.Num(), R.LengthM, R.WidthM);
}

void UJamNeonArena::DarkenBoxes(ARinkActor* Rink, bool bSheet)
{
	RestoreBoxes();
	if (!Rink || !bSheet || CVarLookNeonBoxes.GetValueOnGameThread() == 0) return;
	if (!DarkBoards) DarkBoards = MakeLight(DarkBoard, 0.f);
	if (!DarkBoards) return;
	// The daylight slots of SM_Arena_PenaltyBox (the clock's face is part of `board`); `team` (the seats, the door's panel) and `dark` stay.
	static const TCHAR* const Daylight[3] = { TEXT("board"), TEXT("bench"), TEXT("kick") };
	for (UStaticMeshComponent* C : { Rink->ArenaBoxNeg.Get(), Rink->ArenaBoxPos.Get(), Rink->ArenaBoxDoorNeg.Get(), Rink->ArenaBoxDoorPos.Get(), Rink->ArenaBoxHandNeg.Get(), Rink->ArenaBoxHandPos.Get() })
	{
		if (!C || !C->GetStaticMesh()) continue;
		for (const TCHAR* Slot : Daylight)
		{
			const int32 I = C->GetMaterialIndex(FName(Slot));
			if (I == INDEX_NONE) continue;
			FBoxSave Save;
			Save.Mesh = C; Save.Slot = I; Save.Original = C->GetMaterial(I);
			SavedBoxes.Add(Save);
			C->SetMaterial(I, DarkBoards);
		}
	}
	BoxSlotsDarkened = SavedBoxes.Num();
	UE_LOG(LogJamLook, Log, TEXT("JAMNEON penalty boxes on the dark sheet: %d material slots (board, bench, kick plate)"), BoxSlotsDarkened);
}

void UJamNeonArena::RestoreBoxes()
{
	for (const FBoxSave& S : SavedBoxes)
	{
		if (UStaticMeshComponent* C = S.Mesh.Get()) { if (UMaterialInterface* O = S.Original.Get()) C->SetMaterial(S.Slot, O); }
	}
	SavedBoxes.Reset();
	BoxSlotsDarkened = 0;
}

void UJamNeonArena::BuildGrid(const FJamRinkConfig& R)
{
	GridTiles.Reset();
	TileMids.Reset();
	const double Hl = R.LengthM * 0.5, Hw = R.WidthM * 0.5;
	const int32 NX = FMath::Max(1, FMath::CeilToInt32(R.LengthM / TileM)), NY = FMath::Max(1, FMath::CeilToInt32(R.WidthM / TileM));
	const double TX = R.LengthM / NX, TY = R.WidthM / NY;
	FBuf B;
	int32 Section = 0;
	for (int32 ix = 0; ix < NX; ++ix)
	{
		for (int32 iy = 0; iy < NY; ++iy)
		{
			const double X0 = -Hl + ix * TX, X1 = X0 + TX, Y0 = -Hw + iy * TY, Y1 = Y0 + TY;
			// Lines of constant x (along y) and constant y (along x) on the global pitch, clipped to the tile and to the rounded outline.
			for (double X = FMath::CeilToDouble(X0 / GridPitchM) * GridPitchM; X < X1 - 1e-6; X += GridPitchM)
			{
				const double H = HalfWidthAt(R, X) - 0.15;
				const double A = FMath::Max(Y0, -H), Bm = FMath::Min(Y1, H);
				if (Bm - A > 0.05) B.Strip(FVector2D(X, A) * UU, FVector2D(X, Bm) * UU, GridHalfW, GridZ);
			}
			for (double Y = FMath::CeilToDouble(Y0 / GridPitchM) * GridPitchM; Y < Y1 - 1e-6; Y += GridPitchM)
			{
				// Along x at this y: the x range inside the outline, sampled at the tile's two ends (a tile is never wider than a corner arc's reach).
				const double Xa = FMath::Max(X0, -Hl), Xb = FMath::Min(X1, Hl);
				double A = Xa, Bm = Xb;
				while (A < Bm && HalfWidthAt(R, A) - 0.15 < FMath::Abs(Y)) A += 0.1;
				while (Bm > A && HalfWidthAt(R, Bm) - 0.15 < FMath::Abs(Y)) Bm -= 0.1;
				if (Bm - A > 0.05) B.Strip(FVector2D(A, Y) * UU, FVector2D(Bm, Y) * UU, GridHalfW, GridZ);
			}
			if (B.V.Num() == 0) continue;
			B.Commit(GridMesh, Section);
			// The grid takes the tint of its half (a faint team accent), white-blue across the centre tiles.
			const double Cx = (X0 + X1) * 0.5;
			const FLinearColor Tint = FMath::Abs(Cx) < TX * 0.6 ? White : FLinearColor::LerpUsingHSV(White, TeamColour[Cx < 0 ? 0 : 1], 0.55f);
			UMaterialInstanceDynamic* Mid = MakeLight(Tint, GridRestEmissive);
			GridMesh->SetMaterial(Section, Mid);
			TileMids.Add(Mid);
			FTile T; T.Centre = FVector2D(Cx, (Y0 + Y1) * 0.5); T.Emissive = GridRestEmissive;
			GridTiles.Add(T);
			++Section;
		}
	}
}

void UJamNeonArena::BuildLines(ARinkActor* Rink, const FJamRinkConfig& R)
{
	LineMids.Reset();
	LineMids.Add(MakeLight(TeamColour[0], LineEmissive));
	LineMids.Add(MakeLight(TeamColour[1], OrangeLineEmissive));   // orange past ~5 blooms to yellow (first stills)
	LineMids.Add(MakeLight(White, CentreEmissive));
	LineMids.Add(MakeLight(TeamColour[0], FrameEmissive));
	LineMids.Add(MakeLight(TeamColour[1], FrameEmissive * 0.5f));
	const double Hw = R.WidthM * 0.5 * UU, GL = R.GoalLineX();
	// Colour-blind reading: the orange half's lines are dashed (1 m lit, 0.4 m dark), the blue half's solid.
	const double DashOn = 100.0, DashOff = 40.0;
	FBuf Half[2], Centre;
	for (int32 t = 0; t < 2; ++t)
	{
		const double S = t == 0 ? -1.0 : 1.0;
		const double Off = t == 0 ? 0.0 : DashOff;
		FBuf& H = Half[t];
		const double BlueX = S * R.LengthM / 6.0, GoalX = S * GL;
		const double BlueH = (HalfWidthAt(R, BlueX) - 0.05) * UU, GoalH = (HalfWidthAt(R, GoalX) - 0.05) * UU;
		H.Dashed(FVector2D(BlueX * UU, -BlueH), FVector2D(BlueX * UU, BlueH), 15.0, LineZ, DashOn, Off);
		H.Dashed(FVector2D(GoalX * UU, -GoalH), FVector2D(GoalX * UU, GoalH), 4.0, LineZ, DashOn, Off);
		// The two end-zone faceoff circles and their dots.
		const double DotY = FMath::Min(7.0, R.WidthM * 0.3) * UU, CX = S * (GL - 6.0) * UU;
		for (int32 Zs = -1; Zs <= 1; Zs += 2)
		{
			H.Ring(FVector2D(CX, Zs * DotY), 450.0, 4.0, LineZ, 48, t == 1);
			H.Ring(FVector2D(CX, Zs * DotY), 15.0, 15.0, LineZ, 12, false);
		}
		// The crease: a half ring of 1.8 m on the goal line, open towards the centre (dashed on the orange half).
		const double Cr = 180.0;
		for (int32 i = 0; i < 16; ++i)
		{
			if (t == 1 && (i % 3) == 2) continue;
			const double A0 = -UE_DOUBLE_PI * 0.5 + UE_DOUBLE_PI * i / 16.0, A1 = -UE_DOUBLE_PI * 0.5 + UE_DOUBLE_PI * (i + 1) / 16.0;
			const FVector2D C(GoalX * UU, 0);
			const FVector2D D0(-S * FMath::Cos(A0), FMath::Sin(A0)), D1(-S * FMath::Cos(A1), FMath::Sin(A1));
			H.Strip(C + D0 * Cr, C + D1 * Cr, 3.0, LineZ);
		}
	}
	// The board foot: a 10 cm band of light along the inner face of the boards, by half (dashed on the orange half).
	{
		TArray<FVector2D> Line;
		ARinkActor::Outline(R.LengthM, R.WidthM, R.CornerRadiusM, 12, Line);
		for (int32 i = 0; i < Line.Num(); ++i)
		{
			const FVector2D P0 = Line[i] * UU, P1 = Line[(i + 1) % Line.Num()] * UU;
			const double Len = FVector2D::Distance(P0, P1);
			if (Len < 1.0) continue;
			const FVector2D Dir = (P1 - P0) / Len;
			const FVector2D In(-Dir.Y, Dir.X);   // the outline runs counter-clockwise: inward is to the left
			const FVector2D Q0 = P0 + In * 3.0, Q1 = P1 + In * 3.0;
			const int32 t = (P0.X + P1.X) * 0.5 < 0 ? 0 : 1;
			const double Step = t == 1 ? DashOn + DashOff : Len;
			for (double S = 0; S < Len; S += Step)
			{
				const double E = FMath::Min(Len, S + (t == 1 ? DashOn : Len));
				const FVector2D A = Q0 + Dir * S, Bp = Q0 + Dir * E;
				Half[t].Quad(FVector(A, 0.5), FVector(Bp, 0.5), FVector(Bp, 10.5), FVector(A, 10.5), FVector(In, 0));
			}
		}
	}
	// The centre: the centre line, the circle and the dot in white.
	Centre.Strip(FVector2D(0, -(HalfWidthAt(R, 0) - 0.05) * UU), FVector2D(0, (HalfWidthAt(R, 0) - 0.05) * UU), 8.0, LineZ);
	Centre.Ring(FVector2D::ZeroVector, 450.0, 5.0, LineZ, 64, false);
	Centre.Ring(FVector2D::ZeroVector, 15.0, 15.0, LineZ, 12, false);
	// The goal frames: two posts, the crossbar and the top of the cage, 4 cm bars, in the colour of the half they stand in.
	FBuf Frame[2];
	const double NetH = R.GoalHeightM * UU, Hg = R.GoalWidthM * 0.5 * UU, Dp = UJamSim::NetDepth * UU;
	for (int32 t = 0; t < 2; ++t)
	{
		const double S = t == 0 ? -1.0 : 1.0, X = S * GL * UU, Xb = S * (GL * UU + Dp * 0.8);
		FBuf& F = Frame[t];
		F.Bar(FVector(X, -Hg, 0), FVector(X, -Hg, NetH), 4.0);
		F.Bar(FVector(X, Hg, 0), FVector(X, Hg, NetH), 4.0);
		F.Bar(FVector(X, -Hg, NetH), FVector(X, Hg, NetH), 4.0);
		F.Bar(FVector(X, -Hg, NetH), FVector(Xb, -Hg, NetH * 0.8), 3.0);
		F.Bar(FVector(X, Hg, NetH), FVector(Xb, Hg, NetH * 0.8), 3.0);
		F.Bar(FVector(Xb, -Hg, NetH * 0.8), FVector(Xb, Hg, NetH * 0.8), 3.0);
		F.Bar(FVector(X, -Hg, 2.0), FVector(Xb, -Hg, 2.0), 3.0);
		F.Bar(FVector(X, Hg, 2.0), FVector(Xb, Hg, 2.0), 3.0);
		F.Bar(FVector(Xb, -Hg, 2.0), FVector(Xb, Hg, 2.0), 3.0);
	}
	Half[0].Commit(LineMesh, 0);
	Half[1].Commit(LineMesh, 1);
	Centre.Commit(LineMesh, 2);
	Frame[0].Commit(LineMesh, 3);
	Frame[1].Commit(LineMesh, 4);
	for (int32 i = 0; i < LineMids.Num(); ++i) if (LineMids[i]) LineMesh->SetMaterial(i, LineMids[i]);
}

void UJamNeonArena::BindBus(UJamEventBus* InBus)
{
	if (Bus == InBus) return;
	if (Bus) Bus->OnJamEvent.RemoveDynamic(this, &UJamNeonArena::OnJamEvent);
	Bus = InBus;
	if (Bus) Bus->OnJamEvent.AddDynamic(this, &UJamNeonArena::OnJamEvent);
}

void UJamNeonArena::OnJamEvent(FName Event, const FJamEventPayload& Payload)
{
	if (!bActive || Event != JamEvents::Goal) return;
	GoalFrame = Bus ? Bus->GetSimFrame() : LastSampleFrame;
	const FVector Local = GetOwner() ? GetOwner()->GetActorTransform().InverseTransformPosition(Payload.Where) : Payload.Where;
	GoalEnd = Local.X < 0 ? -1 : 1;
	++GoalPulses;
}

void UJamNeonArena::EndPlay(const EEndPlayReason::Type Reason)
{
	BindBus(nullptr);
	Super::EndPlay(Reason);
}

void UJamNeonArena::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!bActive) return;
	const int64 Frame = Bus ? Bus->GetSimFrame() : LastSampleFrame + 1;
	if (Frame == LastSampleFrame) return;   // the sim did not step (a pause): the light holds still
	LastSampleFrame = Frame;
	SampleAndDrawTrails(Frame);
	UpdateGrid(Frame);
}

void UJamNeonArena::UpdateGrid(int64 SimFrame)
{
	FVector2D Puck(1e9, 1e9);
	for (const FTrail& T : Trails)
	{
		if (T.bPuck && T.Points.Num() > 0) { Puck = FVector2D(T.Points[0].X, T.Points[0].Y) / UU; break; }
	}
	const float Pulse = GoalFrame >= 0 ? GoalPulseAt(SimFrame - GoalFrame) : 0.f;
	for (int32 i = 0; i < GridTiles.Num(); ++i)
	{
		const float E = GridEmissiveAt(FVector2D::Distance(GridTiles[i].Centre, Puck)) + Pulse * GoalPulseGrid;
		if (FMath::Abs(E - GridTiles[i].Emissive) < 0.05f) continue;
		GridTiles[i].Emissive = E;
		if (TileMids.IsValidIndex(i) && TileMids[i]) TileMids[i]->SetScalarParameterValue(TEXT("Emissive"), E);
	}
	// The scored frame flashes with the pulse; the other stays at rest.
	if (LineMids.Num() == 5)
	{
		for (int32 t = 0; t < 2; ++t)
		{
			const bool bScored = GoalFrame >= 0 && ((t == 0) == (GoalEnd < 0));
			const float Rest = t == 0 ? FrameEmissive : FrameEmissive * 0.5f;
			const float E = Rest + (bScored ? Pulse * (GoalFramePeak - Rest) : 0.f);
			if (LineMids[3 + t]) LineMids[3 + t]->SetScalarParameterValue(TEXT("Emissive"), E);
		}
	}
}

void UJamNeonArena::SampleAndDrawTrails(int64 SimFrame)
{
	UWorld* W = GetWorld();
	AActor* Owner = GetOwner();
	if (!W || !Owner || !TrailMesh) return;
	const bool bTrails = CVarJamNeonTrails.GetValueOnGameThread() != 0;
	// The tracked actors: rescanned every second and when one goes away (a match restart spawns new pawns).
	bool bRescan = Trails.Num() == 0 || SimFrame % 60 == 0;
	for (const FTrail& T : Trails) if (!T.Who.IsValid()) { bRescan = true; break; }
	if (bRescan)
	{
		TArray<FTrail> Next;
		for (TActorIterator<AJamSkater> It(W); It; ++It)
		{
			FTrail T; T.Who = *It; T.Team = FMath::Clamp(It->GetTeam(), 0, 1);
			if (const FTrail* Old = Trails.FindByPredicate([&](const FTrail& O) { return O.Who == T.Who; })) T.Points = Old->Points;
			ApplyFigure(*It, T.Team);   // every rescan: a figure's visual can arrive after its pawn; a slot already lit is skipped
			Next.Add(MoveTemp(T));
		}
		for (TActorIterator<AJamPuck> It(W); It; ++It)
		{
			FTrail T; T.Who = *It; T.bPuck = true;
			if (const FTrail* Old = Trails.FindByPredicate([&](const FTrail& O) { return O.Who == T.Who; })) T.Points = Old->Points;
			Next.Add(MoveTemp(T));
		}
		Trails = MoveTemp(Next);
		if (FiguresLit != LoggedFigures) { LoggedFigures = FiguresLit; UE_LOG(LogJamLook, Log, TEXT("JAMNEON figures lit: %d material slots (trim and helmet, team colour)"), FiguresLit); }
	}
	const FTransform ToLocal = Owner->GetActorTransform();
	FBuf Buf[3];
	for (FTrail& T : Trails)
	{
		AActor* A = T.Who.Get();
		if (!A) continue;
		if (A->IsHidden()) { T.Points.Reset(); continue; }
		const FVector P = ToLocal.InverseTransformPosition(A->GetActorLocation());
		const FVector Ice(P.X, P.Y, T.bPuck ? PuckZ : TrailZ);
		// A jump of more than 4 m in a frame is a teleport (a faceoff, the box): the trail starts again there.
		if (T.Points.Num() > 0 && FVector::Dist2D(T.Points[0], Ice) > 400.0) T.Points.Reset();
		T.Points.Insert(Ice, 0);
		const int32 Max = T.bPuck ? PuckTrailFrames : TrailFrames;
		if (T.Points.Num() > Max) T.Points.SetNum(Max);
		if (!bTrails) continue;
		FBuf& B = Buf[T.bPuck ? 2 : T.Team];
		// The ribbon tapers to nothing at its tail (the fade); blue is a double line, orange a single solid one (the colour-blind shape).
		const double W0 = T.bPuck ? 9.0 : 16.0;
		for (int32 i = 0; i + 1 < T.Points.Num(); ++i)
		{
			const FVector2D P0(T.Points[i]), P1(T.Points[i + 1]);
			if (FVector2D::Distance(P0, P1) < 0.5) continue;
			const double F0 = 1.0 - static_cast<double>(i) / Max, F1 = 1.0 - static_cast<double>(i + 1) / Max;
			const FVector2D Dir = (P1 - P0).GetSafeNormal(), Side(-Dir.Y, Dir.X);
			const double Z = T.Points[i].Z;
			auto Ribbon = [&](double Offset, double HalfW)
			{
				const FVector2D A0 = P0 + Side * Offset * F0, A1 = P1 + Side * Offset * F1;
				B.Quad(FVector(A0 - Side * HalfW * F0, Z), FVector(A1 - Side * HalfW * F1, Z), FVector(A1 + Side * HalfW * F1, Z), FVector(A0 + Side * HalfW * F0, Z), FVector::UpVector);
			};
			if (!T.bPuck && T.Team == 0) { Ribbon(W0 * 0.7, W0 * 0.22); Ribbon(-W0 * 0.7, W0 * 0.22); }
			else Ribbon(0.0, W0 * 0.5);
		}
		if (T.bPuck && T.Points.Num() > 0) B.Ring(FVector2D(T.Points[0]), 24.0, 5.0, PuckZ, 20, false);   // the puck's own ring of light on the ice
	}
	if (TrailMids.Num() != 3)
	{
		TrailMids.Reset();
		TrailMids.Add(MakeLight(TeamColour[0], TrailEmissive[0]));
		TrailMids.Add(MakeLight(TeamColour[1], TrailEmissive[1]));
		TrailMids.Add(MakeLight(White, TrailEmissive[2]));
	}
	TrailVerts = 0;
	for (int32 s = 0; s < 3; ++s)
	{
		TrailVerts += Buf[s].V.Num();
		if (Buf[s].V.Num() == 0) { TrailMesh->ClearMeshSection(s); continue; }
		Buf[s].Commit(TrailMesh, s);
		if (TrailMids[s] && TrailMesh->GetMaterial(s) != TrailMids[s]) TrailMesh->SetMaterial(s, TrailMids[s]);
	}
}

void UJamNeonArena::ApplyFigure(AActor* Skater, int32 Team)
{
	// B's ask (16:55): on the neon rig the figure's trim (slot 2) and helmet (slot 4) carry the team colour as light: TeamColour = the jersey's colour,
	// Emissive 6, RimColour = 4 x team colour + 0.05, RimStrength 1.5. Set by name on the visual's own dynamic instances, each behind an exists-check,
	// saved first and given back by Clear (a tier switch, a rebuild).
	if (!Skater || CVarJamNeonFigures.GetValueOnGameThread() == 0) return;
	const FLinearColor C = TeamColour[FMath::Clamp(Team, 0, 1)];
	const FLinearColor Rim = C * 4.f + FLinearColor(0.05f, 0.05f, 0.05f, 0.f);
	// The figure is often an actor of its own attached to the pawn (B's BP_SkaterVisual_TB1, AJamSkater::CreateVisual): its meshes count too.
	TArray<AActor*> Parts;
	Skater->GetAttachedActors(Parts, true, true);
	Parts.Add(Skater);
	TArray<UMeshComponent*> Meshes;
	for (AActor* Part : Parts) { TInlineComponentArray<UMeshComponent*> Own; if (Part) Part->GetComponents(Own, true); Meshes.Append(Own.GetData(), Own.Num()); }
	for (UMeshComponent* M : Meshes)
	{
		if (!M || M->GetNumMaterials() < 5) continue;
		for (const int32 Slot : { 2, 4 })
		{
			UMaterialInterface* Mat = M->GetMaterial(Slot);
			if (!Mat) continue;
			UMaterialInstanceDynamic* D = Cast<UMaterialInstanceDynamic>(Mat);
			FLinearColor Probe;
			const bool bTakes = Mat->GetVectorParameterValue(FHashedMaterialParameterInfo(TEXT("TeamColour")), Probe) || Mat->GetVectorParameterValue(FHashedMaterialParameterInfo(TEXT("RimColour")), Probe);
			if (!bTakes) continue;
			FFigureSave Save;
			if (!D)
			{
				// The visual drew this slot with a plain instance: a dynamic one goes in its place, and Clear puts the plain one back.
				D = M->CreateDynamicMaterialInstance(Slot, Mat);
				if (!D) continue;
				Save.Mesh = M; Save.Slot = Slot; Save.Original = Mat;
			}
			if (SavedFigures.ContainsByPredicate([D](const FFigureSave& F) { return F.Mid.Get() == D; })) continue;
			Save.Mid = D;
			FLinearColor V; float S = 0.f;
			if (D->GetVectorParameterValue(FHashedMaterialParameterInfo(TEXT("TeamColour")), V)) { Save.Vectors.Add(TEXT("TeamColour"), V); D->SetVectorParameterValue(TEXT("TeamColour"), C); }
			if (D->GetVectorParameterValue(FHashedMaterialParameterInfo(TEXT("RimColour")), V)) { Save.Vectors.Add(TEXT("RimColour"), V); D->SetVectorParameterValue(TEXT("RimColour"), Rim); }
			if (D->GetScalarParameterValue(FHashedMaterialParameterInfo(TEXT("Emissive")), S)) { Save.Scalars.Add(TEXT("Emissive"), S); D->SetScalarParameterValue(TEXT("Emissive"), 6.f); }
			if (D->GetScalarParameterValue(FHashedMaterialParameterInfo(TEXT("RimStrength")), S)) { Save.Scalars.Add(TEXT("RimStrength"), S); D->SetScalarParameterValue(TEXT("RimStrength"), 1.5f); }
			SavedFigures.Add(MoveTemp(Save)); ++FiguresLit;
		}
	}
}

void UJamNeonArena::RestoreFigures()
{
	for (const FFigureSave& F : SavedFigures)
	{
		if (UMeshComponent* M = F.Mesh.Get()) { if (UMaterialInterface* O = F.Original.Get()) M->SetMaterial(F.Slot, O); continue; }
		UMaterialInstanceDynamic* D = F.Mid.Get();
		if (!D) continue;
		for (const TPair<FName, FLinearColor>& P : F.Vectors) D->SetVectorParameterValue(P.Key, P.Value);
		for (const TPair<FName, float>& P : F.Scalars) D->SetScalarParameterValue(P.Key, P.Value);
	}
	SavedFigures.Reset();
}
