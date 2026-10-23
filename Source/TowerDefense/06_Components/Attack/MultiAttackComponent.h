#pragma once

#include "CoreMinimal.h"
#include "AttackTypeComponent.h"
#include "MultiAttackComponent.generated.h"

class AProjectile;
class UTowerStatComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TOWERDEFENSE_API UMultiAttackComponent : public UAttackTypeComponent
{
	GENERATED_BODY()
	

public:

	UMultiAttackComponent();

	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> ProjectileClass;


	UPROPERTY(EditAnywhere)
	int32 MaxTargetCount;

public:

	virtual void ExecuteAttack(TArray<AEnemy*>Targets) override;

};
