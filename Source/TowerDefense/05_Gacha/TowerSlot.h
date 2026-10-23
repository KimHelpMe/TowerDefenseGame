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
	UStaticMeshComponent* TowerSlotMesh;

	UPROPERTY(EditAnywhere)
	bool bIsOccupied;
	
	UPROPERTY(EditAnywhere)
	int ReloadCount;
	
	UPROPERTY(EditAnywhere)
	bool bIsUIOpen;

	UPROPERTY(EditAnywhere)
	bool bCanAbandon;

	// 스폰된 타워 인스턴스를 참조해야함. 그래서 포인터.
	UPROPERTY(EditAnywhere)
	ABaseTower* InstalledTower;

	UPROPERTY(EditAnywhere)
	AGachaManager* GachaManager;

	UPROPERTY()
	TArray<TSubclassOf<AActor>> SavedRollResult;


	// 슬롯 반환.
	UPROPERTY()
	AGridTile* OwnerTile;


public:	

	virtual void Tick(float DeltaTime) override;

public:

	UFUNCTION()
	void OnClickedSelect(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

	UFUNCTION()
	void CompleteTower();

	UFUNCTION()
	void GiveUpGacha();
};


