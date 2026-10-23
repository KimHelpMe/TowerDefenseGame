#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "FBuffEffect.h"
#include "BuffTypeComponent.generated.h"



UENUM(BlueprintType)
enum class EBuffTarget : uint8
{
	AllyTower,
	Enemy
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TOWERDEFENSE_API UBuffTypeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UBuffTypeComponent();

	UPROPERTY(EditAnywhere)
	float BuffRadius;
	
	UPROPERTY(EditAnywhere)
	EBuffTarget BuffTarget;


protected:

	virtual void BeginPlay() override;

public:	

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:

	virtual void ApplyBuff();
		
	UFUNCTION()
	TArray<AActor*> GetTargetsInRange();
};
