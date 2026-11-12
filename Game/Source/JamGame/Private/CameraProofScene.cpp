#include "CameraProofScene.h"
#include "ToySkaterVisual.h"
#include "InputMapping.h"
#include "EffectsBinder.h"
#include "HUDWidget.h"
#include "JamEventBus.h"
#include "JamSkaterVisual.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace
{
	constexpr float SimDt = 1.f / 60.f;
	constexpr float TopSpeed = 8.f;

	UStaticMeshComponent* AddBox(AActor* Owner, UStaticMesh* Shape, UMaterialInterface* Base, const TCHAR* Name, FVector LocM, FVector SizeM, FLinearColor Colour)
	{
		UStaticMeshComponent* M = NewObject<UStaticMeshComponent>(Owner, MakeUniqueObjectName(Owner, UStaticMeshComponent::StaticClass(), Name), RF_Transient);
		M->CreationMethod = EComponentCreationMethod::Instance;
		M->SetStaticMesh(Shape);
		M->SetupAttachment(Owner->GetRootComponent());
		M->SetRelativeLocation(LocM * 100.f);
		M->SetRelativeScale3D(SizeM);
		M->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		M->RegisterComponent();
		UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(Base, M);
		Mid->SetVectorParameterValue(TEXT("Color"), Colour);
		M->SetMaterial(0, Mid);
		return M;
	}
}

AJamProofPawn::AJamProofPawn()
{
	USceneComponent* R = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(R);
	Camera = CreateDefaultSubobject<UJamFollowCamera>(TEXT("FollowCamera"));
	Camera->SetupAttachment(R);
	Input = CreateDefaultSubobject<UJamPlayerInput>(TEXT("PlayerInput"));
	bUseControllerRotationYaw = false;
}

AJamProofScene::AJamProofScene()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}

void AJamProofScene::BeginPlay()
{
	Super::BeginPlay();
	BuildRink();
	BuildSkaters();
	Binder = NewObject<UJamEffectsBinder>(this, TEXT("EffectsBinder"));
	Binder->RegisterComponent();

	// Initial frame: the skaters stand where the script starts.
	StepSim();

	if (bSpawnPawnAndHud && GetWorld())
	{
		FActorSpawnParameters P;
		P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Pawn = GetWorld()->SpawnActor<AJamProofPawn>(AJamProofPawn::StaticClass(), FTransform::Identity, P);
		if (Pawn)
		{
			Pawn->Camera->bInterpolate = false;
			Pawn->Camera->ApplyConfig(CameraConfig);
			if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
			{
				PC->Possess(Pawn);
				Hud = UJamHudWidget::CreateAndAdd(PC);
			}
			// A pawn spawned after the first frame still needs a pose: step once more now that its camera listens.
			PushFrame();
		}
	}
}

void AJamProofScene::BuildRink()
{
	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* Plane = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (!Cube || !Plane || !Sphere || !Base)
	{
		return;
	}
	const float L = RinkLengthM, W = RinkWidthM;
	AddBox(this, Plane, Base, TEXT("ice"), FVector(0, 0, 0), FVector(L, W, 1), FLinearColor(0.10f, 0.14f, 0.19f));
	const FLinearColor Board(0.35f, 0.36f, 0.40f);
	AddBox(this, Cube, Base, TEXT("board_n"), FVector(0, W * 0.5f + 0.1f, 0.6f), FVector(L + 0.4f, 0.2f, 1.2f), Board);
	AddBox(this, Cube, Base, TEXT("board_s"), FVector(0, -W * 0.5f - 0.1f, 0.6f), FVector(L + 0.4f, 0.2f, 1.2f), Board);
	AddBox(this, Cube, Base, TEXT("board_e"), FVector(L * 0.5f + 0.1f, 0, 0.6f), FVector(0.2f, W, 1.2f), Board);
	AddBox(this, Cube, Base, TEXT("board_w"), FVector(-L * 0.5f - 0.1f, 0, 0.6f), FVector(0.2f, W, 1.2f), Board);
	// centre line and the two nets (3 m wide, 1.5 high, 1.5 deep), net frames in bright orange so they read at distance
	AddBox(this, Cube, Base, TEXT("centre_line"), FVector(0, 0, 0.01f), FVector(0.3f, W, 0.02f), FLinearColor(0.7f, 0.1f, 0.1f));
	const FLinearColor NetC(1.f, 0.35f, 0.05f);
	for (int32 Sgn : { -1, 1 })
	{
		const float X = Sgn * (L * 0.5f - 4.f);
		AddBox(this, Cube, Base, TEXT("net_back"), FVector(X + Sgn * 1.4f, 0, 0.75f), FVector(0.1f, 3.f, 1.5f), NetC);
		AddBox(this, Cube, Base, TEXT("net_post_a"), FVector(X + Sgn * 0.7f, 1.5f, 0.75f), FVector(1.5f, 0.1f, 1.5f), NetC);
		AddBox(this, Cube, Base, TEXT("net_post_b"), FVector(X + Sgn * 0.7f, -1.5f, 0.75f), FVector(1.5f, 0.1f, 1.5f), NetC);
	}
	PuckMesh = AddBox(this, Sphere, Base, TEXT("puck"), FVector(0, 0, 0.1f), FVector(0.3f, 0.3f, 0.12f), FLinearColor(0.02f, 0.02f, 0.02f));

	if (GetWorld())
	{
		if (ADirectionalLight* Sun = GetWorld()->SpawnActor<ADirectionalLight>(FVector(0, 0, 2000), FRotator(-55.f, 35.f, 0.f)))
		{
			Sun->GetLightComponent()->SetIntensity(3.f);
		}
	}
}

void AJamProofScene::BuildSkaters()
{
	Sk.SetNum(6);
	const FVector2D Start[6] = { {-10, 0}, {-30, 12}, {-35, -12}, {20, 8}, {30, -10}, {40, 0} };
	for (int32 I = 0; I < 6; ++I)
	{
		Sk[I].Pos = Start[I];
		Sk[I].YawDeg = I < 3 ? 0.f : 180.f;

		USceneComponent* Node = NewObject<USceneComponent>(this, MakeUniqueObjectName(this, USceneComponent::StaticClass(), TEXT("skater_node")), RF_Transient);
		Node->CreationMethod = EComponentCreationMethod::Instance;
		Node->SetupAttachment(GetRootComponent());
		Node->RegisterComponent();
		Nodes.Add(Node);

		UToySkaterVisualComponent* V = NewObject<UToySkaterVisualComponent>(this, MakeUniqueObjectName(this, UToySkaterVisualComponent::StaticClass(), TEXT("toy_visual")), RF_Transient);
		V->CreationMethod = EComponentCreationMethod::Instance;
		V->SetupAttachment(Node);
		V->RegisterComponent();
		const FJamPresentationFrame Defaults;
		IJamSkaterVisual::Execute_SetTeam(V, I < 3 ? 0 : 1, Defaults.GetTeamColour(I < 3 ? 0 : 1));
		IJamSkaterVisual::Execute_SetNumber(V, I < 3 ? I + 1 : I + 7);
		Visuals.Add(V);
	}
	IJamSkaterVisual::Execute_SetHasPuck(Visuals[0], true);
}

void AJamProofScene::BroadcastEventForProof(FName Event, int32 Who)
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (UJamEventBus* Bus = GI ? GI->GetSubsystem<UJamEventBus>() : nullptr)
	{
		FJamEventPayload P;
		P.Who = Who;
		P.Where = Sk.IsValidIndex(Who) ? FVector(Sk[Who].Pos.X * 100.f, Sk[Who].Pos.Y * 100.f, 0.f) : FVector::ZeroVector;
		P.Strength = 0.6f;
		Bus->Broadcast(Event, P);
	}
}

void AJamProofScene::StepSim()
{
	++SimFrame;
	const int32 F = (int32)SimFrame;
	const float L = RinkLengthM, W = RinkWidthM;

	// ---- the human (index 0)
	FSk& H = Sk[0];
	float YawRate = 0.f;  // rad/s, Unreal yaw sign (positive = clockwise from above = a right turn)
	float TargetSpeed = TopSpeed;
	bool bShoot = false, bCheck = false;
	if (Pawn && Pawn->Input && Pawn->Input->GetAxis().Size() > 0.1f)
	{
		bManual = true;
	}
	if (bManual && Pawn && Pawn->Input)
	{
		const FVector2D A = Pawn->Input->GetAxis();
		TargetSpeed = FMath::Max(0.f, A.Y) * TopSpeed;
		YawRate = A.X * 3.f;
		bShoot = Pawn->Input->ConsumeShootPressed();
		bCheck = Pawn->Input->ConsumeCheckPressed();
	}
	else if (bScriptedHuman)
	{
		YawRate = 2.2f * FMath::Sin(F / 70.f);
		bShoot = F % 90 == 45;
		bCheck = F % 150 == 100;
		const float Mx = L * 0.5f - 8.f, My = W * 0.5f - 6.f;
		if (FMath::Abs(H.Pos.X) > Mx || FMath::Abs(H.Pos.Y) > My)
		{
			const float ToCentre = FMath::RadiansToDegrees(FMath::Atan2(-H.Pos.Y, -H.Pos.X));
			YawRate = FMath::Clamp(FMath::UnwindDegrees(ToCentre - H.YawDeg) * 0.1f, -3.f, 3.f);
		}
	}
	else
	{
		TargetSpeed = 0.f;
	}
	H.Speed += (TargetSpeed - H.Speed) * 0.04f;
	H.YawDeg = FMath::UnwindDegrees(H.YawDeg + FMath::RadiansToDegrees(YawRate) * SimDt);
	H.TurnRate = -YawRate;  // contract: positive = turning left seen from above
	const float Yr = FMath::DegreesToRadians(H.YawDeg);
	H.Pos += FVector2D(FMath::Cos(Yr), FMath::Sin(Yr)) * H.Speed * SimDt;

	// ---- the bots circle on ellipses
	const FVector2D Base[6] = { {0, 0}, {-22, 10}, {-26, -10}, {22, 8}, {28, -10}, {36, 0} };
	for (int32 I = 1; I < 6; ++I)
	{
		FSk& S = Sk[I];
		const float Ph = I * 1.1f, Wv = 0.011f + 0.002f * I;
		const FVector2D Prev = S.Pos;
		const float Rx = 9.f + I, Ry = 6.f + 0.5f * I;
		S.Pos = Base[I] + FVector2D(Rx * FMath::Cos(Wv * F + Ph), Ry * FMath::Sin(Wv * F + Ph));
		const FVector2D D = (S.Pos - Prev);
		if (D.SizeSquared() > 1e-8f)
		{
			const float NewYaw = FMath::RadiansToDegrees(FMath::Atan2(D.Y, D.X));
			S.TurnRate = -FMath::DegreesToRadians(FMath::UnwindDegrees(NewYaw - S.YawDeg)) / SimDt;  // positive = left
			S.YawDeg = NewYaw;
		}
		S.Speed = D.Size() / SimDt;
	}
	// a bot gets knocked down every 240 frames for 60 frames, towards +X
	if (F % 240 == 100)
	{
		Sk[1].bDown = true;
		Sk[1].DownUntil = F + 60;
		IJamSkaterVisual::Execute_SetKnockedDown(Visuals[1], true, 1.f, 0.f);
		BroadcastEventForProof(JamEvents::Knockdown, 1);
	}
	if (Sk[1].bDown && F >= Sk[1].DownUntil)
	{
		Sk[1].bDown = false;
		IJamSkaterVisual::Execute_SetKnockedDown(Visuals[1], false, 0.f, 0.f);
	}

	// ---- shots, checks and the puck
	if (bShoot && PuckFree == 0)
	{
		IJamSkaterVisual::Execute_PlayShoot(Visuals[0]);
		IJamSkaterVisual::Execute_SetHasPuck(Visuals[0], false);
		PuckVel = FVector2D(FMath::Cos(Yr), FMath::Sin(Yr)) * 20.f;
		PuckFree = 50;
		BroadcastEventForProof(JamEvents::Shot, 0);
	}
	if (bCheck)
	{
		IJamSkaterVisual::Execute_PlayCheck(Visuals[0]);
		BroadcastEventForProof(JamEvents::Check, 0);
	}
	if (PuckFree > 0)
	{
		Puck += PuckVel * SimDt;
		PuckVel *= 0.985f;
		Puck.X = FMath::Clamp(Puck.X, -L * 0.5f + 0.5f, L * 0.5f - 0.5f);
		Puck.Y = FMath::Clamp(Puck.Y, -W * 0.5f + 0.5f, W * 0.5f - 0.5f);
		if (--PuckFree == 0)
		{
			IJamSkaterVisual::Execute_SetHasPuck(Visuals[0], true);
		}
	}
	else
	{
		const FVector2D Fwd(FMath::Cos(Yr), FMath::Sin(Yr)), Right(-FMath::Sin(Yr), FMath::Cos(Yr));
		Puck = H.Pos + Fwd * 1.15f + Right * 0.35f;
	}

	// ---- visuals, then the presentation frame (what the real skater pawns and game mode do)
	for (int32 I = 0; I < 6; ++I)
	{
		Nodes[I]->SetWorldLocation(FVector(Sk[I].Pos.X * 100.f, Sk[I].Pos.Y * 100.f, 0.f));
		IJamSkaterVisual::Execute_UpdateMotion(Visuals[I], Sk[I].Speed, Sk[I].TurnRate, FRotator(0.f, Sk[I].YawDeg, 0.f));
	}
	if (PuckMesh)
	{
		PuckMesh->SetRelativeLocation(FVector(Puck.X * 100.f, Puck.Y * 100.f, 8.f));
	}
	PushFrame();
}

void AJamProofScene::PushFrame()
{
	UJamPresentation* P = UJamPresentation::Get(this);
	if (!P || Sk.Num() < 6)
	{
		return;
	}
	FJamPresentationFrame Fr;
	Fr.SimFrame = SimFrame;
	Fr.RinkLengthM = RinkLengthM;
	Fr.RinkWidthM = RinkWidthM;
	for (int32 I = 0; I < 6; ++I)
	{
		FJamPresentationSkater S;
		S.PosM = Sk[I].Pos;
		S.YawDeg = Sk[I].YawDeg;
		S.TeamId = I < 3 ? 0 : 1;
		S.bHuman = I == 0;
		Fr.Skaters.Add(S);
	}
	Fr.PuckM = Puck;
	Fr.HumanIndex = 0;
	Fr.AttackSign = 1;
	Fr.ScoreHome = ScoreHome;
	Fr.ScoreAway = ScoreAway;
	Fr.Take = 1;
	Fr.bHasTimer = true;
	Fr.FramesRemaining = FMath::Max(0, 180 * 60 - (int32)SimFrame);
	Fr.Phase = Phase;
	Fr.bDirectorsCallReady = SimFrame > 300 && Phase == EJamPhase::Play;
	P->PushFrame(Fr);
}

void AJamProofScene::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	Accumulator += DeltaSeconds;
	int32 Steps = 0;
	while (Accumulator >= SimDt - 1e-5f && Steps < 8)
	{
		StepSim();
		Accumulator -= SimDt;
		++Steps;
	}
	if (Steps == 8)
	{
		Accumulator = 0.f;
	}
	if (UJamPresentation* P = UJamPresentation::Get(this))
	{
		P->SetRenderAlpha(FMath::Max(0.f, Accumulator) / SimDt);
	}
}

void AJamProofScene::StepFrames(int32 N)
{
	for (int32 I = 0; I < N; ++I)
	{
		StepSim();
	}
}

void AJamProofScene::SetPhaseForProof(EJamPhase InPhase, int32 Home, int32 Away)
{
	Phase = InPhase;
	ScoreHome = Home;
	ScoreAway = Away;
	PushFrame();
}

void AJamProofScene::SetArenaMode(bool bArena)
{
	bArenaMode = bArena;
	if (Pawn && Pawn->Camera)
	{
		CameraConfig.Mode = bArena ? EJamCameraMode::Arena : EJamCameraMode::ThirdPerson;
		if (bArena)
		{
			CameraConfig.FovDeg = 70.f;
		}
		Pawn->Camera->ApplyConfig(CameraConfig);
		PushFrame();
	}
}

void AJamProofScene::PlaceHumanAt(float XM, float ZM, float YawDeg)
{
	if (Sk.Num() > 0)
	{
		Sk[0].Pos = FVector2D(XM, ZM);
		Sk[0].YawDeg = YawDeg;
		Sk[0].Speed = 0.f;
		bScriptedHuman = false;
		StepSim();
	}
}
