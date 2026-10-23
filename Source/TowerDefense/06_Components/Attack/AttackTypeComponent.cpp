#include "AttackTypeComponent.h"
#include "BaseTower.h"
#include "Enemy.h"
#include "Kismet/GameplayStatics.h"

UAttackTypeComponent::UAttackTypeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAttackTypeComponent::BeginPlay()
{
	Super::BeginPlay();


	
}

void UAttackTypeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UAttackTypeComponent::ExecuteAttack(TArray<AEnemy*> Targets)
{
}

FVector UAttackTypeComponent::GetMuzzleLocation()
{
	ABaseTower* Tower = Cast<ABaseTower>(GetOwner());
	if (!Tower) return FVector::ZeroVector;
	
		return Tower->MuzzleComponent->GetComponentLocation();
	
}

