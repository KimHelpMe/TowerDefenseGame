#pragma once

#include "CoreMinimal.h"
#include "BaseTower.h"
#include "MagicTower.generated.h"


class AEnemy;
class UAttackTypeComponent;
class UManaComponent;

UCLASS()
class TOWERDEFENSE_API AMagicTower : public ABaseTower
{
	GENERATED_BODY()
	

public:

	AMagicTower();

	UPROPERTY(EditAnywhere)
	TArray<AEnemy*> CurrentTargets;

	UPROPERTY(EditAnywhere)
	UAttackTypeComponent* AttackTypeComponent;

	UPROPERTY(EditAnywhere)
	UManaComponent* ManaComponent;

protected:
	virtual void BeginPlay() override;

public:
	// 델리게이트 수신용
	UFUNCTION()
	void OnManaFull();

	virtual void Attack_Implementation() override;

	UFUNCTION()
	void OnEnemyEnter(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnEnemyExit(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);


};
