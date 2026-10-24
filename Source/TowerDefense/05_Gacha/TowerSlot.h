#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "GridTile.h"
#include "TowerSlot.generated.h"

class ABaseTower;
class AGachaManager;

UCLASS()
class TOWERDEFENSE_API ATowerSlot : public AActor
{
	GENERATED_BODY()
	
public:	
	ATowerSlot();

public:

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> TowerSlotMesh;

	UPROPERTY(EditAnywhere)
	bool bIsOccupied;
	
	UPROPERTY(EditAnywhere)
	int ReloadCount;
	
	UPROPERTY(EditAnywhere)
	bool bCanAbandon;

	// 스폰된 타워 인스턴스를 참조해야함. 그래서 포인터.
	UPROPERTY(EditAnywhere)
	TObjectPtr<ABaseTower> InstalledTower;

	UPROPERTY(EditAnywhere)
	TObjectPtr<AGachaManager> GachaManager;

	UPROPERTY()
	TArray<TSubclassOf<AActor>> SavedRollResult;


	// 슬롯 반환.
	UPROPERTY()
	TObjectPtr<AGridTile> OwnerTile;


public:	

	virtual void Tick(float DeltaTime) override;

public:

	UFUNCTION()
	void OnClickedSelect(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

	// 뽑기 결과·리롤 횟수를 슬롯 클래스 기본값으로 되돌림(타워 선택 후, 슬롯이 비워질 때).
	UFUNCTION()
	void ResetGachaState();
};


