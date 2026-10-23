#pragma once

#include "CoreMinimal.h"

class UUserWidget;
class UWorld;

class TOWERDEFENSE_API FFantasyUITheme
{
public:
	static void ApplyToWidgetTree(UUserWidget* RootWidget);
	static void ApplyToViewport(UWorld* World);
};
