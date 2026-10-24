
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
	TObjectPtr<UNiagaraSystem> AreaEffect = nullptr;

	// 첫 대상 주변 폭발 반경(cm). 타워 공격 사거리와 별개.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	float ExplosionRadius = 200.f;

public:

	virtual void ExecuteAttack(const TArray<AEnemy*>& Targets) override;
};
