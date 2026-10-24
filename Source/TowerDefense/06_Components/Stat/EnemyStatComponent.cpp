

#include "EnemyStatComponent.h"
#include "EnemyDataRow.h"
#include "AirAbilityComponent.h"
#include "Enemy.h"


UEnemyStatComponent::UEnemyStatComponent()
{
    EnemyGrade = TEXT("None");

    // 기본 스탯 설정
    BaseStats.Add(FGameplayTag::RequestGameplayTag("Stat.HP"), 100.f);
    BaseStats.Add(FGameplayTag::RequestGameplayTag("Stat.MaxHP"), 100.f);
    BaseStats.Add(FGameplayTag::RequestGameplayTag("Stat.MoveSpeed"), 100.f);
    BaseStats.Add(FGameplayTag::RequestGameplayTag("Stat.AttackDamage"), 10.f);
    BaseStats.Add(FGameplayTag::RequestGameplayTag("Stat.AttackSpeed"), 1.f);
    BaseStats.Add(FGameplayTag::RequestGameplayTag("Stat.Defense"), 0.f);
}

void UEnemyStatComponent::BeginPlay()
{
    Super::BeginPlay();
    InitFromDataTable();
}

void UEnemyStatComponent::InitFromDataTable()
{
    if (!EnemyDataTable) return;

    FString RowKeyString = EnemyRowName.ToString();
    FName RowKey = FName(*RowKeyString);

    FEnemyDataRow* Row = EnemyDataTable->FindRow<FEnemyDataRow>(RowKey, TEXT(""));

    if (Row)
    {
        EnemyGrade = Row->EnemyGrade;
        DefenseTypes = Row->DefenseTypes;
        ElementResistRate = Row->ElementResistRate;

        // 태그 적용.
        Elements = Row->Elements;

        // 스탯 적용.
        // SetBaseStat은 매번 HP를 MaxHP로 클램프하므로, 행 순서(HP가 MaxHP보다 먼저)에 따라
        // HP가 기본 MaxHP(100)로 잘리지 않도록 모두 넣은 뒤 한 번만 재계산한다.
        const FGameplayTag HPTag = GetTag("Stat.HP");
        const FGameplayTag MaxHPTag = GetTag("Stat.MaxHP");
        bool bRowHasHP = false;
        bool bRowHasMaxHP = false;
        for (const FBuffEffect& Stat : Row->Stats)
        {
            BaseStats.Add(Stat.StatTag, Stat.Value);
            bRowHasHP |= Stat.StatTag == HPTag;
            bRowHasMaxHP |= Stat.StatTag == MaxHPTag;
        }

        // MaxHP만 지정된 행은 최대 체력으로 시작.
        if (bRowHasMaxHP && !bRowHasHP)
        {
            BaseStats.Add(HPTag, GetBaseStat(MaxHPTag));
        }
        RecalculateStats();

        // 외형 적용.
        AEnemy* Enemy = Cast<AEnemy>(GetOwner());
        if (Enemy)
        {
            if (Row->EnemyMesh) Enemy->EnemyMeshComponent->SetSkeletalMeshAsset(Row->EnemyMesh);
            if (Row->EnemyMaterial) Enemy->EnemyMeshComponent->SetMaterial(0, Row->EnemyMaterial);
            Enemy->GoldReward = Row->GoldReward;
            Enemy->ExpReward = Row->ExpReward;
        }

        // 공중 여부 적용.
        if (Row->bIsAirUnit)
        {
            // BP에 이미 공중 컴포넌트가 있으면 중복 생성하지 않는다.
            if (Enemy && !Enemy->GetComponentByClass<UAirAbilityComponent>())
            {
                UAirAbilityComponent* AirComp = NewObject<UAirAbilityComponent>(Enemy, TEXT("AirAbility"));
                // 인스턴스 컴포넌트로 등록해야 디테일 패널·직렬화·GetComponents에서 일관되게 다뤄진다.
                Enemy->AddInstanceComponent(AirComp);
                AirComp->RegisterComponent();
            }
        }

    }
}