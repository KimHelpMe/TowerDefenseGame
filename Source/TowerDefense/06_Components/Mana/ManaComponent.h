#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ManaComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnManaFull);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnManaChanged, float, ManaPercent);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TOWERDEFENSE_API UManaComponent : public UActorComponent
{
	GENERATED_BODY()


public:

	UManaComponent();

	UPROPERTY(EditAnywhere)
	float CurrentMana;

	UPROPERTY(EditAnywhere)
	float MaxMana;

	// 마나 회복량(Tick).
	UPROPERTY(EditAnywhere)
	float ManaRegenRate;

	// 회복 간격.
	UPROPERTY(EditAnywhere)
	float ManaRegenInterval;

	// 공격 시 소모량.
	UPROPERTY(EditAnywhere)
	float ManaCost;

	// 마나 충전.
	UPROPERTY(BlueprintAssignable)
	FOnManaChanged OnManaChanged;


	// 자동 온오프.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoFire = true;

protected:
	virtual void BeginPlay() override;

public:
	void RefreshRegenTimer();

	// 타이머 호출, 마나회복 + 최대 도달 시 공격
	UFUNCTION()
	void RegenMana();

	UFUNCTION(BlueprintCallable)
	void ConsumeMana();

	UPROPERTY(BlueprintAssignable)
	FOnManaFull OnManaFull;

private:

	// 마나 회복 타이머를 제어할 핸들.
	FTimerHandle ManaRegenTimerHandle;
};
