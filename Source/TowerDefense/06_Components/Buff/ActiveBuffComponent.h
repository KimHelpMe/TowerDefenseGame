
#pragma once

#include "CoreMinimal.h"
#include "BuffTypeComponent.h"
#include "ActiveBuffComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TOWERDEFENSE_API UActiveBuffComponent : public UBuffTypeComponent
{
	GENERATED_BODY()
	

public:

	UActiveBuffComponent();

	UPROPERTY(EditAnywhere)
	TArray<FBuffEffect> BuffEffects;

	UPROPERTY(EditAnywhere)
	float Duration;
	
	UPROPERTY(EditAnywhere)
	float Cooldown;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsBuffActive;

public:
	
	// 버프 발동(지속시간 동안 범위 내 대상에 적용).
	UFUNCTION(BlueprintCallable)
	void ActivateBuff();

	// 버프 해제 후 재사용 대기 시작.
	UFUNCTION()
	void DeactivateBuff();

	virtual void ApplyBuff() override;

protected:

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:

	UPROPERTY(EditAnywhere)
	TArray<AActor*> PreviousTargets;

private:

	FTimerHandle CooldownTimerHandle;
	
	FTimerHandle DurationTimerHandle;

};
