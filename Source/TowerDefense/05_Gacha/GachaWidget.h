#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GachaCardInfo.h"
#include "GachaWidget.generated.h"


UCLASS()
class TOWERDEFENSE_API UGachaWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateReloadUI(int32 ReloadCount, bool bCanAbandon);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateGachaUI(const TArray<FGachaCardInfo>& CardInfos);

};
