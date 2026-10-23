#include "TowerStatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TowerDefenseGameMode.h"
#include "SingleAttackComponent.h"
#include "MultiAttackComponent.h"
#include "BaseTower.h"
#include "TowerInfoWidget.h"
#include "EpicTowerManager.h"
#include "SynergyManager.h"
#include "ActiveBuffComponent.h"
#include "PassiveBuffComponent.h"
#include "BuffTypeComponent.h"
#include "TowerSlot.h"

UTowerStatComponent::UTowerStatComponent()
{
	TowerName = TEXT("None");
	TowerInfo = TEXT("None");
	Grade = TEXT("None");
	TowerType = TEXT("None");
	TowerIcon = nullptr;
}

void UTowerStatComponent::BeginPlay()
{
    Super::BeginPlay();
    InitFromDataTable();
}




#if WITH_EDITOR
void UTowerStatComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    ABaseTower* Tower = Cast<ABaseTower>(GetOwner());
    if (Tower && Tower->TowerAttackRangeComponent)
    {
        Tower->TowerAttackRangeComponent->SetSphereRadius(
        GetStat(FGameplayTag::RequestGameplayTag("Stat.AttackRange")));
    }
}
#endif


void UTowerStatComponent::InitFromDataTable()
{
    UE_LOG(LogTemp, Warning, TEXT("InitFromDataTable called"));
    if (!TowerDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("TowerDataTable is NULL"));
        return;
    }


    FString RowKeyString = TowerRowName.ToString() + "_Lv" + FString::FromInt(CurrentLevel);
    FName RowKey = FName(*RowKeyString);

    FTowerDataRow* Row = TowerDataTable->FindRow<FTowerDataRow>(RowKey, TEXT(""));

    if (Row)
    {
      // 기본 정보 저장.
        TowerName = Row->TowerName;
        Grade = Row->Grade;
        TowerType = Row->TowerType;
        MaxLevel = Row->MaxLevel;

        // 태그 적용.
        Elements = Row->Elements;
        AttackTypes = Row->AttackTypes;
        Abilities = Row->Abilities;
        SynergyTags = Row->SynergyTags;
  

        // 스탯 적용.

        for (FBuffEffect& Stat : Row->Stats)
        {
            SetBaseStat(Stat.StatTag, Stat.Value);
        }

        // 버프 적용.
        ApplyBuffFromData(Row->BuffSetting);

        // 외형 적용.
        ABaseTower* Tower = Cast<ABaseTower>(GetOwner());
        if (Tower)
        {
            Tower->TowerMeshComponent->OnClicked.AddUniqueDynamic(Tower, &ABaseTower::OnTowerClicked);
        }

        // 아이콘 적용.
        if (Row->Icon) TowerIcon = Row->Icon;

    }
}


void UTowerStatComponent::LevelUp()
{
	ATowerDefenseGameMode* GameMode = Cast<ATowerDefenseGameMode>(
		UGameplayStatics::GetGameMode(GetWorld()));
	if (!GameMode) return;
	if (CurrentLevel >= MaxLevel || !TowerDataTable) return;

	ABaseTower* OldTower = Cast<ABaseTower>(GetOwner());
	if (!IsValid(OldTower) || !OldTower->NextLevelClass) return;

	ATowerSlot* OldOwnerSlot = OldTower->OwnerSlot;
	if (!IsValid(OldOwnerSlot)) return;

	const FString RowKeyString = TowerRowName.ToString() + TEXT("_Lv") + FString::FromInt(CurrentLevel + 1);
	FTowerDataRow* Row = TowerDataTable->FindRow<FTowerDataRow>(FName(*RowKeyString), TEXT(""));
	if (!Row || GameMode->Gold < Row->GoldCost) return;

	ABaseTower* NewTower = GetWorld()->SpawnActor<ABaseTower>(
		OldTower->NextLevelClass, OldTower->GetActorLocation(), FRotator::ZeroRotator);
	if (!IsValid(NewTower)) return;

	UTowerStatComponent* NewTowerStat = NewTower->TowerStatComponent;
	if (!IsValid(NewTowerStat))
	{
		NewTower->Destroy();
		return;
	}

	NewTowerStat->TowerDataTable = TowerDataTable;
	NewTowerStat->TowerRowName = TowerRowName;
	NewTowerStat->CurrentLevel = CurrentLevel + 1;
	NewTowerStat->InitFromDataTable();
	NewTowerStat->ActiveBuffs = ActiveBuffs;
	NewTowerStat->RecalculateStats();

	NewTower->OwnerSlot = OldOwnerSlot;
	OldOwnerSlot->InstalledTower = NewTower;
	GameMode->Gold -= Row->GoldCost;

	ASynergyManager* SynergyManager = Cast<ASynergyManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ASynergyManager::StaticClass()));
	AEpicTowerManager* EpicManager = Cast<AEpicTowerManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AEpicTowerManager::StaticClass()));
	if (EpicManager && SynergyManager)
	{
		EpicManager->CheckConditions(SynergyManager->FieldTowers);
	}

	if (OldTower->TowerInfoWidget && OldTower->TowerInfoWidget->IsInViewport())
	{
		OldTower->TowerInfoWidget->TowerStat = nullptr;
		OldTower->TowerInfoWidget->CloseWidget();
		NewTower->OnTowerClicked(nullptr, FKey());
	}

	if (SynergyManager)
	{
		SynergyManager->UnregisterTower(OldTower);
	}
	OldTower->Destroy();
}

float UTowerStatComponent::GetNextLevelGoldCost()
{
    if (CurrentLevel >= MaxLevel) return -1.f;
    if (!TowerDataTable) return -1.f;

    FString RowKeyString = TowerRowName.ToString() + "_Lv" + FString::FromInt(CurrentLevel + 1);
    FName RowKey = FName(*RowKeyString);

    FTowerDataRow* Row = TowerDataTable->FindRow<FTowerDataRow>(RowKey, TEXT(""));
    if (!Row) return -1.f;

    return Row->GoldCost;
}



void UTowerStatComponent::ApplyBuffFromData(const FBuffConfig& BuffSetting)
{
  
    // 버프 적용.
    switch (BuffSetting.BuffType)
    {
    case EBuffComponentType::None:
        break;

    case EBuffComponentType::Passive:
    {
        UPassiveBuffComponent* PassiveBuffComp = GetOwner()->GetComponentByClass<UPassiveBuffComponent>();
        if (PassiveBuffComp)
        {
            PassiveBuffComp->BuffTarget = BuffSetting.BuffTarget;
            PassiveBuffComp->BuffEffects = BuffSetting.BuffEffects;
            PassiveBuffComp->BuffInterval = BuffSetting.BuffInterval;
            PassiveBuffComp->BuffRadius = BuffSetting.BuffRadius;
        }
    }
    break;

    case EBuffComponentType::Active:
    {
        UActiveBuffComponent* ActiveBuffComp = GetOwner()->GetComponentByClass<UActiveBuffComponent>();
        if (ActiveBuffComp)
        {
            ActiveBuffComp->BuffTarget = BuffSetting.BuffTarget;
            ActiveBuffComp->BuffEffects = BuffSetting.BuffEffects;
            ActiveBuffComp->Duration = BuffSetting.BuffDuration;
            ActiveBuffComp->Cooldown = BuffSetting.BuffCooldown;
            ActiveBuffComp->BuffRadius = BuffSetting.BuffRadius;
            ActiveBuffComp->ActivateBuff();
        }
    }
    break;
    }
}