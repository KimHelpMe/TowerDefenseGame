#include "UpgradeItem.h"
#include "TowerDefenseLog.h"
#include "Kismet/GameplayStatics.h"
#include "TowerDefenseGameMode.h"

AUpgradeItem::AUpgradeItem()
{
	PrimaryActorTick.bCanEverTick = false; // Tick does nothing; BP Event Tick re-enables it on compile.

	UpgradeName = TEXT("None");
	UpgradeInfo = TEXT("None");
	Grade = TEXT("None");
	UpgradeIcon = nullptr;
	MaxCount = 10;
}

void AUpgradeItem::BeginPlay()
{
	Super::BeginPlay();
	
}


void AUpgradeItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AUpgradeItem::ApplyUpgradeEffects()
{
	ATowerDefenseGameMode* GameMode = Cast<ATowerDefenseGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!GameMode) return;

	for (FBuffEffect& Effect : UpgradeEffects)
	{
		UE_LOG(LogTowerDefense, Log, TEXT("Applying Upgrade: %s, Value: %f"),
			*Effect.StatTag.ToString(), Effect.Value);


		GameMode->ApplyUpgrade(Effect);
	}

}


// Gacha


FString AUpgradeItem::GetItemName()
{
	return FString(UpgradeName);
}

FString AUpgradeItem::GetItemGrade()
{
	return FString(Grade);
}

UTexture2D* AUpgradeItem::GetItemIcon()
{
	return nullptr;
}

void AUpgradeItem::OnSelected(ATowerSlot* Slot)
{
	ApplyUpgradeEffects();
}