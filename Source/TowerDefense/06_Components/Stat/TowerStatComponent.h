
#pragma once

#include "CoreMinimal.h"
#include "StatComponent.h"
#include "GameplayTagContainer.h"
#include "Components/SphereComponent.h"
#include "Engine/DataTable.h"
#include "TowerDataRow.h"
#include "TowerStatComponent.generated.h"




UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TOWERDEFENSE_API UTowerStatComponent : public UStatComponent
{
	GENERATED_BODY()
	

public:

	UTowerStatComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TowerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TowerInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Grade;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TowerType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer SynergyTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* TowerIcon;

	UPROPERTY(EditAnywhere)
	FGameplayTagContainer Abilities;


public:

	// Data Table

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* TowerDataTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TowerRowName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxLevel = 3;

	virtual void RecalculateStats() override;

	// 기본 사거리에 인게임 사거리 업그레이드를 적용한 최종 공격 사거리.
	UFUNCTION(BlueprintCallable)
	float GetFinalAttackRange() const;

	UFUNCTION()
	void InitFromDataTable();

	UFUNCTION()
	void LevelUp();

	UFUNCTION(BlueprintCallable)
	float GetNextLevelGoldCost();

private:

	UFUNCTION()
	void ApplyBuffFromData(const FBuffConfig& BuffSetting);

protected:

	virtual void BeginPlay() override;

public:

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
