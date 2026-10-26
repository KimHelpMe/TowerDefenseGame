

#include "BuffTower.h"
#include "BuffTypeComponent.h"
#include "TowerStatComponent.h"
#include "PassiveBuffComponent.h"
#include "ActiveBuffComponent.h"

ABuffTower::ABuffTower()
{
	BuffRangeComponent = CreateDefaultSubobject<USphereComponent>(TEXT("BuffRangeComponent"));
	BuffRangeComponent->SetupAttachment(RootComponent);
	BuffRange = 300.f;
	BuffRangeComponent->SetSphereRadius(BuffRange);


}

void ABuffTower::BeginPlay()
{
	Super::BeginPlay();


	BuffComponent = GetComponentByClass<UBuffTypeComponent>();
}

float ABuffTower::GetRangeRadius()
{
    // 사거리 데칼이 20Hz로 호출하므로 데이터 행 조회는 레벨/테이블이 바뀔 때만 다시 한다.
    const bool bCacheValid = TowerStatComponent
        && CachedBuffComponentLevel == TowerStatComponent->CurrentLevel
        && CachedBuffComponentTable == TowerStatComponent->TowerDataTable;
    if (TowerStatComponent && TowerStatComponent->TowerDataTable && !bCacheValid)
    {
        CachedBuffComponentLevel = TowerStatComponent->CurrentLevel;
        CachedBuffComponentTable = TowerStatComponent->TowerDataTable;
        const FName Key(*FString::Printf(TEXT("%s_Lv%d"), *TowerStatComponent->TowerRowName.ToString(), TowerStatComponent->CurrentLevel));
        if (const FTowerDataRow* Row = TowerStatComponent->TowerDataTable->FindRow<FTowerDataRow>(Key, TEXT("Buff range")))
        {
            // Blueprints can contain both buff types; use the configured component.
            if (Row->BuffSetting.BuffType == EBuffComponentType::Passive)
                BuffComponent = GetComponentByClass<UPassiveBuffComponent>();
            else if (Row->BuffSetting.BuffType == EBuffComponentType::Active)
                BuffComponent = GetComponentByClass<UActiveBuffComponent>();

            // 실제 버프가 적용되는 반경은 데이터 테이블의 BuffRadius이므로, BP의 BuffRange 기본값(300) 대신 이 값을 표시에 쓴다.
            if (Row->BuffSetting.BuffRadius > 0.f) BuffRange = Row->BuffSetting.BuffRadius;
        }
    }
	if (BuffRange > 0.f) return BuffRange;
	if (IsValid(BuffComponent) && BuffComponent->BuffRadius > 0.f) return BuffComponent->BuffRadius;
	return IsValid(BuffRangeComponent) ? BuffRangeComponent->GetScaledSphereRadius() : 0.f;
}
