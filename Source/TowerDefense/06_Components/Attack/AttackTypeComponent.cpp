#include "AttackTypeComponent.h"
#include "BaseTower.h"
#include "Enemy.h"
#include "Kismet/GameplayStatics.h"

UAttackTypeComponent::UAttackTypeComponent()
{
	PrimaryComponentTick.bCanEverTick = false; // Tick does nothing; BP Event Tick re-enables it on compile.
}

void UAttackTypeComponent::BeginPlay()
{
	Super::BeginPlay();


	
}

void UAttackTypeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UAttackTypeComponent::ExecuteAttack(const TArray<AEnemy*>& Targets)
{
}

FVector UAttackTypeComponent::GetMuzzleLocation()
{
	ABaseTower* Tower = Cast<ABaseTower>(GetOwner());
	if (!Tower) return FVector::ZeroVector;
	
		return Tower->MuzzleComponent->GetComponentLocation();
	
}

