#include "MultiAttackComponent.h"
#include "Projectile.h"
#include "TowerStatComponent.h"
#include "Enemy.h"

UMultiAttackComponent::UMultiAttackComponent()
{
	MaxTargetCount = 2;
}

void UMultiAttackComponent::ExecuteAttack(TArray<AEnemy*> Targets)
{

	UE_LOG(LogTemp, Warning, TEXT("MultiAttack called, Targets: %d, MaxTargetCount: %d"), Targets.Num(), MaxTargetCount);

	// Targets는 배열(TArray). nullptr이 아님. 
	if (Targets.IsEmpty())
	{
		return;
	}

	if (!ProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("MultiAttack ProjectileClass is NULL"));
		return;
	}


	for (int i = 0; i < MaxTargetCount && i <Targets.Num(); ++i)
	{
		FVector FireMuzzleLocation = GetMuzzleLocation();
		FVector Direction = (Targets[i]->GetActorLocation() - GetMuzzleLocation()).GetSafeNormal();
		FRotator TargetRotation = Direction.Rotation();

		AProjectile* fireProjectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, FireMuzzleLocation, TargetRotation);
		if (!fireProjectile)
		{
			UE_LOG(LogTemp, Warning, TEXT("MultiAttack SpawnActor failed"));
			return;
		}
		fireProjectile->TowerStatComponent = GetOwner()->GetComponentByClass<UTowerStatComponent>();
	}
}
