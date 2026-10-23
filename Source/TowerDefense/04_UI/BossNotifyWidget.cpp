#include "BossNotifyWidget.h"
#include "FantasyUITheme.h"


void UBossNotifyWidget::NativeConstruct()
{
	Super::NativeConstruct();
	FFantasyUITheme::ApplyToWidgetTree(this);
}
