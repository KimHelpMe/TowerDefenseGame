#include "GachaWidget.h"
#include "FantasyUITheme.h"


void UGachaWidget::NativeConstruct()
{
	Super::NativeConstruct();
	FFantasyUITheme::ApplyToWidgetTree(this);
}
