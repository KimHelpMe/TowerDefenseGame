

#include "EpicNotifyWidget.h"
#include "FantasyUITheme.h"


void UEpicNotifyWidget::NativeConstruct()
{
	Super::NativeConstruct();
	FFantasyUITheme::ApplyToWidgetTree(this);
}
