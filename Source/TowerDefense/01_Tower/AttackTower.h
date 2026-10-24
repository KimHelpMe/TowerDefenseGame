#pragma once

#include "CoreMinimal.h"
#include "TargetingTower.h"
#include "AttackTower.generated.h"

// 공격 타이머 주기(Stat.AttackSpeed)마다 사거리 안의 적을 공격하는 타워.
// 대상 추적·공격 실행은 ATargetingTower가 담당한다.
UCLASS()
class TOWERDEFENSE_API AAttackTower : public ATargetingTower
{
	GENERATED_BODY()

public:

	AAttackTower();

	// 공격 간격이 바뀌었으면(버프·업그레이드) 공격 타이머를 다시 설정한다.
	void RefreshAttackTimer();

protected:
	virtual void BeginPlay() override;

private:

	// 공격 타이머 콜백. Attack()(BlueprintNativeEvent)을 통해 공격한다.
	UFUNCTION()
	void HandleAttackTimer();

	//타이머 제어에 필요한 핸들, 타워도 일정 간격으로 발사해야함.
	FTimerHandle AttackTimerHandle;

	// 현재 공격 타이머에 적용된 간격(초).
	float AppliedAttackInterval = -1.f;

};
