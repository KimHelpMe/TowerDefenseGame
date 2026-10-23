#pragma once

#include "CoreMinimal.h"
#include "AttackTypeComponent.h"
#include "SingleAttackComponent.generated.h"

class AProjectile;
class UTowerStatComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TOWERDEFENSE_API USingleAttackComponent : public UAttackTypeComponent
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> ProjectileClass;

public:

	virtual void ExecuteAttack(TArray<AEnemy*>Targets) override;
};
