

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
	if (!BuffRangeComponent) return 0.f;
	return BuffRangeComponent->GetScaledSphereRadius();
}