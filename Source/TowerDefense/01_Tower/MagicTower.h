#pragma once

#include "CoreMinimal.h"
#include "TargetingTower.h"
#include "MagicTower.generated.h"

class UManaComponent;

// 마나가 가득 차면 사거리 안의 적을 공격하는 타워.
// 대상 추적·공격 실행은 ATargetingTower가 담당한다.
UCLASS()
class TOWERDEFENSE_API AMagicTower : public ATargetingTower
{
	GENERATED_BODY()

public:

	AMagicTower();

	UPROPERTY(EditAnywhere)
	TObjectPtr<UManaComponent> ManaComponent;

protected:
	virtual void BeginPlay() override;

public:
	// 델리게이트 수신용
	UFUNCTION()
	void OnManaFull();
};
