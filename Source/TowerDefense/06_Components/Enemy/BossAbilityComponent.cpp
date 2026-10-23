
#include "BossAbilityComponent.h"
#include "Enemy.h"
#include "FBuffEffect.h"
#include "StatComponent.h"
#include "EnemyStatComponent.h"

void UBossAbilityComponent::OnPhaseChanged(int32 NewPhase)
{
	CurrentPhase = NewPhase;
	SpawnMinions();

	ApplyPhaseBuffs(NewPhase);
	UE_LOG(LogTemp, Warning, TEXT("Boss Phase Changed to %d"), NewPhase);
}

void UBossAbilityComponent::SpawnMinions()
{
	AEnemy* OwnerBoss = Cast<AEnemy>(GetOwner());
	if (!OwnerBoss) return;

	for (int32 i = 0; i < MinionSpawnCount; i++)
	{
		// 스폰 위치에 랜덤 오프셋 추가
		FVector SpawnLocation = OwnerBoss->GetActorLocation();
		SpawnLocation.X += FMath::RandRange(-50.f, 50.f);
		SpawnLocation.Y += FMath::RandRange(-50.f, 50.f);

		// Owner 위치에 스폰
		AEnemy* SpawnedEnemy = GetWorld()->SpawnActor<AEnemy>(
			MinionClass,
			SpawnLocation,
			FRotator::ZeroRotator
		);

		// splinePath와 CurrentSplinePoint 전달
		if (SpawnedEnemy)
		{
			SpawnedEnemy->splinePath = OwnerBoss->splinePath;
			SpawnedEnemy->CurrentSplinePoint = OwnerBoss->CurrentSplinePoint;
		}
	}

}

void UBossAbilityComponent::ApplyPhaseBuffs(int32 Phase)
{
	UEnemyStatComponent* BossStat = GetOwner()->GetComponentByClass<UEnemyStatComponent>();
	if (!BossStat) return;

	if (Phase == 2)
	{
		FBuffEffect SpeedEffect;
		SpeedEffect.StatTag = UStatComponent::GetTag("Stat.MoveSpeed");
		SpeedEffect.Value = 0.3f;
		SpeedEffect.ValueType = EBuffValueType::Percent;
		SpeedEffect.Source = GetOwner();
		BossStat->ModifyStat(SpeedEffect);

		FBuffEffect DefenseEffect;
		DefenseEffect.StatTag = UStatComponent::GetTag("Stat.Defense");
		DefenseEffect.Value = 0.3f;
		DefenseEffect.ValueType = EBuffValueType::Percent;
		DefenseEffect.Source = GetOwner();
		BossStat->ModifyStat(DefenseEffect);
	}
	if (Phase == 3)
	{
		FBuffEffect SpeedEffect;
		SpeedEffect.StatTag = UStatComponent::GetTag("Stat.MoveSpeed");
		SpeedEffect.Value = 0.5f;
		SpeedEffect.ValueType = EBuffValueType::Percent;
		SpeedEffect.Source = GetOwner();
		BossStat->ModifyStat(SpeedEffect);

		FBuffEffect DefenseEffect;
		DefenseEffect.StatTag = UStatComponent::GetTag("Stat.Defense");
		DefenseEffect.Value = 0.5f;
		DefenseEffect.ValueType = EBuffValueType::Percent;
		DefenseEffect.Source = GetOwner();
		BossStat->ModifyStat(DefenseEffect);
	}
}


