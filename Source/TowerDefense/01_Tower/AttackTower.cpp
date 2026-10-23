#include "AttackTower.h"
#include "Projectile.h"
#include "Enemy.h"
#include "AttackTypeComponent.h"
#include "AirAbilityComponent.h"
#include "TowerStatComponent.h"
#include "GameplayTagContainer.h"
#include "Kismet/GameplayStatics.h"
#include "TowerDefenseGameMode.h"

AAttackTower::AAttackTower()
{
	PrimaryActorTick.bCanEverTick = true;
}


void AAttackTower::BeginPlay()
{
	Super::BeginPlay();



	if (TowerAttackRangeComponent)
	{

		TowerAttackRangeComponent->OnComponentBeginOverlap.AddDynamic(this, &AAttackTower::OnEnemyEnter);
		
	}


	TowerAttackRangeComponent->OnComponentEndOverlap.AddDynamic(this, &AAttackTower::OnEnemyExit);
	AttackTypeComponent = GetComponentByClass<UAttackTypeComponent>();
	UTowerStatComponent* TowerStat = GetComponentByClass<UTowerStatComponent>();
	ATowerDefenseGameMode* GameMode = Cast<ATowerDefenseGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	if (TowerStat)
	{
		float AttackRangeBonus = GameMode ? GameMode->GetUpgradeBonus(
			UStatComponent::GetTag("Upgrade.AttackRange")) : 0.f;
		float FinalAttackRange = TowerStat->GetStat(UStatComponent::GetTag("Stat.AttackRange")) * (1.f + AttackRangeBonus);

		TowerAttackRangeComponent->SetSphereRadius(FinalAttackRange);



		float AttackSpeedBonus = GameMode ? GameMode->GetUpgradeBonus(
			UStatComponent::GetTag("Upgrade.AttackSpeed")) : 0.f;

		float FinalAttackSpeed = TowerStat->GetStat(UStatComponent::GetTag("Stat.AttackSpeed")) * (1.f - AttackSpeedBonus);

		GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AAttackTower::Attack_Implementation, FinalAttackSpeed, true);
	}
	else
	{
		TowerAttackRangeComponent->SetSphereRadius(500.f);
		GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AAttackTower::Attack_Implementation, 1.f, true);
	}
}

void AAttackTower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CurrentTargets.RemoveAll([](AEnemy* Enemy)
		{
			return !IsValid(Enemy);
		});

}


void AAttackTower::OnEnemyEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AEnemy* enemy = Cast<AEnemy>(OtherActor);

	if(!enemy)
	{
		return;
	}


	UTowerStatComponent* TowerStat = GetComponentByClass<UTowerStatComponent>();


	// 공중 유닛인지 확인
	UAirAbilityComponent* AirComp = enemy->GetComponentByClass<UAirAbilityComponent>();

	if (AirComp)
	{
		if (TowerStat && TowerStat->Abilities.HasTag(
			UStatComponent::GetTag(("Ability.AttackAir"))))
		{
			CurrentTargets.Add(enemy);
			SetTurretTarget(CurrentTargets[0]);
		}
		// 공중 공격 불가 타워면 무시
	}
	else
	{
		if (TowerStat && TowerStat->Abilities.HasTag(
			UStatComponent::GetTag(("Ability.AttackGround"))))
		{
			CurrentTargets.Add(enemy);
			SetTurretTarget(CurrentTargets[0]);
		}
	}

}

void AAttackTower::OnEnemyExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
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



void AAttackTower::Attack_Implementation()
{
	Super::Attack_Implementation();

	if (!AttackTypeComponent)
	{

		return;
	}
	AttackTypeComponent->ExecuteAttack(CurrentTargets);
}


float AAttackTower::GetRangeRadius()
{
	return IsValid(TowerAttackRangeComponent) ? TowerAttackRangeComponent->GetScaledSphereRadius() : 0.f;
}
