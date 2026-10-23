#pragma once

#include "CoreMinimal.h"
#include "EnemyAbilityComponent.h"
#include "ShieldAbilityComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TOWERDEFENSE_API UShieldAbilityComponent : public UEnemyAbilityComponent
{
	GENERATED_BODY()
	

public:

	UShieldAbilityComponent();

	UPROPERTY(EditAnywhere)
	float ShieldAmount;

	UPROPERTY(EditAnywhere)
	float MaxShieldAmount;

public:

	UFUNCTION()
	float AbsorbDamage(float IncomingDamage);

};
