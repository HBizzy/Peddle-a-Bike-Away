// CyclistPawn.cpp
// Main controllable character for Peddle a Bike Away

#include "CyclistPawn.h"
#include "PedalComponent.h"
#include "LeanComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "PhysicalAnimationComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "TimerManager.h"

ACyclistPawn::ACyclistPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// Root — bike frame is the physics anchor
	BikeFrameMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BikeFrameMesh"));
	SetRootComponent(BikeFrameMesh);
	BikeFrameMesh->SetSimulatePhysics(true);
	BikeFrameMesh->SetCollisionProfileName(TEXT("PhysicsActor"));

	// Rider mesh — separate physics body, connected to bike via constraint
	RiderMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RiderMesh"));
	RiderMesh->SetupAttachment(BikeFrameMesh);
	RiderMesh->SetSimulatePhysics(true);
	RiderMesh->SetCollisionProfileName(TEXT("Ragdoll"));

	// Physical Animation — blends between animation pose and ragdoll
	PhysicalAnimation = CreateDefaultSubobject<UPhysicalAnimationComponent>(TEXT("PhysicalAnimation"));

	// Constraint connecting rider to bike (allows separation on falls)
	RiderBikeConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("RiderBikeConstraint"));
	RiderBikeConstraint->SetupAttachment(BikeFrameMesh);

	// One PedalComponent per leg
	LeftPedal = CreateDefaultSubobject<UPedalComponent>(TEXT("LeftPedal"));
	RightPedal = CreateDefaultSubobject<UPedalComponent>(TEXT("RightPedal"));

	// Balance controller
	LeanController = CreateDefaultSubobject<ULeanComponent>(TEXT("LeanController"));

	// Side-scroll camera rig — locked to one side, 2.5D perspective
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(BikeFrameMesh);
	SpringArm->TargetArmLength = 800.0f;
	SpringArm->bDoCollisionTest = false;
	// Lock to the side for consistent 2.5D view (Getting Over It perspective)
	SpringArm->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bInheritRoll = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
}

void ACyclistPawn::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = GetActorLocation();

	// Wire up rider mesh references so pedals/lean know what to push
	LeftPedal->RiderMeshRef = RiderMesh;
	RightPedal->RiderMeshRef = RiderMesh;
	LeanController->RiderMeshRef = RiderMesh;

	// Assign per-leg bone names (override these in BP_Cyclist defaults)
	// LeftPedal->TorqueBoneName  = FName("thigh_l");
	// RightPedal->TorqueBoneName = FName("thigh_r");
	// LeanController->LeanBoneName = FName("spine_01");

	// Apply Physical Animation blend to all bones below pelvis
	PhysicalAnimation->SetSkeletalMeshComponent(RiderMesh);
	FPhysicalAnimationData AnimData;
	AnimData.bIsLocalSimulation = false;
	AnimData.OrientationStrength = 1000.0f;
	AnimData.AngularVelocityStrength = 100.0f;
	AnimData.PositionStrength = 0.0f; // don't lock position — let physics drive it
	AnimData.VelocityStrength = 0.0f;
	PhysicalAnimation->ApplyPhysicalAnimationSettingsBelow(FName("pelvis"), AnimData, true);
	PhysicalAnimation->SetStrengthMultiplyer(PhysicalAnimBlendWeight);

	// Register Enhanced Input mapping context
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ACyclistPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(IA_PedalLeft,  ETriggerEvent::Started,  this, &ACyclistPawn::HandlePedalLeft);
		EIC->BindAction(IA_PedalLeft,  ETriggerEvent::Completed, this, &ACyclistPawn::HandlePedalLeft);
		EIC->BindAction(IA_PedalRight, ETriggerEvent::Started,  this, &ACyclistPawn::HandlePedalRight);
		EIC->BindAction(IA_PedalRight, ETriggerEvent::Completed, this, &ACyclistPawn::HandlePedalRight);
		EIC->BindAction(IA_Lean,       ETriggerEvent::Triggered, this, &ACyclistPawn::HandleLean);
	}
}

void ACyclistPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CheckForFall(DeltaTime);
}

// ────────────────────────────────────────────────────────────────────────────
// Input Handlers
// ────────────────────────────────────────────────────────────────────────────

void ACyclistPawn::HandlePedalLeft(const FInputActionValue& Value)
{
	const bool bPressed = Value.Get<bool>();
	LeftPedal->Activate(bPressed);
}

void ACyclistPawn::HandlePedalRight(const FInputActionValue& Value)
{
	const bool bPressed = Value.Get<bool>();
	RightPedal->Activate(bPressed);
}

void ACyclistPawn::HandleLean(const FInputActionValue& Value)
{
	const float Axis = Value.Get<float>();
	LeanController->HandleLean(Axis);
}

// ────────────────────────────────────────────────────────────────────────────
// Fall Detection
// ────────────────────────────────────────────────────────────────────────────

void ACyclistPawn::CheckForFall(float DeltaTime)
{
	// Measure bike frame tilt from world up vector
	const FVector BikeUp = BikeFrameMesh->GetUpVector();
	const float TiltAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(BikeUp, FVector::UpVector)));

	if (TiltAngle > FallAngleThreshold)
	{
		if (!bFallTimerRunning)
		{
			bFallTimerRunning = true;
			GetWorldTimerManager().SetTimer(
				FallConfirmTimer,
				[this]() { OnFall(); bFallTimerRunning = false; },
				FallConfirmDelay,
				false
			);
		}
	}
	else
	{
		// Player recovered — cancel the fall timer
		if (bFallTimerRunning)
		{
			GetWorldTimerManager().ClearTimer(FallConfirmTimer);
			bFallTimerRunning = false;
		}
	}
}

// ────────────────────────────────────────────────────────────────────────────
// Gameplay
// ────────────────────────────────────────────────────────────────────────────

float ACyclistPawn::GetCurrentHeight() const
{
	return GetActorLocation().Z - SpawnLocation.Z;
}

void ACyclistPawn::OnFall_Implementation()
{
	// Default: log the fall. Override in BP_Cyclist to add:
	// - screen shake
	// - respawn to last checkpoint
	// - death SFX / Niagara particle burst
	UE_LOG(LogTemp, Warning, TEXT("CyclistPawn: Fall confirmed!"));
}
