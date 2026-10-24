#include "TowerStatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TowerDefenseGameMode.h"
#include "SingleAttackComponent.h"
#include "MultiAttackComponent.h"
#include "BaseTower.h"
#include "AttackTower.h"
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

void UTowerStatComponent::RecalculateStats()
{
	Super::RecalculateStats();

	ABaseTower* Tower = Cast<ABaseTower>(GetOwner());
	if (!IsValid(Tower) || !IsValid(Tower->TowerAttackRangeComponent)) return;

	const float BaseRange = GetStat(UStatComponent::GetTag("Stat.AttackRange"));
	ATowerDefenseGameMode* GameMode = GetWorld()
		? Cast<ATowerDefenseGameMode>(UGameplayStatics::GetGameMode(GetWorld()))
		: nullptr;
	const float AttackRangeBonus = GameMode
		? GameMode->GetUpgradeBonus(UStatComponent::GetTag("Upgrade.AttackRange"))
		: 0.f;
	const float FinalAttackRange = FMath::Max(0.f, BaseRange * (1.f + AttackRangeBonus));
	Tower->TowerAttackRangeComponent->SetSphereRadius(FinalAttackRange);

	// 버프·시너지로 바뀐 공격속도를 공격 타이머에 반영한다.
	// 버프는 제거 후 재적용되므로 같은 프레임의 변경을 모아 다음 틱에 한 번 확인한다.
	AAttackTower* AttackTower = Cast<AAttackTower>(Tower);
	if (AttackTower && AttackTower->HasActorBegunPlay() && GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(AttackTower, &AAttackTower::RefreshAttackTimer);
	}
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
	// 이전 타워의 ActiveBuffs는 복사하지 않는다. 다른 타워의 Active 버프가 복사되면
	// 버프를 준 쪽의 PreviousTargets에 새 타워가 없어 지속시간이 끝나도 지워지지 않는다.
	// 시너지는 아래 UnregisterTower(OldTower)에서 다시 계산되고, Passive 버프는 다음 주기에 다시 적용된다.

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
