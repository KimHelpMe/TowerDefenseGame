
#include "TowerInfoWidget.h"
#include "FantasyUITheme.h"
#include "TowerStatComponent.h"

void UTowerInfoWidget::LevelUp()
{
	if (!TowerStat) return;
    TowerStat->LevelUp();
}

void UTowerInfoWidget::CloseWidget()
{
	RemoveFromParent();
}

void UTowerInfoWidget::InitWidget_Implementation(UTowerStatComponent* Stat)
{
	if (!Stat) return;
	TowerStat = Stat;
	TowerMana = TowerStat->GetOwner()->GetComponentByClass<UManaComponent>();
	OwnerTower = Cast<ABaseTower>(TowerStat->GetOwner());
}

void UTowerInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();
	FFantasyUITheme::ApplyToWidgetTree(this);
}
