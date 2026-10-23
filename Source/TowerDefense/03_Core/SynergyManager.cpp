
#include "SynergyManager.h"
#include "BaseTower.h"
#include "TowerStatComponent.h"
#include "EpicTowerManager.h"
#include "Kismet/GameplayStatics.h"

ASynergyManager::ASynergyManager()
{
 	
	PrimaryActorTick.bCanEverTick = true;

}

void ASynergyManager::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASynergyManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASynergyManager::RegisterTower(ABaseTower* Tower)
{
	if (!Tower) return;
	FieldTowers.Add(Tower);
	UpdateSynergies();

	// EpicTowerManager에 조건 체크 요청
	AEpicTowerManager* EpicManager = Cast<AEpicTowerManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AEpicTowerManager::StaticClass()));
	if (EpicManager)
	{
		EpicManager->CheckConditions(FieldTowers);
	}
}


void ASynergyManager::UnregisterTower(ABaseTower* Tower)
{
	if (!Tower) return;

	FieldTowers.Remove(Tower);
	UpdateSynergies();
}

void ASynergyManager::UpdateSynergies()
{
	if (!SynergyDataTable) return;

	// 태그별 타워 수 계산.
	TMap<FGameplayTag, int32> TagCounts;
	for (ABaseTower* Tower : FieldTowers)
	{
		UTowerStatComponent* Stat = Tower->GetComponentByClass<UTowerStatComponent>();
		if (!Stat) continue;
		for (FGameplayTag Tag : Stat->SynergyTags)
		{
			TagCounts.FindOrAdd(Tag)++;
		}

	}

	// DataTable 순회.
	TArray<FName> RowNames = SynergyDataTable->GetRowNames();
	for (FName RowName : RowNames)
	{
		FSynergyDataRow* Row = SynergyDataTable->FindRow<FSynergyDataRow>(RowName, TEXT(""));
		if (!Row || !Row->SynergyTag.IsValid()) continue;
		const FName BuffSourceKey(*FString::Printf(TEXT("Synergy_%s"), *RowName.ToString()));

		// 현재 태그의 타워 수 가져오기
		int32 TowerCount = TagCounts.FindRef(Row->SynergyTag);

		// 활성화할 단계 결정
		int32 NewStep = 0;

		for (int32 i = 0; i < Row->Steps.Num(); i++)
		{
			if (TowerCount >= Row->Steps[i].RequireCount)
			{
				NewStep = i + 1;
			}
		}

		// 이전 단계와 다르면 버프 갱신.
		int32 PrevStep = ActiveSynergySteps.FindRef(Row->SynergyTag);
		if (NewStep != PrevStep)
		{
			for (ABaseTower* Tower : FieldTowers)
			{
				UTowerStatComponent* Stat = Tower->GetComponentByClass<UTowerStatComponent>();
				if (!Stat) continue;
				if (!Stat->SynergyTags.HasTag(Row->SynergyTag)) continue;

				// 현재 시너지 행의 버프만 제거.
				Stat->RemoveBuffByKey(BuffSourceKey);

				// 새 단계 버프 적용(0 이면 버프 없음).
				if (NewStep > 0)
				{
					for (FBuffEffect Effect : Row->Steps[NewStep - 1].Effects)
					{
						Effect.Source = this;
						Effect.SourceKey = BuffSourceKey;
						Stat->ModifyStat(Effect);
					}
				}

			}
			// ActiveSynergySteps 업데이트.
			ActiveSynergySteps.Add(Row->SynergyTag, NewStep);
		}
	}

	// SynergyCountMap 업데이트
	SynergyCountMap = TagCounts;

	// 위젯에 알림
	OnSynergyUpdated.Broadcast();

}