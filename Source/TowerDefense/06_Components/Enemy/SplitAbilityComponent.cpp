
#include "SplitAbilityComponent.h"
#include "TowerDefenseLog.h"
#include "Enemy.h"
#include "Kismet/GameplayStatics.h"

USplitAbilityComponent::USplitAbilityComponent()
{
}

void USplitAbilityComponent::OnEnemyDeathAbility()
{

	if (!SplitEnemyClass) return;

  // Owner를 AEnemy로 캐스팅
	AEnemy* OwnerEnemy = Cast<AEnemy>(GetOwner());
	if (!OwnerEnemy) return;

	// SplitCount 만큼 반복
	for (int32 i = 0; i < SplitCount; i++)
	{
		// 스폰 위치에 랜덤 오프셋 추가
		FVector SpawnLocation = OwnerEnemy->GetActorLocation();
		SpawnLocation.X += FMath::RandRange(-50.f, 50.f);
		SpawnLocation.Y += FMath::RandRange(-50.f, 50.f);

		// Owner 위치에 스폰
		AEnemy* SpawnedEnemy = GetWorld()->SpawnActor<AEnemy>(
			SplitEnemyClass,
			SpawnLocation,
			FRotator::ZeroRotator
		);

		// splinePath와 CurrentSplinePoint 전달
		if (SpawnedEnemy)
		{
			SpawnedEnemy->splinePath = OwnerEnemy->splinePath;
			// Stagger children along the spline so they do not move as one stacked unit.
			SpawnedEnemy->CurrentSplinePoint = FMath::Max(0.f, OwnerEnemy->CurrentSplinePoint - i * SplitSpacing);
			UE_LOG(LogTowerDefense, Verbose, TEXT("Split! Spawned Enemy %d"), i);
		}
	}
}
