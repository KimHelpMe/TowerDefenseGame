
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
	USphereComponent* BuffRangeComponent;

	UPROPERTY(EditAnywhere)
	float BuffRange;

	UPROPERTY(EditAnywhere)
	UBuffTypeComponent* BuffComponent;

	virtual float GetRangeRadius() override;

protected:
	virtual void BeginPlay() override;

};
