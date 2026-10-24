#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttackTypeComponent.generated.h"

class AEnemy;
class ABaseTower;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TOWERDEFENSE_API UAttackTypeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UAttackTypeComponent();


protected:

	virtual void BeginPlay() override;

public:	

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	
	virtual void ExecuteAttack(const TArray<AEnemy*>& Targets);
	
	UFUNCTION()
	FVector GetMuzzleLocation();
};
