#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "SynergyDataRow.h"
#include "SynergyManager.generated.h"

class AEpicTowerManager;
class ABaseTower;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSynergyUpdated);

UCLASS()
class TOWERDEFENSE_API ASynergyManager : public AActor
{
	GENERATED_BODY()
	
public:	

	ASynergyManager();


public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UDataTable> SynergyDataTable = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<FGameplayTag, int32> ActiveSynergySteps;

	UPROPERTY(VisibleAnywhere)
	TArray<ABaseTower*> FieldTowers;

public:

	UFUNCTION()
	void RegisterTower(ABaseTower* Tower);

	UFUNCTION()
	void UnregisterTower(ABaseTower* Tower);

	UFUNCTION()
	void UpdateSynergies();

public:

	// 시너지 UI.

	UPROPERTY(BlueprintAssignable)
	FOnSynergyUpdated OnSynergyUpdated;

	// 태그별 현재 보유 수.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<FGameplayTag, int32> SynergyCountMap;

protected:

	virtual void BeginPlay() override;

public:	

	virtual void Tick(float DeltaTime) override;

};
