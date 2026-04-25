// LeanComponent.h
// Handles the rider's balance axis via analog input
// Drop into: Source/PeddleABikeAway/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LeanComponent.generated.h"

class USkeletalMeshComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PEDDLEABIKEAWAY_API ULeanComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULeanComponent();

	/**
	 * Called by the pawn's input handler each frame the lean axis has input.
	 * AxisValue is in the range [-1, 1] (from analog stick or mouse axis).
	 */
	UFUNCTION(BlueprintCallable, Category = "Lean")
	void HandleLean(float AxisValue);

	// ---------- Tuning ----------

	/**
	 * Maximum torque applied for a full lean input (Newton-centimetres).
	 * Primary balance tuning knob:
	 *   Lower = harder to correct a fall (more chaotic, harder game)
	 *   Higher = too easy to stay upright (less funny)
	 * Start at 2200, tune based on PedalComponent torque values.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lean|Tuning")
	float MaxLeanTorque = 2200.0f;

	/**
	 * Deadzone: axis values within this range are ignored (treated as zero).
	 * Prevents drift from slightly off-centre sticks.
	 * Values outside the deadzone are remapped to the full 0-1 range
	 * so leaning never feels sluggish at the edges.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lean|Tuning")
	float InputDeadzone = 0.12f;

	/**
	 * Name of the torso bone to apply lean torque to.
	 * E.g. "spine_01" or "pelvis".
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lean|Setup")
	FName LeanBoneName = NAME_None;

	/** Reference to rider skeletal mesh */
	UPROPERTY(BlueprintReadWrite, Category = "Lean|Setup")
	USkeletalMeshComponent* RiderMeshRef = nullptr;

private:
	// Remaps axis value outside deadzone to full 0-1 range
	float RemapDeadzone(float RawAxis) const;
};
