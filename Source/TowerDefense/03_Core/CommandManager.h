
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CommandManager.generated.h"


class ABase;
class ABossEnemy;
class AEnemy;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCooldownUpdated, float, HealCooldown, float, PurgeCooldown);

UCLASS()
class TOWERDEFENSE_API ACommandManager : public AActor
{
	GENERATED_BODY()

public:

	ACommandManager();

public:

	UPROPERTY(EditAnywhere)
	float HealCooldown;

	UPROPERTY(EditAnywhere)
	float HealCooldownRemaining;

	UPROPERTY(EditAnywhere)
	float PurgeCooldown;

	UPROPERTY(EditAnywhere)
	float PurgeCooldownRemaining;

	UPROPERTY(EditAnywhere)
	int32 CurrentSpeedIndex;

	UPROPERTY(VisibleAnywhere)
	ABase* Base;

public:

	UFUNCTION()
	bool CanHeal();

	UFUNCTION()
	bool CanPurge();

	UFUNCTION(BlueprintCallable)
	void HealBase();

	UFUNCTION(BlueprintCallable)
	void PurgeAllEnemies();

	UFUNCTION(BlueprintCallable)
	void SetGameSpeed(int32 SpeedIndex);

	UFUNCTION()
	void TickCooldowns(float DeltaTime);


public:

	UPROPERTY(BlueprintAssignable)
	FOnCooldownUpdated OnCooldownUpdated;



protected:

	virtual void BeginPlay() override;

public:

	virtual void Tick(float DeltaTime) override;

};
