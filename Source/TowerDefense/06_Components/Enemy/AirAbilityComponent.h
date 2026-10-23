
#pragma once

#include "CoreMinimal.h"
#include "EnemyAbilityComponent.h"
#include "AirAbilityComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TOWERDEFENSE_API UAirAbilityComponent : public UEnemyAbilityComponent
{
	GENERATED_BODY()
	

public:

	UAirAbilityComponent();

	UPROPERTY(EditAnywhere)
	bool bIsAirUnit = true;

};
