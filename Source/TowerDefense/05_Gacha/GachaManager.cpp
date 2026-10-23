#include "GachaManager.h"
#include "BaseTower.h"
#include "TowerSlot.h"
#include "Gachable.h"
#include "Kismet/GameplayStatics.h"
#include "UpgradeItem.h"
#include "TowerDefenseGameMode.h"
#include "SynergyManager.h"
#include "TowerStatComponent.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "EpicTowerManager.h"

AGachaManager::AGachaManager()
{
	PrimaryActorTick.bCanEverTick = true;

	// 확률 생성자
	CommonPoolRate = 40.f;
	RarePoolRate = 20.f;
	UniquePoolRate = 5.f;
	UpgradePoolRate = 35.f;
	TotalPoolRate = 100.f;


	CurrentSlot = nullptr;

	bIsGachaOpen = false;
}


void AGachaManager::BeginPlay()
{
	Super::BeginPlay();
	
}


void AGachaManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}



void AGachaManager::OpenGacha(ATowerSlot* Slot)
{
	bIsGachaOpen = true;

	if (!GachaWidgetClass) return;

	GachaWidget = CreateWidget<UGachaWidget>(GetWorld(), GachaWidgetClass);
	GachaWidget->AddToViewport();

	CurrentSlot = Slot;
	
	// 저장된 결과가 있으면 Roll 안함.
	if (Slot->SavedRollResult.Num() > 0)
	{
		CurrentRollResult = Slot->SavedRollResult;
		GachaWidget->UpdateGachaUI(BuildCardInfos(CurrentRollResult));
	}
	else
	{
		Roll();
	}

	GachaWidget->UpdateReloadUI(CurrentSlot->ReloadCount, CurrentSlot->bCanAbandon);
}

void AGachaManager::CloseGacha()
{
	bIsGachaOpen = false;

	if (GachaWidget)
	{
		GachaWidget->RemoveFromParent();
	}

}



void AGachaManager::Roll()
{
	CurrentRollResult.Empty();

	// PendingFusionTowers가 있으면 Index 0에 강제 배치.
	if (PendingFusionTowers.Num() > 0)
	{
		CurrentRollResult.Add(PendingFusionTowers[0]);
	}


	// 나머지 2장 뽑기 (i는 이미 있는 개수부터 시작)
	for (int32 i = CurrentRollResult.Num(); i < 3; i++)
	{
		TArray<TSubclassOf<AActor>>* SelectedPool;
		float RandValue = FMath::RandRange(0.f, TotalPoolRate);

		if (RandValue < CommonPoolRate)
			SelectedPool = &CommonPool;
		else if (RandValue < CommonPoolRate + RarePoolRate)
				SelectedPool = &RarePool;
		else if (RandValue < CommonPoolRate + RarePoolRate + UniquePoolRate)
				SelectedPool = &UniquePool;
		else if (RandValue < CommonPoolRate + RarePoolRate + UniquePoolRate + EpicPoolRate)
				SelectedPool = &EpicPool;
		else
				SelectedPool = &UpgradePool;


		// 선택된 풀이 비어있으면 다른 풀로 대체
		if (SelectedPool->Num() == 0) SelectedPool = &CommonPool;
		if (SelectedPool->Num() == 0) SelectedPool = &RarePool;
		if (SelectedPool->Num() == 0) SelectedPool = &UniquePool;
		if (SelectedPool->Num() == 0) SelectedPool = &UpgradePool;

		// 모든 풀이 비어있으면 건너뜀
		if (SelectedPool->Num() == 0) continue;

		// 선택된 풀에서 이미 뽑힌 것 제외.
		TArray<TSubclassOf<AActor>> AvailablePool = SelectedPool->FilterByPredicate(
			[this](const TSubclassOf<AActor>& Item)
			{
				return !CurrentRollResult.Contains(Item);
			});

		// 없으면 모든 풀에서 합치기.
		if (AvailablePool.Num() == 0)
		{
			for (auto* Pool : TArray<TArray<TSubclassOf<AActor>>*>
				{ &CommonPool, &RarePool, &UniquePool, &EpicPool, &UpgradePool })
			{
				for (auto& Item : *Pool)
				{
					if (!CurrentRollResult.Contains(Item))
					{
						AvailablePool.Add(Item);
					}
				}
			}
		}

			// 그래도 없으면 건너뜀.
			if (AvailablePool.Num() == 0) continue;

			int32 RandIndex = FMath::RandRange(0, AvailablePool.Num() - 1);
			CurrentRollResult.Add(AvailablePool[RandIndex]);
	
	}

	for (auto& Item : CurrentRollResult)
	{
		UE_LOG(LogTemp, Warning, TEXT("RollResult: %s"),
			Item ? *Item->GetName() : TEXT("NULL"));
	}

	if (GachaWidget)
	{
		GachaWidget->UpdateGachaUI(BuildCardInfos(CurrentRollResult));
	}

	CurrentSlot->SavedRollResult = CurrentRollResult;
}


void AGachaManager::SelectItem(int32 Index)
{
	if (!CurrentRollResult[Index]) return;
	if (!CurrentSlot) return;

	FVector CurrentSlotLocation = CurrentSlot->GetActorLocation();
	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(CurrentRollResult[Index], CurrentSlotLocation, FRotator::ZeroRotator);

	if (SpawnedActor)
	{
		IGachable* GachableItem = Cast<IGachable>(SpawnedActor);
		if (GachableItem)
		{
			GachableItem->OnSelected(CurrentSlot);
		}

		// 타워 -> 슬롯 점유함.
		// 업그레이드 -> 슬롯 반환함.
		ABaseTower* Tower = Cast<ABaseTower>(SpawnedActor);
		if (Tower)
		{
			Tower->OwnerSlot = CurrentSlot;
			CurrentSlot->InstalledTower = Tower;
			CurrentSlot->bIsOccupied = true;
			CurrentSlot->TowerSlotMesh->SetVisibility(false);

			// 풀에서 제거
			CommonPool.Remove(CurrentRollResult[Index]);
			RarePool.Remove(CurrentRollResult[Index]);
			UniquePool.Remove(CurrentRollResult[Index]);

			// 합성형 에픽 타워인지 확인
			if (PendingFusionTowers.Contains(CurrentRollResult[Index]))
			{
				AEpicTowerManager* EpicManager = Cast<AEpicTowerManager>(
					UGameplayStatics::GetActorOfClass(GetWorld(), AEpicTowerManager::StaticClass()));
				ASynergyManager* SynergyManager = Cast<ASynergyManager>(
					UGameplayStatics::GetActorOfClass(GetWorld(), ASynergyManager::StaticClass()));
				if (EpicManager)
				{
					EpicManager->FuseEpicTower(
						CurrentRollResult[Index],
						SynergyManager ? SynergyManager->FieldTowers : TArray<ABaseTower*>(),
						this,
						SynergyManager);
				}
			}
		}
		else
		{  
			// 업그레이드 선택 횟수 증가
			int32& Count = UpgradeItemCounts.FindOrAdd(CurrentRollResult[Index]);
			Count++;

			// MaxCount 도달 시 UpgradePool에서 제거
			AUpgradeItem* UpgradeItem = Cast<AUpgradeItem>(SpawnedActor);
			if (UpgradeItem && Count >= UpgradeItem->MaxCount)
			{
				UpgradePool.Remove(CurrentRollResult[Index]);
			}
			
			// 업그레이드 -> 슬롯 반환.
			ATowerDefenseGameMode* GameMode = Cast<ATowerDefenseGameMode>(
				UGameplayStatics::GetGameMode(GetWorld()));
			if (GameMode) GameMode->CurrentTowerSlot--;

			// 업그레이드 -> 슬롯 반환.
			if (CurrentSlot->OwnerTile)
			{
				CurrentSlot->OwnerTile->bIsOccupied = false;
				CurrentSlot->OwnerTile->SetHighlight(false);
			}
			CurrentSlot->Destroy();
			CurrentSlot = nullptr;
		}

	}
	CloseGacha();
}

void AGachaManager::Reload()
{
	if (!CurrentSlot) return;

	if (CurrentSlot->ReloadCount > 0)
	{
		CurrentSlot->ReloadCount -= 1;
		Roll();
	}
	if (CurrentSlot->ReloadCount == 0)
	{
		CurrentSlot->bCanAbandon = true;
	}

	if(GachaWidget)
	{
		GachaWidget->UpdateReloadUI(CurrentSlot->ReloadCount, CurrentSlot->bCanAbandon);
	}


}


void AGachaManager::AbandonGacha()
{
	if (!CurrentSlot) return;
	CloseGacha();

	ATowerDefenseGameMode* GameMode = Cast<ATowerDefenseGameMode>(
		UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode) GameMode->CurrentTowerSlot--;

	if (CurrentSlot->OwnerTile)
	{
		CurrentSlot->OwnerTile->bIsOccupied = false;
		CurrentSlot->OwnerTile->SetHighlight(false);
	}

	CurrentSlot->Destroy();
	CurrentSlot = nullptr;
}


TArray<FGachaCardInfo> AGachaManager::BuildCardInfos(const TArray<TSubclassOf<AActor>>& Items)
{
	TArray<FGachaCardInfo> CardInfos;
	for (auto& ItemClass : Items)
	{
		UE_LOG(LogTemp, Warning, TEXT("BuildCardInfos - ItemClass: %s"),
			ItemClass ? *ItemClass->GetName() : TEXT("NULL"));

		FGachaCardInfo CardInfo;
		ABaseTower* TowerCDO = Cast<ABaseTower>(ItemClass->GetDefaultObject());
		if (TowerCDO)
		{
			UTowerStatComponent* TowerStat = TowerCDO->TowerStatComponent;
			UE_LOG(LogTemp, Warning, TEXT("TowerStat Direct: %s"), TowerStat ? TEXT("Valid") : TEXT("Null"));
			if (TowerStat && TowerStat->TowerDataTable)
			{
				UE_LOG(LogTemp, Warning, TEXT("TowerRowName: %s"), *TowerStat->TowerRowName.ToString());
				FString RowKey = TowerStat->TowerRowName.ToString() + "_Lv1";
				FTowerDataRow* Row = TowerStat->TowerDataTable->FindRow<FTowerDataRow>(
					FName(*RowKey), TEXT(""));
				if (Row)
				{
					CardInfo.Name = Row->TowerName;
					CardInfo.Grade = Row->Grade;
					CardInfo.Icon = Row->Icon;
					CardInfo.SynergyTags = Row->SynergyTags;
					CardInfo.bIsUpgrade = false;
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Trying UpgradeCDO..."));
			AUpgradeItem* UpgradeCDO = Cast<AUpgradeItem>(ItemClass->GetDefaultObject());
			UE_LOG(LogTemp, Warning, TEXT("UpgradeCDO: %s"), UpgradeCDO ? TEXT("Valid") : TEXT("Null"));
			if (UpgradeCDO)
			{
		
				UE_LOG(LogTemp, Warning, TEXT("UpgradeName: %s"), *UpgradeCDO->UpgradeName);

				CardInfo.Name = UpgradeCDO->UpgradeName;
				CardInfo.Grade = UpgradeCDO->Grade;
				CardInfo.Icon = UpgradeCDO->UpgradeIcon;
				CardInfo.Description = UpgradeCDO->UpgradeInfo;
				CardInfo.bIsUpgrade = true;
			}
		}
		CardInfos.Add(CardInfo);
	}
	return CardInfos;
}