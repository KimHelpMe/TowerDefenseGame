#include "EpicTowerManager.h"
#include "BaseTower.h"
#include "TowerStatComponent.h"
#include "GachaManager.h"
#include "SynergyManager.h"
#include "EpicNotifyWidget.h"
#include "Kismet/GameplayStatics.h"


AEpicTowerManager::AEpicTowerManager()
{
 
	PrimaryActorTick.bCanEverTick = true;

}

void AEpicTowerManager::BeginPlay()
{
	Super::BeginPlay();
	
}


void AEpicTowerManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEpicTowerManager::CheckConditions(const TArray<ABaseTower*>& FieldTowers)
{
	if (!EpicTowerDataTable) return;

	TArray<FName> RowNames = EpicTowerDataTable->GetRowNames();
	for (FName RowName : RowNames)
	{
		FEpicTowerDataRow* Row = EpicTowerDataTable->FindRow<FEpicTowerDataRow>(RowName, TEXT(""));
		if (!Row) continue;

		// 언락/합성 가능 목록에 이미 있으면 건너뜀.
		if (UnlockedEpicTowers.Contains(Row->EpicTowerClass)) continue;
		if (PendingFusionTowers.Contains(Row->EpicTowerClass)) continue;

		// 모든 조건 만족 여부 체크.
		bool bAllConditionsMet = true;
		for (const FEpicTowerConditionEntry& Condition : Row->Conditions)
		{
			bool bConditionMet = false;
			for (ABaseTower* Tower : FieldTowers)
			{
				UTowerStatComponent* Stat = Tower->GetComponentByClass<UTowerStatComponent>();
				if (!Stat) continue;

				if (!Condition.TowerClass) continue;

				if (Tower->IsA(Condition.TowerClass) &&
					Stat->CurrentLevel >= Condition.RequiredLevel)
				{
					bConditionMet = true;
					break;
				}

			}
			if (!bConditionMet)
			{
				bAllConditionsMet = false;
				break;
			}

		}

		// 모든 조건 만족인 경우.
		if (bAllConditionsMet)
		{
			if (Row->EpicType == EEpicTowerType::Unlock)
			{
				UnlockedEpicTowers.Add(Row->EpicTowerClass);

				// GachaManager EpicPool에 추가
				AGachaManager* GachaManager = Cast<AGachaManager>(
					UGameplayStatics::GetActorOfClass(GetWorld(), AGachaManager::StaticClass()));
				if (GachaManager)
				{
					GachaManager->EpicPool.Add(Row->EpicTowerClass);
				}
			}
			else if (Row->EpicType == EEpicTowerType::Fusion)
			{
				PendingFusionTowers.Add(Row->EpicTowerClass);

				// GachaManager PendingFusionTowers에도 추가 ← 누락
				AGachaManager* GachaManager = Cast<AGachaManager>(
					UGameplayStatics::GetActorOfClass(GetWorld(), AGachaManager::StaticClass()));
				if (GachaManager)
				{
					GachaManager->PendingFusionTowers.Add(Row->EpicTowerClass);
				}

			}
			// 알팀 텍스트 표시
			ShowNotifyText(Row->NotifyText);
		}

	}

}



void AEpicTowerManager::FuseEpicTower(
	TSubclassOf<AActor> EpicTowerClass,
	const TArray<ABaseTower*>& FieldTowers, 
	AGachaManager* GachaManager, 
	ASynergyManager* SynergyManager)
{

	TArray<FName> RowNames = EpicTowerDataTable->GetRowNames();
	for (FName RowName : RowNames)
	{
		FEpicTowerDataRow* Row = EpicTowerDataTable->FindRow<FEpicTowerDataRow>(RowName, TEXT(""));
		if (!Row) continue;
		if (Row->EpicTowerClass != EpicTowerClass) continue;

		// 조건에 맞는 재료 타워 찾아서 소멸.
		for (const FEpicTowerConditionEntry& Condition : Row->Conditions)
		{
			for (ABaseTower* Tower : FieldTowers)
			{
				UTowerStatComponent* Stat = Tower->GetComponentByClass<UTowerStatComponent>();
				if (!Stat) continue;
				if (Tower->GetClass() == Condition.TowerClass &&
					Stat->CurrentLevel >= Condition.RequiredLevel)
				{
					// SynergyManager에서 제거.
					if (SynergyManager)
					{
						SynergyManager->UnregisterTower(Tower);
					}
					// 타워 소멸.
					Tower->Destroy();
					break;
				}
			}
		}
		// PendingFusionTowers에서 제거.
		PendingFusionTowers.Remove(EpicTowerClass);
		if (GachaManager)
		{
			GachaManager->PendingFusionTowers.Remove(EpicTowerClass);
		}
	}


}



void AEpicTowerManager::ShowNotifyText(const FString& Text)
{
	if (!NotifyWidgetClass) return;

	UEpicNotifyWidget* NotifyWidget = CreateWidget<UEpicNotifyWidget>(GetWorld(), NotifyWidgetClass);
	if (!NotifyWidget) return;

	NotifyWidget->AddToViewport();
	NotifyWidget->SetNotifyText(Text);

	// 3초 후 자동 제거
	FTimerHandle NotifyTimerHandle;
	GetWorldTimerManager().SetTimer(NotifyTimerHandle, [NotifyWidget]()
		{
			if (IsValid(NotifyWidget))
			{
				NotifyWidget->RemoveFromParent();
			}
		}, 3.f, false);
}