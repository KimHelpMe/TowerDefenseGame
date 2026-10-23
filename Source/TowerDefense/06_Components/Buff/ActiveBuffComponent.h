
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
	
	bool bIsBuffActive;

public:
	
	// 버프 발동.
	UFUNCTION()
	void ActivateBuff();

	// 버프 종료.
	UFUNCTION()
	void DeactivateBuff();

	virtual void ApplyBuff() override;

public:

	UPROPERTY(EditAnywhere)
	TArray<AActor*> PreviousTargets;

private:

	FTimerHandle CooldownTimerHandle;
	
	FTimerHandle DurationTimerHandle;

};
