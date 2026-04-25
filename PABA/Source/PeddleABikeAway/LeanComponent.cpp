// LeanComponent.cpp
// Event-driven lean/balance handler — not ticked (cheaper, purely input-driven)

#include "LeanComponent.h"
#include "Components/SkeletalMeshComponent.h"

ULeanComponent::ULeanComponent()
{
	// Not ticked — purely event-driven from HandleLean in the pawn
	PrimaryComponentTick.bCanEverTick = false;
}

void ULeanComponent::HandleLean(float AxisValue)
{
	const float MappedAxis = RemapDeadzone(AxisValue);

	if (FMath::IsNearlyZero(MappedAxis)) return;

	if (RiderMeshRef && LeanBoneName != NAME_None)
	{
		// Apply corrective torque to the torso bone
		// Positive axis = lean right (positive Y), negative = lean left
		// bAccelChange=true for mass-independent feel
		const FVector LeanTorque = FVector(0.0f, MappedAxis * MaxLeanTorque, 0.0f);
		RiderMeshRef->AddTorqueInDegrees(LeanTorque, LeanBoneName, /*bAccelChange=*/true);
	}
}

float ULeanComponent::RemapDeadzone(float RawAxis) const
{
	// If inside deadzone, return zero
	if (FMath::Abs(RawAxis) < InputDeadzone) return 0.0f;

	// Remap: map [Deadzone, 1.0] -> [0.0, 1.0] preserving sign
	// This means full lean always reaches MaxLeanTorque, even at the edges
	const float Sign = FMath::Sign(RawAxis);
	const float AbsValue = FMath::Abs(RawAxis);
	const float Remapped = (AbsValue - InputDeadzone) / (1.0f - InputDeadzone);
	return Sign * FMath::Clamp(Remapped, 0.0f, 1.0f);
}
