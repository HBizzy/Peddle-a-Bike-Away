// PedalComponent.cpp
// Applies physics torque to a leg bone when the player pedals

#include "PedalComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Math/UnrealMathUtility.h"

UPedalComponent::UPedalComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPedalComponent::Activate(bool IsPressed)
{
	if (IsPressed && !bOnCooldown)
	{
		bIsActive = true;
		StrokeTimer = 0.0f;
	}
	else if (!IsPressed)
	{
		// Allow early release — stroke will still complete its natural curve
		// but the bIsActive flag lets the pawn know the key is up
	}
}

void UPedalComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Tick down cooldown
	if (bOnCooldown)
	{
		CooldownTimer -= DeltaTime;
		if (CooldownTimer <= 0.0f)
		{
			bOnCooldown = false;
			CooldownTimer = 0.0f;
		}
	}

	// Active stroke in progress
	if (bIsActive)
	{
		StrokeTimer += DeltaTime;
		const float NormalisedT = FMath::Clamp(StrokeTimer / TorqueDuration, 0.0f, 1.0f);
		const float Torque = ComputeTorqueThisFrame(NormalisedT);

		if (RiderMeshRef && TorqueBoneName != NAME_None)
		{
			// Apply torque to the specific leg bone in the Physics Asset
			// bAccelChange=true means it's treated as an angular acceleration
			// (mass-independent), which gives more consistent feel regardless of ragdoll weight
			RiderMeshRef->AddTorqueInDegrees(
				TorqueAxis * Torque,
				TorqueBoneName,
				/*bAccelChange=*/true
			);
		}

		// Stroke complete
		if (StrokeTimer >= TorqueDuration)
		{
			bIsActive = false;
			StrokeTimer = 0.0f;
			bOnCooldown = true;
			CooldownTimer = CooldownTime;
		}
	}
}

float UPedalComponent::ComputeTorqueThisFrame(float NormalisedT) const
{
	// Sqrt ease-out: fast initial push, smooth release at end of stroke
	// Graph: starts at 0, peaks near NormalisedT=0.25, eases to 0 at 1.0
	// This is more physically natural than a flat burst
	const float EasedT = FMath::Sqrt(1.0f - FMath::Square(NormalisedT - 1.0f));
	return PeakTorque * EasedT;
}
