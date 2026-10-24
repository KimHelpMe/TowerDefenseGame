#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ManaComponent.h"
#include "BaseTower.h"
#include "TowerInfoWidget.generated.h"

class UTowerStatComponent;

UCLASS()
class TOWERDEFENSE_API UTowerInfoWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;

public:

	UPROPERTY(BlueprintReadWrite)
	UTowerStatComponent* TowerStat;

	UPROPERTY(BlueprintReadWrite)
	UManaComponent* TowerMana;

public:

	UFUNCTION(BlueprintCallable)
	void LevelUp();

	UFUNCTION(BlueprintCallable)
	void CloseWidget();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void InitWidget(UTowerStatComponent* Stat);

	UPROPERTY(BlueprintReadWrite)
	ABaseTower* OwnerTower;

};
