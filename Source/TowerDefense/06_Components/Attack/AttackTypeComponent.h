#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "AttackTypeComponent.generated.h"

class AEnemy;
class ABaseTower;
class UNiagaraSystem;
class UNiagaraComponent;

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

	// 몬스터 피격 시 재생하는 기본 타격 이펙트(한 번 재생). 색은 타워 속성에 맞춰 바뀐다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Effect")
	TObjectPtr<UNiagaraSystem> HitEffect = nullptr;

	// 속성별 타격 이펙트 색. 타워 투사체·범위 이펙트에 고른 색과 같은 계열.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Effect")
	TMap<FGameplayTag, FLinearColor> ElementHitColors;

	// 속성이 없는 타워(혼돈 등)의 타격 이펙트 색.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Effect")
	FLinearColor NoElementHitColor = FLinearColor(0.6f, 0.05f, 1.f);

	// 소유 타워의 속성에 맞는 타격 이펙트 색.
	FLinearColor GetHitColor() const;

	// 타격 이펙트를 지정한 색으로 한 번 재생.
	static UNiagaraComponent* SpawnHitEffect(UWorld* World, UNiagaraSystem* Effect, const FVector& Location, const FLinearColor& Color);
};
