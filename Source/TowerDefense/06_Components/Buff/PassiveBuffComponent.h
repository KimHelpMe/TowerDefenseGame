#pragma once

#include "CoreMinimal.h"
#include "BuffTypeComponent.h"
#include "PassiveBuffComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TOWERDEFENSE_API UPassiveBuffComponent : public UBuffTypeComponent
{
	GENERATED_BODY()
	
public:

	UPassiveBuffComponent();

	UPROPERTY(EditAnywhere)
	TArray<FBuffEffect> BuffEffects;

	UPROPERTY(EditAnywhere)
	float BuffInterval;

public:

	UPROPERTY(EditAnywhere)
	TArray<AActor*> PreviousTargets;

public:

	virtual void ApplyBuff() override;

protected:

	virtual void BeginPlay() override;

public:

	FTimerHandle BuffTimerHandle;

};

