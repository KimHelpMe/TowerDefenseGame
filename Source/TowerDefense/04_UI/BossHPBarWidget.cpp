
#include "BossHPBarWidget.h"
#include "FantasyUITheme.h"


void UBossHPBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
	FFantasyUITheme::ApplyToWidgetTree(this);
}
