
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
	
	// ���� �ߵ�.
	UFUNCTION(BlueprintCallable)
	void ActivateBuff();

	// ���� ����.
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
