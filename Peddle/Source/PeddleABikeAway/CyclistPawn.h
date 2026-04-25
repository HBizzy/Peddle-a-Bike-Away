// CyclistPawn.h
// Main controllable character for Peddle a Bike Away
// Drop into: Source/PeddleABikeAway/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "CyclistPawn.generated.h"

class USkeletalMeshComponent;
class UPhysicalAnimationComponent;
class USpringArmComponent;
class UCameraComponent;
class UPedalComponent;
class ULeanComponent;
class UPhysicsConstraintComponent;
class UInputMappingContext;
class UInputAction;

UCLASS()
class PEDDLEABIKEAWAY_API ACyclistPawn : public APawn
{
	GENERATED_BODY()

public:
	ACyclistPawn();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	virtual void Tick(float DeltaTime) override;

	// ---------- Components ----------

	/** Bike frame — separate physics body from the rider */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* BikeFrameMesh;

	/** Rider ragdoll mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* RiderMesh;

	/** Blends between animation pose and ragdoll physics */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPhysicalAnimationComponent* PhysicalAnimation;

	/** Left leg pedal controller */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPedalComponent* LeftPedal;

	/** Right leg pedal controller */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPedalComponent* RightPedal;

	/** Balance/lean controller */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	ULeanComponent* LeanController;

	/** Connects rider to bike so they can separate on falls */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPhysicsConstraintComponent* RiderBikeConstraint;

	/** Side-scroll camera arm */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArm;

	/** Main gameplay camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* Camera;

	// ---------- Enhanced Input ----------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_PedalLeft;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_PedalRight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_Lean;

	// ---------- Physics / Feel Tuning ----------

	/**
	 * Blend weight between animation target pose and ragdoll result.
	 * 0 = fully ragdoll (chaos), 1 = fully animated (boring).
	 * Default 0.45 — expose in Blueprint Details for live PIE tuning.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Tuning", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PhysicalAnimBlendWeight = 0.45f;

	/**
	 * Delay in seconds before a fall is confirmed.
	 * Prevents brief wobbles from instantly triggering death.
	 * 0.3s gives a natural recovery window.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Tuning")
	float FallConfirmDelay = 0.3f;

	/** Tilt angle (degrees) from vertical that counts as "fallen" */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Tuning")
	float FallAngleThreshold = 70.0f;

	// ---------- Gameplay ----------

	/** Returns height gained above spawn point (Z-delta). Pipe into HUD progress bar. */
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	float GetCurrentHeight() const;

	/** Called when fall is confirmed — override in BP for respawn VFX/SFX */
	UFUNCTION(BlueprintNativeEvent, Category = "Gameplay")
	void OnFall();
	virtual void OnFall_Implementation();

private:
	// Input handlers
	void HandlePedalLeft(const FInputActionValue& Value);
	void HandlePedalRight(const FInputActionValue& Value);
	void HandleLean(const FInputActionValue& Value);

	// Fall detection
	void CheckForFall(float DeltaTime);
	FTimerHandle FallConfirmTimer;
	bool bFallTimerRunning = false;

	// Spawn reference for height tracking
	FVector SpawnLocation;
};
