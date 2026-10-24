

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
    if (TowerStatComponent && TowerStatComponent->TowerDataTable)
    {
        const FName Key(*FString::Printf(TEXT("%s_Lv%d"), *TowerStatComponent->TowerRowName.ToString(), TowerStatComponent->CurrentLevel));
        if (const FTowerDataRow* Row = TowerStatComponent->TowerDataTable->FindRow<FTowerDataRow>(Key, TEXT("Buff range")))
        {
            // Blueprints can contain both buff types; use the configured component.
            if (Row->BuffSetting.BuffType == EBuffComponentType::Passive)
                BuffComponent = GetComponentByClass<UPassiveBuffComponent>();
            else if (Row->BuffSetting.BuffType == EBuffComponentType::Active)
                BuffComponent = GetComponentByClass<UActiveBuffComponent>();
        }
    }
	if (IsValid(BuffComponent) && BuffComponent->BuffRadius > 0.f)
	{
		return BuffComponent->BuffRadius;
	}
	return IsValid(BuffRangeComponent) ? BuffRangeComponent->GetScaledSphereRadius() : 0.f;
}
