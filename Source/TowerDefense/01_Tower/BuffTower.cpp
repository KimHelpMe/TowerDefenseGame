

#include "BuffTower.h"
#include "BuffTypeComponent.h"

ABuffTower::ABuffTower()
{
	BuffRangeComponent = CreateDefaultSubobject<USphereComponent>(TEXT("BuffRangeComponent"));
	BuffRangeComponent->SetupAttachment(RootComponent);
	BuffRange = 300.f;
	BuffRangeComponent->SetSphereRadius(BuffRange);


}

void ABuffTower::BeginPlay()
{
	Super::BeginPlay();


	BuffComponent = GetComponentByClass<UBuffTypeComponent>();
}

float ABuffTower::GetRangeRadius()
{
	if (IsValid(BuffComponent) && BuffComponent->BuffRadius > 0.f)
	{
		return BuffComponent->BuffRadius;
	}
	return IsValid(BuffRangeComponent) ? BuffRangeComponent->GetScaledSphereRadius() : 0.f;
}
