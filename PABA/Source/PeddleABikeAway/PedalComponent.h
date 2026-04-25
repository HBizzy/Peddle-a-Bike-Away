// PedalComponent.h
// One instance per leg — applies torque to drive the bike upward/forward
// Drop into: Source/PeddleABikeAway/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PedalComponent.generated.h"

class USkeletalMeshComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PEDDLEABIKEAWAY_API UPedalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPedalComponent();

	/**
	 * Call this when the player presses the pedal button.
	 * IsPressed = true on press, false on release.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pedal")
	void Activate(bool IsPressed);

	// ---------- Tuning (all BlueprintReadWrite for live PIE adjustment) ----------

	/**
	 * Peak torque in Newton-centimetres applied to the leg physics body.
	 * Too low = bike barely moves. Too high = instant flip.
	 * Start at 180000, tune from there.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedal|Tuning")
	float PeakTorque = 180000.0f;

	/**
	 * Duration (seconds) over which torque ramps up and eases out.
	 * Uses a sqrt curve for a natural "push then release" feel.
	 * 0.18s is a good starting point.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedal|Tuning")
	float TorqueDuration = 0.18f;

	/**
	 * Cooldown between pedal strokes in seconds.
	 * Prevents button mashing from breaking physics balance.
	 * 0.12s is the minimum safe value.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedal|Tuning")
	float CooldownTime = 0.12f;

	/**
	 * Name of the bone in the Physics Asset to apply torque to.
	 * E.g. "thigh_l" for left leg, "thigh_r" for right leg.
	 * Set this per-instance in Blueprint defaults.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedal|Setup")
	FName TorqueBoneName = NAME_None;

	/** Direction vector for the torque. Default drives forward pedalling motion. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedal|Setup")
	FVector TorqueAxis = FVector(0.0f, 1.0f, 0.0f);

	/** Reference to the rider's skeletal mesh for physics body access */
	UPROPERTY(BlueprintReadWrite, Category = "Pedal|Setup")
	USkeletalMeshComponent* RiderMeshRef = nullptr;

	/** True while a pedal stroke is actively applying torque */
	UPROPERTY(BlueprintReadOnly, Category = "Pedal")
	bool bIsActive = false;

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

private:
	float StrokeTimer = 0.0f;
	float CooldownTimer = 0.0f;
	bool bOnCooldown = false;

	// Eases torque out using sqrt curve — natural push-then-release feel
	float ComputeTorqueThisFrame(float NormalisedT) const;
};
