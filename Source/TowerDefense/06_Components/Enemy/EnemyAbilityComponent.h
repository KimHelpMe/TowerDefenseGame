#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyAbilityComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TOWERDEFENSE_API UEnemyAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UEnemyAbilityComponent();

protected:

	virtual void BeginPlay() override;

public:	

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
public:

	UFUNCTION()
	virtual void OnAbilityActivate();

	UFUNCTION()
	virtual void OnEnemyDeathAbility();
};
