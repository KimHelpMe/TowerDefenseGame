
#pragma once

#include "CoreMinimal.h"
#include "BaseTower.h"
#include "Components/SphereComponent.h"

#include "BuffTower.generated.h"

class UBuffTypeComponent;

UCLASS()
class TOWERDEFENSE_API ABuffTower : public ABaseTower
{
	GENERATED_BODY()
	

public:

	ABuffTower();

	UPROPERTY(EditAnywhere)
	TObjectPtr<USphereComponent> BuffRangeComponent;

	UPROPERTY(EditAnywhere)
	float BuffRange;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBuffTypeComponent> BuffComponent;

	virtual float GetRangeRadius() override;

protected:
	virtual void BeginPlay() override;

private:
	// GetRangeRadius에서 버프 컴포넌트를 고른 레벨/테이블(바뀔 때만 데이터 행을 다시 조회).
	int32 CachedBuffComponentLevel = INDEX_NONE;
	TWeakObjectPtr<UDataTable> CachedBuffComponentTable;

};
