
#pragma once

#include "CoreMinimal.h"
#include "AttackTypeComponent.h"
#include "AreaAttackComponent.generated.h"

class UNiagaraSystem;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TOWERDEFENSE_API UAreaAttackComponent : public UAttackTypeComponent
{
	GENERATED_BODY()
	


public:

	UAreaAttackComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UNiagaraSystem* AreaEffect = nullptr;

public:

	virtual void ExecuteAttack(TArray<AEnemy*>Targets) override;
};
