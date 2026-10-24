#include "EnemyAbilityComponent.h"


UEnemyAbilityComponent::UEnemyAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = false; // Tick does nothing; BP Event Tick re-enables it on compile.
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

