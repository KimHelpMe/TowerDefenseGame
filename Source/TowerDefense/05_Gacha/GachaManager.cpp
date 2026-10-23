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
	if (!IsValid(Slot) || !GachaWidgetClass) return;

	GachaWidget = CreateWidget<UGachaWidget>(GetWorld(), GachaWidgetClass);
	if (!GachaWidget) return;

	CurrentSlot = Slot;
	bIsGachaOpen = true;
	GachaWidget->AddToViewport();

	if (Slot->SavedRollResult.Num() > 0)
	{
		CurrentRollResult = Slot->SavedRollResult;
		const TArray<FGachaCardInfo> SavedCardInfos = BuildCardInfos(CurrentRollResult);
		if (SavedCardInfos.Num() == 3)
		{
			GachaWidget->UpdateGachaUI(SavedCardInfos);
		}
		else
		{
			Roll();
		}
	}
	else
	{
		Roll();
	}

	if (IsValid(CurrentSlot))
	{
		GachaWidget->UpdateReloadUI(CurrentSlot->ReloadCount, CurrentSlot->bCanAbandon);
	}
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
	if (!IsValid(CurrentSlot)) return;

	CurrentRollResult.Empty();
	const auto IsSupportedItem = [](const TSubclassOf<AActor>& Item)
	{
		if (!Item) return false;
		const AActor* ItemCDO = Item->GetDefaultObject<AActor>();
		return ItemCDO && (ItemCDO->IsA(ABaseTower::StaticClass())
			|| ItemCDO->IsA(AUpgradeItem::StaticClass()));
	};
	if (PendingFusionTowers.Num() > 0 && IsSupportedItem(PendingFusionTowers[0]))
	{
		CurrentRollResult.Add(PendingFusionTowers[0]);
	}

	TArray<TArray<TSubclassOf<AActor>>*> Pools = {
		&CommonPool, &RarePool, &UniquePool, &EpicPool, &UpgradePool
	};
	const TArray<float> PoolRates = {
		CommonPoolRate, RarePoolRate, UniquePoolRate, EpicPoolRate, UpgradePoolRate
	};

	TArray<TSubclassOf<AActor>> AllValidItems;
	for (TArray<TSubclassOf<AActor>>* Pool : Pools)
	{
		if (!Pool) continue;
		for (const TSubclassOf<AActor>& Item : *Pool)
		{
			if (IsSupportedItem(Item)) AllValidItems.AddUnique(Item);
		}
	}
	if (!CurrentRollResult.IsEmpty())
	{
		AllValidItems.AddUnique(CurrentRollResult[0]);
	}

	TArray<float> EffectiveRates;
	for (float Rate : PoolRates)
	{
		EffectiveRates.Add(FMath::Max(0.f, Rate));
	}

	while (CurrentRollResult.Num() < 3 && !AllValidItems.IsEmpty())
	{
		TArray<int32> AvailablePoolIndices;
		for (int32 PoolIndex = 0; PoolIndex < Pools.Num(); ++PoolIndex)
		{
			if (!Pools[PoolIndex]) continue;
			for (const TSubclassOf<AActor>& Item : *Pools[PoolIndex])
			{
				if (IsSupportedItem(Item) && !CurrentRollResult.Contains(Item))
				{
					AvailablePoolIndices.AddUnique(PoolIndex);
					break;
				}
			}
		}

		TArray<TSubclassOf<AActor>> AvailableItems;
		if (!AvailablePoolIndices.IsEmpty())
		{
			float AvailableRate = 0.f;
			for (int32 PoolIndex : AvailablePoolIndices)
			{
				AvailableRate += EffectiveRates[PoolIndex];
			}
			const bool bUseEqualWeights = AvailableRate <= KINDA_SMALL_NUMBER;
			if (bUseEqualWeights)
			{
				AvailableRate = static_cast<float>(AvailablePoolIndices.Num());
			}

			float RollValue = FMath::FRandRange(0.f, AvailableRate);
			int32 SelectedPoolIndex = AvailablePoolIndices.Last();
			for (int32 PoolIndex : AvailablePoolIndices)
			{
				const float Weight = bUseEqualWeights ? 1.f : EffectiveRates[PoolIndex];
				RollValue -= Weight;
				if (RollValue <= 0.f)
				{
					SelectedPoolIndex = PoolIndex;
					break;
				}
			}

			for (const TSubclassOf<AActor>& Item : *Pools[SelectedPoolIndex])
			{
				if (IsSupportedItem(Item) && !CurrentRollResult.Contains(Item))
				{
					AvailableItems.AddUnique(Item);
				}
			}
		}

		if (AvailableItems.IsEmpty())
		{
			for (TArray<TSubclassOf<AActor>>* Pool : Pools)
			{
				if (!Pool) continue;
				for (const TSubclassOf<AActor>& Item : *Pool)
				{
					if (IsSupportedItem(Item) && !CurrentRollResult.Contains(Item))
					{
						AvailableItems.AddUnique(Item);
					}
				}
			}
		}

		// 풀이 세 종류 미만이면 유효한 카드를 반복해 빈 칸을 만들지 않는다.
		if (AvailableItems.IsEmpty())
		{
			AvailableItems = AllValidItems;
		}
		if (AvailableItems.IsEmpty()) break;

		CurrentRollResult.Add(AvailableItems[FMath::RandRange(0, AvailableItems.Num() - 1)]);
	}

	for (const TSubclassOf<AActor>& Item : CurrentRollResult)
	{
		UE_LOG(LogTemp, Warning, TEXT("RollResult: %s"), Item ? *Item->GetName() : TEXT("NULL"));
	}

	if (GachaWidget)
	{
		GachaWidget->UpdateGachaUI(BuildCardInfos(CurrentRollResult));
	}
	CurrentSlot->SavedRollResult = CurrentRollResult;
}

void AGachaManager::SelectItem(int32 Index)
{
	if (!CurrentRollResult.IsValidIndex(Index) || !CurrentRollResult[Index]) return;
	if (!IsValid(CurrentSlot)) return;

	const FVector CurrentSlotLocation = CurrentSlot->GetActorLocation();
	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(
		CurrentRollResult[Index], CurrentSlotLocation, FRotator::ZeroRotator);
	if (!SpawnedActor) return;

	if (IGachable* GachableItem = Cast<IGachable>(SpawnedActor))
	{
		GachableItem->OnSelected(CurrentSlot);
	}
	if (!IsValid(CurrentSlot))
	{
		CloseGacha();
		return;
	}

	ABaseTower* Tower = Cast<ABaseTower>(SpawnedActor);
	if (Tower)
	{
		Tower->OwnerSlot = CurrentSlot;
		CurrentSlot->InstalledTower = Tower;
		CurrentSlot->bIsOccupied = true;
		if (CurrentSlot->TowerSlotMesh) CurrentSlot->TowerSlotMesh->SetVisibility(false);

		CommonPool.Remove(CurrentRollResult[Index]);
		RarePool.Remove(CurrentRollResult[Index]);
		UniquePool.Remove(CurrentRollResult[Index]);
		EpicPool.Remove(CurrentRollResult[Index]);

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
		AUpgradeItem* UpgradeItem = Cast<AUpgradeItem>(SpawnedActor);
		if (!UpgradeItem)
		{
			SpawnedActor->Destroy();
			return;
		}

		int32& Count = UpgradeItemCounts.FindOrAdd(CurrentRollResult[Index]);
		Count++;
		if (Count >= UpgradeItem->MaxCount)
		{
			UpgradePool.Remove(CurrentRollResult[Index]);
		}
		SpawnedActor->Destroy();

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
	for (const TSubclassOf<AActor>& ItemClass : Items)
	{
		if (!ItemClass) continue;

		AActor* ItemCDO = ItemClass->GetDefaultObject<AActor>();
		if (!ItemCDO) continue;

		FGachaCardInfo CardInfo;
		CardInfo.Name = ItemClass->GetName();
		CardInfo.Grade = TEXT("Unknown");

		if (ABaseTower* TowerCDO = Cast<ABaseTower>(ItemCDO))
		{
			if (UTowerStatComponent* TowerStat = TowerCDO->TowerStatComponent)
			{
				if (TowerStat->TowerDataTable)
				{
					const FString RowKey = TowerStat->TowerRowName.ToString() + TEXT("_Lv1");
					if (FTowerDataRow* Row = TowerStat->TowerDataTable->FindRow<FTowerDataRow>(
						FName(*RowKey), TEXT("")))
					{
						if (!Row->TowerName.IsEmpty()) CardInfo.Name = Row->TowerName;
						if (!Row->Grade.IsEmpty()) CardInfo.Grade = Row->Grade;
						CardInfo.Icon = Row->Icon;
						CardInfo.SynergyTags = Row->SynergyTags;
					}
				}
			}
			CardInfo.bIsUpgrade = false;
		}
		else if (AUpgradeItem* UpgradeCDO = Cast<AUpgradeItem>(ItemCDO))
		{
			if (!UpgradeCDO->UpgradeName.IsEmpty()) CardInfo.Name = UpgradeCDO->UpgradeName;
			if (!UpgradeCDO->Grade.IsEmpty()) CardInfo.Grade = UpgradeCDO->Grade;
			CardInfo.Icon = UpgradeCDO->UpgradeIcon;
			CardInfo.Description = UpgradeCDO->UpgradeInfo;
			CardInfo.bIsUpgrade = true;
		}
		else
		{
			continue;
		}

		CardInfos.Add(CardInfo);
	}
	return CardInfos;
}
