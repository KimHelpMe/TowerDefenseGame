#include "EnemyAbilityComponent.h"


UEnemyAbilityComponent::UEnemyAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}



void UEnemyAbilityComponent::BeginPlay()
{
	Super::BeginPlay();
}



void UEnemyAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UEnemyAbilityComponent::OnAbilityActivate()
{
}

void UEnemyAbilityComponent::OnEnemyDeathAbility()
{
}

