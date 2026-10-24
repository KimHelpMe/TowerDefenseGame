
#include "SynergyManager.h"
#include "BaseTower.h"
#include "TowerStatComponent.h"
#include "EpicTowerManager.h"
#include "Kismet/GameplayStatics.h"

ASynergyManager::ASynergyManager()
{
 	
	PrimaryActorTick.bCanEverTick = false; // Tick does nothing; BP Event Tick re-enables it on compile.

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

	if (FieldTowers.Remove(Tower) > 0)
	{
		UpdateSynergies();
	}
}

void ASynergyManager::UpdateSynergies()
{
	// 파괴되었거나 GC로 null이 된 타워 정리.
	FieldTowers.RemoveAll([](const ABaseTower* Tower)
	{
		return !IsValid(Tower);
	});

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

		// 현재 단계 버프 목록(0 단계면 버프 없음).
		TArray<FBuffEffect> StepBuffs;
		if (NewStep > 0)
		{
			for (FBuffEffect Effect : Row->Steps[NewStep - 1].Effects)
			{
				Effect.Source = this;
				Effect.SourceKey = BuffSourceKey;
				StepBuffs.Add(Effect);
			}
		}

		// 단계가 그대로여도 새로 들어온 타워가 버프를 받도록 해당 태그 타워 전체를 갱신.
		// 이미 같은 버프가 걸린 타워는 ReplaceBuffsByKey가 재계산 없이 넘어간다.
		for (ABaseTower* Tower : FieldTowers)
		{
			UTowerStatComponent* Stat = Tower->GetComponentByClass<UTowerStatComponent>();
			if (!Stat) continue;
			if (!Stat->SynergyTags.HasTag(Row->SynergyTag)) continue;

			Stat->ReplaceBuffsByKey(BuffSourceKey, StepBuffs);
		}
		// ActiveSynergySteps 업데이트.
		ActiveSynergySteps.Add(Row->SynergyTag, NewStep);
	}

	// SynergyCountMap 업데이트
	SynergyCountMap = TagCounts;

	// 위젯에 알림
	OnSynergyUpdated.Broadcast();

}