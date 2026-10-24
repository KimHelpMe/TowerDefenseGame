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

	if (IsValid(TowerAttackRangeComponent))
	{
		TowerAttackRangeComponent->OnComponentBeginOverlap.AddDynamic(this, &AAttackTower::OnEnemyEnter);
		TowerAttackRangeComponent->OnComponentEndOverlap.AddDynamic(this, &AAttackTower::OnEnemyExit);
	}

	AttackTypeComponent = GetComponentByClass<UAttackTypeComponent>();

	if (IsValid(TowerAttackRangeComponent))
	{
		UTowerStatComponent* TowerStat = GetComponentByClass<UTowerStatComponent>();
		ATowerDefenseGameMode* GameMode = Cast<ATowerDefenseGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
		const float BaseRange = TowerStat ? TowerStat->GetStat(UStatComponent::GetTag("Stat.AttackRange")) : 500.f;
		const float RangeBonus = GameMode ? GameMode->GetUpgradeBonus(UStatComponent::GetTag("Upgrade.AttackRange")) : 0.f;
		const float SafeRangeBonus = FMath::IsFinite(RangeBonus) ? RangeBonus : 0.f;
		const float FinalRange = FMath::IsFinite(BaseRange) ? FMath::Max(0.f, BaseRange * (1.f + SafeRangeBonus)) : 500.f;
		TowerAttackRangeComponent->SetSphereRadius(FinalRange);
	}

	RefreshAttackTimer();
}

void AAttackTower::RefreshAttackTimer()
{
	if (!GetWorld()) return;

	UTowerStatComponent* TowerStat = GetComponentByClass<UTowerStatComponent>();
	ATowerDefenseGameMode* GameMode = Cast<ATowerDefenseGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	const float BaseAttackSpeed = TowerStat ? TowerStat->GetStat(UStatComponent::GetTag("Stat.AttackSpeed")) : 1.f;
	const float AttackSpeedBonus = GameMode ? GameMode->GetUpgradeBonus(UStatComponent::GetTag("Upgrade.AttackSpeed")) : 0.f;
	const float SafeAttackSpeedBonus = FMath::IsFinite(AttackSpeedBonus) ? AttackSpeedBonus : 0.f;
	const float FinalAttackSpeed = FMath::IsFinite(BaseAttackSpeed)
		? FMath::Max(0.01f, BaseAttackSpeed * (1.f - SafeAttackSpeedBonus))
		: 1.f;

	// 간격이 그대로면 타이머를 다시 걸지 않는다(버프 재적용마다 공격 주기가 리셋되는 것 방지).
	if (GetWorldTimerManager().IsTimerActive(AttackTimerHandle)
		&& FMath::IsNearlyEqual(AppliedAttackInterval, FinalAttackSpeed))
	{
		return;
	}
	AppliedAttackInterval = FinalAttackSpeed;

	GetWorldTimerManager().SetTimer(
		AttackTimerHandle,
		this,
		&AAttackTower::HandleAttackTimer,
		FinalAttackSpeed,
		true);
}

void AAttackTower::HandleAttackTimer()
{
	// BlueprintNativeEvent 진입점으로 호출해야 BP에서 Attack을 override했을 때도 그 구현이 실행된다.
	Attack();
}
void AAttackTower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CurrentTargets.RemoveAll([](AEnemy* Enemy)
	{
		return !IsValid(Enemy) || Enemy->bIsDead;
	});
	SetTurretTarget(CurrentTargets.IsEmpty() ? nullptr : CurrentTargets[0]);
}


void AAttackTower::OnEnemyEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AEnemy* enemy = Cast<AEnemy>(OtherActor);

	if (!IsValid(enemy) || enemy->bIsDead || CurrentTargets.Contains(enemy)) return;


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
	CurrentTargets.RemoveAll([](AEnemy* Enemy)
	{
		return !IsValid(Enemy) || Enemy->bIsDead;
	});
	if (CurrentTargets.IsEmpty() || !IsValid(AttackTypeComponent)) return;

	Super::Attack_Implementation();
	AttackTypeComponent->ExecuteAttack(CurrentTargets);
}


float AAttackTower::GetRangeRadius()
{
	return IsValid(TowerAttackRangeComponent) ? TowerAttackRangeComponent->GetScaledSphereRadius() : 0.f;
}
