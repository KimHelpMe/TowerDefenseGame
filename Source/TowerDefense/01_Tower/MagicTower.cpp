#include "MagicTower.h"
#include "ManaComponent.h"
#include "AttackTypeComponent.h"

AMagicTower::AMagicTower()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMagicTower::BeginPlay()
{
	Super::BeginPlay();

	ManaComponent = GetComponentByClass<UManaComponent>();
	if (ManaComponent)
	{
		ManaComponent->OnManaFull.AddDynamic(this, &AMagicTower::OnManaFull);
	}
}

void AMagicTower::OnManaFull()
{
	// 공격할 대상이 있을 때만 마나를 소모하고 공격한다.
	if (!PruneTargets() || !IsValid(ManaComponent) || !IsValid(AttackTypeComponent)) return;

	ManaComponent->ConsumeMana();
	Attack();
}
