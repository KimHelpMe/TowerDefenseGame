
#pragma once

#include "CoreMinimal.h"
#include "AttackTypeComponent.h"
#include "AreaAttackComponent.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;

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

	// AreaEffect를 원래 크기(Scale_All 기본값)로 재생했을 때 덮는 반경(cm).
	// 폭발 반경에 맞춰 이펙트를 넓게 펼치는 기준값.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1"))
	float AreaEffectBaseRadius = 150.f;

public:

	virtual void ExecuteAttack(const TArray<AEnemy*>& Targets) override;

	// 폭발 반경에 맞춰 넓힌 AreaEffect를 재생.
	UNiagaraComponent* SpawnAreaEffect(const FVector& Center) const;
};
