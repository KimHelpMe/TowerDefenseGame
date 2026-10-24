#include "EpicTowerManager.h"
#include "BaseTower.h"
#include "TowerStatComponent.h"
#include "GachaManager.h"
#include "SynergyManager.h"
#include "TowerSlot.h"
#include "EpicNotifyWidget.h"
#include "Kismet/GameplayStatics.h"


AEpicTowerManager::AEpicTowerManager()
{
 
	PrimaryActorTick.bCanEverTick = false; // Tick does nothing; BP Event Tick re-enables it on compile.

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

		// 각 조건은 서로 다른 타워가 충족해야 함.
		bool bAllConditionsMet = true;
		TSet<ABaseTower*> MatchedTowers;
		for (const FEpicTowerConditionEntry& Condition : Row->Conditions)
		{
			if (!Condition.TowerClass)
			{
				bAllConditionsMet = false;
				break;
			}

			ABaseTower* MatchedTower = nullptr;
			for (ABaseTower* Tower : FieldTowers)
			{
				if (!IsValid(Tower) || MatchedTowers.Contains(Tower)) continue;

				UTowerStatComponent* Stat = Tower->GetComponentByClass<UTowerStatComponent>();
				if (!IsValid(Stat)) continue;

				if (Tower->IsA(Condition.TowerClass) && Stat->CurrentLevel >= Condition.RequiredLevel)
				{
					MatchedTower = Tower;
					break;
				}
			}

			if (!IsValid(MatchedTower))
			{
				bAllConditionsMet = false;
				break;
			}
			MatchedTowers.Add(MatchedTower);
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

				AGachaManager* GachaManager = Cast<AGachaManager>(
					UGameplayStatics::GetActorOfClass(GetWorld(), AGachaManager::StaticClass()));
				if (GachaManager)
				{
					GachaManager->PendingFusionTowers.Add(Row->EpicTowerClass);
				}
			}
			ShowNotifyText(Row->NotifyText);
		}
	}
}

bool AEpicTowerManager::FindFusionMaterials(
	TSubclassOf<AActor> EpicTowerClass,
	const TArray<ABaseTower*>& FieldTowers,
	TArray<ABaseTower*>& OutMaterials) const
{
	OutMaterials.Reset();
	if (!EpicTowerDataTable || !EpicTowerClass) return false;

	TArray<FName> RowNames = EpicTowerDataTable->GetRowNames();
	for (FName RowName : RowNames)
	{
		FEpicTowerDataRow* Row = EpicTowerDataTable->FindRow<FEpicTowerDataRow>(RowName, TEXT(""));
		if (!Row || Row->EpicTowerClass != EpicTowerClass) continue;

		TSet<ABaseTower*> MatchedTowers;
		for (const FEpicTowerConditionEntry& Condition : Row->Conditions)
		{
			if (!Condition.TowerClass)
			{
				OutMaterials.Reset();
				return false;
			}

			ABaseTower* MatchedTower = nullptr;
			for (ABaseTower* Tower : FieldTowers)
			{
				if (!IsValid(Tower) || MatchedTowers.Contains(Tower)) continue;

				UTowerStatComponent* Stat = Tower->GetComponentByClass<UTowerStatComponent>();
				if (!IsValid(Stat)) continue;

				if (Tower->IsA(Condition.TowerClass) && Stat->CurrentLevel >= Condition.RequiredLevel)
				{
					MatchedTower = Tower;
					break;
				}
			}

			if (!IsValid(MatchedTower))
			{
				OutMaterials.Reset();
				return false;
			}

			MatchedTowers.Add(MatchedTower);
			OutMaterials.Add(MatchedTower);
		}
		return true;
	}
	return false;
}

void AEpicTowerManager::ConsumeFusionMaterials(
	TSubclassOf<AActor> EpicTowerClass,
	const TArray<ABaseTower*>& Materials,
	AGachaManager* GachaManager,
	ASynergyManager* SynergyManager)
{
	for (ABaseTower* Tower : Materials)
	{
		if (!IsValid(Tower)) continue;

		if (SynergyManager)
		{
			SynergyManager->UnregisterTower(Tower);
		}

		ATowerSlot* OwnerSlot = Tower->OwnerSlot;
		if (IsValid(OwnerSlot) && OwnerSlot->InstalledTower == Tower)
		{
			OwnerSlot->InstalledTower = nullptr;
			OwnerSlot->bIsOccupied = false;
			OwnerSlot->ResetGachaState();
			if (IsValid(OwnerSlot->TowerSlotMesh)) OwnerSlot->TowerSlotMesh->SetVisibility(true);
		}

		Tower->Destroy();
	}

	PendingFusionTowers.Remove(EpicTowerClass);
	if (IsValid(GachaManager))
	{
		GachaManager->PendingFusionTowers.Remove(EpicTowerClass);
	}
}

bool AEpicTowerManager::FuseEpicTower(
	TSubclassOf<AActor> EpicTowerClass,
	const TArray<ABaseTower*>& FieldTowers,
	AGachaManager* GachaManager,
	ASynergyManager* SynergyManager)
{
	// 재료를 모두 찾은 경우에만 슬롯과 타워를 소비.
	TArray<ABaseTower*> Materials;
	if (!FindFusionMaterials(EpicTowerClass, FieldTowers, Materials)) return false;

	ConsumeFusionMaterials(EpicTowerClass, Materials, GachaManager, SynergyManager);
	return true;
}

void AEpicTowerManager::CancelPendingFusion(TSubclassOf<AActor> EpicTowerClass, AGachaManager* GachaManager)
{
	PendingFusionTowers.Remove(EpicTowerClass);
	if (IsValid(GachaManager))
	{
		GachaManager->PendingFusionTowers.Remove(EpicTowerClass);
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