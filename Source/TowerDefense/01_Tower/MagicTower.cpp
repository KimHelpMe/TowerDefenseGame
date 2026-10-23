#include "MagicTower.h"
#include "Projectile.h"
#include "Enemy.h"
#include "ManaComponent.h"
#include "AttackTypeComponent.h"
#include "AirAbilityComponent.h"
#include "GameplayTagContainer.h"
#include "TowerStatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TowerDefenseGameMode.h"

AMagicTower::AMagicTower()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMagicTower::BeginPlay()
{
	Super::BeginPlay();

	TowerAttackRangeComponent->OnComponentBeginOverlap.AddDynamic(this, &AMagicTower::OnEnemyEnter);
	TowerAttackRangeComponent->OnComponentEndOverlap.AddDynamic(this, &AMagicTower::OnEnemyExit);

	AttackTypeComponent = GetComponentByClass<UAttackTypeComponent>();
	ManaComponent = GetComponentByClass<UManaComponent>();

	UTowerStatComponent* TowerStat = GetComponentByClass<UTowerStatComponent>();
	ATowerDefenseGameMode* GameMode = Cast<ATowerDefenseGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	if (TowerStat)
	{
		float AttackRangeBonus = GameMode ? GameMode->GetUpgradeBonus(
			UStatComponent::GetTag("Upgrade.AttackRange")) : 0.f;
		float FinalAttackRange = TowerStat->GetStat(UStatComponent::GetTag("Stat.AttackRange")) * (1.f + AttackRangeBonus);
		TowerAttackRangeComponent->SetSphereRadius(FinalAttackRange);
	}

	else
	{
		// fallback.
		TowerAttackRangeComponent->SetSphereRadius(500.f);
	}

	if (ManaComponent)
	{
		ManaComponent->OnManaFull.AddDynamic(this, &AMagicTower::OnManaFull);
	}

}


void AMagicTower::Attack_Implementation()
{
	CurrentTargets.RemoveAll([](AEnemy* Enemy)
	{
		return !IsValid(Enemy) || Enemy->bIsDead;
	});
	if (CurrentTargets.IsEmpty() || !IsValid(AttackTypeComponent)) return;

	Super::Attack_Implementation();
	AttackTypeComponent->ExecuteAttack(CurrentTargets);
}


void AMagicTower::OnEnemyEnter(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{

	AEnemy* enemy = Cast<AEnemy>(OtherActor);
	if (!IsValid(enemy) || enemy->bIsDead || CurrentTargets.Contains(enemy)) return;


	UTowerStatComponent* TowerStat = GetComponentByClass<UTowerStatComponent>();


	// 공중 유닛인지 확인
	UAirAbilityComponent* AirComp = enemy->GetComponentByClass<UAirAbilityComponent>();

	if (AirComp)
	{
		if (TowerStat && TowerStat->Abilities.HasTag(
			UStatComponent::GetTag("Ability.AttackAir")))
		{
			CurrentTargets.Add(enemy);
			SetTurretTarget(CurrentTargets[0]);
		}
		// 공중 공격 불가 타워면 무시
	}
	else
	{
		if (TowerStat && TowerStat->Abilities.HasTag(
			UStatComponent::GetTag("Ability.AttackGround")))
		{
			CurrentTargets.Add(enemy);
			SetTurretTarget(CurrentTargets[0]);
		}
	}
}


void AMagicTower::OnEnemyExit(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	AEnemy* enemy = Cast<AEnemy>(OtherActor);
	if (enemy)
	{
		CurrentTargets.Remove(enemy);

		if (CurrentTargets.IsEmpty())
		{
			SetTurretTarget(nullptr);
		}
		else
		{
			SetTurretTarget(CurrentTargets[0]);
		}
	}
}

void AMagicTower::OnManaFull()
{
	CurrentTargets.RemoveAll([](AEnemy* Enemy)
	{
		return !IsValid(Enemy) || Enemy->bIsDead;
	});
	if (CurrentTargets.IsEmpty() || !IsValid(ManaComponent) || !IsValid(AttackTypeComponent)) return;

	ManaComponent->ConsumeMana();
	Attack();
}
