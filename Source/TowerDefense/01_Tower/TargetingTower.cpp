#include "TargetingTower.h"
#include "TowerDefenseTags.h"
#include "Enemy.h"
#include "AttackTypeComponent.h"
#include "AirAbilityComponent.h"
#include "TowerStatComponent.h"

void ATargetingTower::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(TowerAttackRangeComponent))
	{
		TowerAttackRangeComponent->OnComponentBeginOverlap.AddDynamic(this, &ATargetingTower::OnEnemyEnter);
		TowerAttackRangeComponent->OnComponentEndOverlap.AddDynamic(this, &ATargetingTower::OnEnemyExit);

		UTowerStatComponent* TowerStat = GetComponentByClass<UTowerStatComponent>();
		TowerAttackRangeComponent->SetSphereRadius(TowerStat ? TowerStat->GetFinalAttackRange() : 500.f);
	}

	AttackTypeComponent = GetComponentByClass<UAttackTypeComponent>();
}

void ATargetingTower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PruneTargets();
	SetTurretTarget(CurrentTargets.IsEmpty() ? nullptr : CurrentTargets[0]);
}

bool ATargetingTower::PruneTargets()
{
	CurrentTargets.RemoveAll([](AEnemy* Enemy)
	{
		return !IsValid(Enemy) || Enemy->bIsDead;
	});
	return !CurrentTargets.IsEmpty();
}

bool ATargetingTower::CanTarget(const AEnemy* Enemy) const
{
	const UTowerStatComponent* TowerStat = GetComponentByClass<UTowerStatComponent>();
	if (!TowerStat) return false;

	// 공중 유닛은 공중 공격 가능 타워만, 지상 유닛은 지상 공격 가능 타워만.
	const bool bIsAir = Enemy->GetComponentByClass<UAirAbilityComponent>() != nullptr;
	return TowerStat->Abilities.HasTag(bIsAir ? TDTags::Ability_AttackAir : TDTags::Ability_AttackGround);
}

void ATargetingTower::OnEnemyEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AEnemy* Enemy = Cast<AEnemy>(OtherActor);
	if (!IsValid(Enemy) || Enemy->bIsDead || CurrentTargets.Contains(Enemy)) return;
	if (!CanTarget(Enemy)) return;

	CurrentTargets.Add(Enemy);
	SetTurretTarget(CurrentTargets[0]);
}

void ATargetingTower::OnEnemyExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AEnemy* Enemy = Cast<AEnemy>(OtherActor);
	if (!Enemy) return;

	CurrentTargets.Remove(Enemy);
	SetTurretTarget(CurrentTargets.IsEmpty() ? nullptr : CurrentTargets[0]);
}

void ATargetingTower::Attack_Implementation()
{
	if (!PruneTargets() || !IsValid(AttackTypeComponent)) return;

	Super::Attack_Implementation();
	AttackTypeComponent->ExecuteAttack(CurrentTargets);
}
