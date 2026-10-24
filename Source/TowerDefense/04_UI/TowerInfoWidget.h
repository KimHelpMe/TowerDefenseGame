#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ManaComponent.h"
#include "BaseTower.h"
#include "TowerInfoWidget.generated.h"

class UTowerStatComponent;
class UVerticalBox;

UCLASS()
class TOWERDEFENSE_API UTowerInfoWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;

	void StyleStatCards();
	void EnsureDetailsPanel();
	void BuildDetailsPanel(const FString& Details);

public:

	UPROPERTY(BlueprintReadWrite)
	UTowerStatComponent* TowerStat;

	UPROPERTY(BlueprintReadWrite)
	UManaComponent* TowerMana;

	UFUNCTION(BlueprintCallable)
	void LevelUp();

	UFUNCTION(BlueprintCallable)
	void CloseWidget();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void InitWidget(UTowerStatComponent* Stat);

	UPROPERTY(BlueprintReadWrite)
	ABaseTower* OwnerTower;

private:
	UPROPERTY(Transient)
	UVerticalBox* DetailedEffectsPanel = nullptr;

	FString LastRenderedDetails;
};
