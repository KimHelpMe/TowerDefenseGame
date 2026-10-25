#include "SingleAttackComponent.h"
#include "TowerDefenseLog.h"
#include "Projectile.h"
#include "TowerStatComponent.h"
#include "Enemy.h"

void USingleAttackComponent::ExecuteAttack(const TArray<AEnemy*>& Targets)
{
	UE_LOG(LogTowerDefense, Verbose, TEXT("ExecuteAttack called, Targets: %d"), Targets.Num());

	// Targets는 배열(TArray). nullptr이 아님. 
	if (Targets.IsEmpty())
	{
		return;
	}

	if (!ProjectileClass)
	{
		UE_LOG(LogTowerDefense, Warning, TEXT("ProjectileClass is NULL"));
		return;
	}

	FVector FireMuzzleLocation = GetMuzzleLocation();
	FVector Direction = (Targets[0]->GetActorLocation() - GetMuzzleLocation()).GetSafeNormal();
	FRotator TargetRotation = Direction.Rotation();

	AProjectile* fireProjectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, FireMuzzleLocation, TargetRotation);
	if (!fireProjectile) return;
	fireProjectile->TowerStatComponent = GetOwner()->GetComponentByClass<UTowerStatComponent>();
	fireProjectile->HitEffect = HitEffect;
	fireProjectile->HitColor = GetHitColor();
}


