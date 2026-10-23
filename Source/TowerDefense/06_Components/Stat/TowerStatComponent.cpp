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
            Tower->TowerMeshComponent->OnClicked.AddDynamic(Tower, &ABaseTower::OnTowerClicked);
        }

        // 아이콘 적용.
        if (Row->Icon) TowerIcon = Row->Icon;

    }
}


void UTowerStatComponent::LevelUp()
{
    // 1. 유효성 체크
    ATowerDefenseGameMode* GameMode = Cast<ATowerDefenseGameMode>(
        UGameplayStatics::GetGameMode(GetWorld()));
        if (!GameMode) return;
    if (CurrentLevel >= MaxLevel) return;
    if (!TowerDataTable) return;

    // 2. OldTower / NextLevelClass 체크 (Gold 차감 전)
    ABaseTower* OldTower = Cast<ABaseTower>(GetOwner());
    if (!OldTower) return;
    if (!OldTower->NextLevelClass) return;

    // 3. Row 유효 체크
    FString RowKeyString = TowerRowName.ToString() + "_Lv" + FString::FromInt(CurrentLevel + 1);
    FTowerDataRow* Row = TowerDataTable->FindRow<FTowerDataRow>(FName(*RowKeyString), TEXT(""));
    if (!Row) return;

    // 4. Gold 체크 및 차감
    if (GameMode->Gold < Row->GoldCost) return;
    GameMode->Gold -= Row->GoldCost;

    // 5. NewTower 스폰
    ABaseTower* NewTower = GetWorld()->SpawnActor<ABaseTower>(
        OldTower->NextLevelClass, OldTower->GetActorLocation(), FRotator::ZeroRotator);
    if (!NewTower) return;

    // 6. CurrentLevel 이전.
    NewTower->TowerStatComponent->CurrentLevel = CurrentLevel + 1;

    // 7. ActiveBuffs 이전 후 RecalculateStats()
    NewTower->TowerStatComponent->ActiveBuffs = ActiveBuffs;
    NewTower->TowerStatComponent->RecalculateStats();

    // 8. OwnerSlot 이전 / 슬롯 갱신`
    NewTower->OwnerSlot = OldTower->OwnerSlot;
    OldTower->OwnerSlot->InstalledTower = NewTower;

    // 9. EpicManager 체크
    ASynergyManager* SynergyManager = Cast<ASynergyManager>(
        UGameplayStatics::GetActorOfClass(GetWorld(), ASynergyManager::StaticClass()));
    AEpicTowerManager* EpicManager = Cast<AEpicTowerManager>(
        UGameplayStatics::GetActorOfClass(GetWorld(), AEpicTowerManager::StaticClass()));
    if (EpicManager && SynergyManager)
        EpicManager->CheckConditions(SynergyManager->FieldTowers);

    // 10. TowerInfoWidget 닫기
    if (OldTower->TowerInfoWidget && OldTower->TowerInfoWidget->IsInViewport())
    {
        OldTower->TowerInfoWidget->TowerStat = nullptr;
        OldTower->TowerInfoWidget->CloseWidget();

        // 새 타워 위젯 열기
        NewTower->OnTowerClicked(nullptr, FKey());
    }

    // 11. Unregister 후 Destroy
    if (SynergyManager)
        SynergyManager->UnregisterTower(OldTower);
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
        }
    }
    break;
    }
}