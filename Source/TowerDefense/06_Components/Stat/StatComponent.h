#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "FBuffEffect.h"
#include "StatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TOWERDEFENSE_API UStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UStatComponent();

	// 속성 , 공격 , 방어 타입
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer Elements;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer AttackTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer DefenseTypes;

public:

	//Damage + Death
	UFUNCTION(BlueprintCallable)
	void ApplyDamage(float Amount);

	UPROPERTY(BlueprintAssignable)
	FOnDeath OnDeath;

public:
	
	// Stat

	UPROPERTY(EditAnywhere)
	TMap<FGameplayTag, float> BaseStats;

	UPROPERTY(EditAnywhere)
	TMap<FGameplayTag, float> CurrentStats;

	UPROPERTY(VisibleAnywhere)
	TArray<FBuffEffect> ActiveBuffs;

	UFUNCTION(BlueprintCallable)
	float GetStat(FGameplayTag StatTag) const;

	UFUNCTION(BlueprintCallable)
	float GetBaseStat(FGameplayTag StatTag) const;

	UFUNCTION(BlueprintCallable)
	void SetBaseStat(FGameplayTag StatTag, float Value);

	UFUNCTION(BlueprintCallable)
	void ModifyStat(FBuffEffect BuffEffect);

	UFUNCTION(BlueprintCallable)
	virtual void RecalculateStats();

	UFUNCTION(BlueprintCallable)
	void RemoveBuff(AActor* Source);

	UFUNCTION(BlueprintCallable)
	void RemoveBuffByKey(FName SourceKey);

	// SourceKey 버프를 NewBuffs로 교체한다. 이미 같으면 아무것도 하지 않고, 바뀌면 재계산을 한 번만 한다.
	void ReplaceBuffsByKey(FName SourceKey, const TArray<FBuffEffect>& NewBuffs);

public:

	static FGameplayTag GetTag(const FString& TagName)
	{
		return FGameplayTag::RequestGameplayTag(FName(*TagName), false);
	}

protected:

	virtual void BeginPlay() override;

public:	

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;



};
