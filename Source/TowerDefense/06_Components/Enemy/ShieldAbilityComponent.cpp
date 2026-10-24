
#include "ShieldAbilityComponent.h"

UShieldAbilityComponent::UShieldAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = false; // Tick does nothing; BP Event Tick re-enables it on compile.

	ShieldAmount = 100.f;
	MaxShieldAmount = 100.f;

}

float UShieldAbilityComponent::AbsorbDamage(float IncomingDamage)
{
	// Negative or invalid damage must never increase the shield.
	if (!FMath::IsFinite(IncomingDamage) || IncomingDamage <= 0.f) return 0.f;

	if (IncomingDamage <= ShieldAmount)
	{
		ShieldAmount -= IncomingDamage;

		return 0.f;
	}

	else
	{
		float OverShieldDamage = IncomingDamage - ShieldAmount;
		ShieldAmount = 0.f;
		return OverShieldDamage;
	}
}
