

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

        for (FBuffEffect& Stat : Row->Stats)
        {
            SetBaseStat(Stat.StatTag, Stat.Value);
        }

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
            UAirAbilityComponent* AirComp = NewObject<UAirAbilityComponent>(Enemy);
            AirComp->RegisterComponent();
        }

    }
}