
#include "BossAbilityComponent.h"
#include "TowerDefenseTags.h"
#include "TowerDefenseLog.h"
#include "Enemy.h"
#include "EnemySplinePath.h"
#include "FBuffEffect.h"
#include "StatComponent.h"
#include "EnemyStatComponent.h"

void UBossAbilityComponent::OnPhaseChanged(int32 NewPhase)
{
	if (NewPhase <= CurrentPhase || NewPhase > 3) return;

	CurrentPhase = NewPhase;
	SpawnMinions();
	ApplyPhaseBuffs(NewPhase);
	UE_LOG(LogTowerDefense, Log, TEXT("Boss Phase Changed to %d"), NewPhase);
}

void UBossAbilityComponent::SpawnMinions()
{
	AEnemy* OwnerBoss = Cast<AEnemy>(GetOwner());
	if (!IsValid(OwnerBoss) || !GetWorld() || !MinionClass || MinionSpawnCount <= 0) return;
	if (!IsValid(OwnerBoss->splinePath) || !IsValid(OwnerBoss->splinePath->splineComponent)) return;

	for (int32 i = 0; i < MinionSpawnCount; i++)
	{
		FVector SpawnLocation = OwnerBoss->GetActorLocation();
		SpawnLocation.X += FMath::RandRange(-50.f, 50.f);
		SpawnLocation.Y += FMath::RandRange(-50.f, 50.f);

		AEnemy* SpawnedEnemy = GetWorld()->SpawnActor<AEnemy>(
			MinionClass,
			SpawnLocation,
			FRotator::ZeroRotator
		);

		if (IsValid(SpawnedEnemy))
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
		SpeedEffect.StatTag = TDTags::Stat_MoveSpeed;
		SpeedEffect.Value = 0.3f;
		SpeedEffect.ValueType = EBuffValueType::Percent;
		SpeedEffect.Source = GetOwner();
		BossStat->ModifyStat(SpeedEffect);

		FBuffEffect DefenseEffect;
		DefenseEffect.StatTag = TDTags::Stat_Defense;
		DefenseEffect.Value = 0.3f;
		DefenseEffect.ValueType = EBuffValueType::Percent;
		DefenseEffect.Source = GetOwner();
		BossStat->ModifyStat(DefenseEffect);
	}
	if (Phase == 3)
	{
		FBuffEffect SpeedEffect;
		SpeedEffect.StatTag = TDTags::Stat_MoveSpeed;
		SpeedEffect.Value = 0.5f;
		SpeedEffect.ValueType = EBuffValueType::Percent;
		SpeedEffect.Source = GetOwner();
		BossStat->ModifyStat(SpeedEffect);

		FBuffEffect DefenseEffect;
		DefenseEffect.StatTag = TDTags::Stat_Defense;
		DefenseEffect.Value = 0.5f;
		DefenseEffect.ValueType = EBuffValueType::Percent;
		DefenseEffect.Source = GetOwner();
		BossStat->ModifyStat(DefenseEffect);
	}
}
