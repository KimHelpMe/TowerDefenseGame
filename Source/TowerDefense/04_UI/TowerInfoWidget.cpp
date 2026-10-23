
#include "TowerInfoWidget.h"
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
