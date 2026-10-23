
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

	UFUNCTION()
	void FuseEpicTower(TSubclassOf<AActor> EpicTowerClass,
		const TArray<ABaseTower*>& FieldTowers,
		AGachaManager* GachaManager,
		ASynergyManager* SynergyManager);

protected:

	virtual void BeginPlay() override;

public:	

	virtual void Tick(float DeltaTime) override;

};
