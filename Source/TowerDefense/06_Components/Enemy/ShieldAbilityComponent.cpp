
#include "ShieldAbilityComponent.h"

UShieldAbilityComponent::UShieldAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	ShieldAmount = 100.f;
	MaxShieldAmount = 100.f;

}

float UShieldAbilityComponent::AbsorbDamage(float IncomingDamage)
{
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
