
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "EpicTowerDataRow.h"
#include "EpicTowerManager.generated.h"

class ABaseTower;
class ASynergyManager;
class AGachaManager;
class UEpicNotifyWidget;

UCLASS()
class TOWERDEFENSE_API AEpicTowerManager : public AActor
{
	GENERATED_BODY()
	
public:	

	AEpicTowerManager();

public:

	UPROPERTY(EditAnywhere)
	UDataTable* EpicTowerDataTable = nullptr;

	UPROPERTY(VisibleAnywhere)
	TArray<TSubclassOf<AActor>> UnlockedEpicTowers;

	UPROPERTY(VisibleAnywhere)
	TArray<TSubclassOf<AActor>> PendingFusionTowers;

	UFUNCTION()
	void CheckConditions(const TArray<ABaseTower*>& FieldTowers);

public:

	//// Epic Notify.
	UPROPERTY(EditAnywhere)
	TSubclassOf<UEpicNotifyWidget> NotifyWidgetClass;

	UFUNCTION()
	void ShowNotifyText(const FString& Text);

public:

	// 합성 재료를 찾기만 한다(소비하지 않음). 모든 조건을 서로 다른 타워가 충족해야 true.
	bool FindFusionMaterials(TSubclassOf<AActor> EpicTowerClass,
		const TArray<ABaseTower*>& FieldTowers,
		TArray<ABaseTower*>& OutMaterials) const;

	// FindFusionMaterials로 찾은 재료 타워를 파괴하고 슬롯을 비운다.
	void ConsumeFusionMaterials(TSubclassOf<AActor> EpicTowerClass,
		const TArray<ABaseTower*>& Materials,
		AGachaManager* GachaManager,
		ASynergyManager* SynergyManager);

	// 재료를 찾아 소비한다. 재료가 부족하면 아무것도 소비하지 않고 false.
	UFUNCTION()
	bool FuseEpicTower(TSubclassOf<AActor> EpicTowerClass,
		const TArray<ABaseTower*>& FieldTowers,
		AGachaManager* GachaManager,
		ASynergyManager* SynergyManager);

	// 조건이 더 이상 충족되지 않는 합성 카드를 대기 목록에서 제거.
	void CancelPendingFusion(TSubclassOf<AActor> EpicTowerClass, AGachaManager* GachaManager);

protected:

	virtual void BeginPlay() override;

public:	

	virtual void Tick(float DeltaTime) override;

};
