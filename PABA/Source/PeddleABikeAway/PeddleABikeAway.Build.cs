// PeddleABikeAway.Build.cs
// UE5 module build file — add this to Source/PeddleABikeAway/

using UnrealBuildTool;

public class PeddleABikeAway : ModuleRules
{
	public PeddleABikeAway(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",

			// Enhanced Input — required for IA_PedalLeft, IA_PedalRight, IA_Lean
			"EnhancedInput",

			// Physical Animation Component
			"PhysicsCore",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			// Chaos Physics — ragdoll, bike frame collisions
			"ChaosVehicles",
			"PhysicsControl",

			// Niagara — dust/spark particles on falls (add when ready)
			// "Niagara",
		});
	}
}
