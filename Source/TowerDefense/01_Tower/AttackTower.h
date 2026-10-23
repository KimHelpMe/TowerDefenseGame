#pragma once

#include "CoreMinimal.h"
#include "BaseTower.h"
#include "Components/SphereComponent.h"
#include "AttackTower.generated.h"

class AEnemy;
class UAttackTypeComponent;

UCLASS()
class TOWERDEFENSE_API AAttackTower : public ABaseTower
{
	GENERATED_BODY()
	


public:

	AAttackTower();

	UPROPERTY(VisibleAnywhere)
	TArray<AEnemy*> CurrentTargets;

	UPROPERTY(EditAnywhere)
	UAttackTypeComponent* AttackTypeComponent;


protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

public:

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


	virtual void Attack_Implementation() override;

	virtual float GetRangeRadius() override;

private:

	//타이머 제어에 필요한 핸들, 타워도 일정 간격으로 발사해야함.
	FTimerHandle AttackTimerHandle;

};
