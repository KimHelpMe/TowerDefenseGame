#pragma once

#include "CoreMinimal.h"
#include "BaseTower.h"
#include "TargetingTower.generated.h"

class AEnemy;
class UAttackTypeComponent;

// 사거리 안의 적을 추적하고 AttackTypeComponent로 공격하는 타워의 공통 부모.
// (AAttackTower: 공격 타이머로 공격, AMagicTower: 마나가 차면 공격)
UCLASS(Abstract)
class TOWERDEFENSE_API ATargetingTower : public ABaseTower
{
	GENERATED_BODY()

public:

	// 사거리 안에서 공격 가능한(공중/지상) 적. 먼저 들어온 순서.
	UPROPERTY(VisibleAnywhere)
	TArray<AEnemy*> CurrentTargets;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAttackTypeComponent> AttackTypeComponent;

	virtual void Tick(float DeltaTime) override;

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

protected:

	virtual void BeginPlay() override;

	// 죽었거나 파괴된 대상을 목록에서 제거. 공격할 대상이 남아 있으면 true.
	bool PruneTargets();

	// 타워의 Ability 태그로 이 적(공중/지상)을 공격할 수 있는지.
	bool CanTarget(const AEnemy* Enemy) const;
};
