#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "FBuffEffect.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "Materials/MaterialInstance.h"
#include "Projectile.h"
#include "BuffTypeComponent.h"


#include "TowerDataRow.generated.h"


UENUM(BlueprintType)
enum class EBuffComponentType : uint8
{
	None,
	Passive,
	Active
};

USTRUCT(BlueprintType)
struct FBuffConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	EBuffComponentType BuffType = EBuffComponentType::None;

	UPROPERTY(EditAnywhere)
	TArray<FBuffEffect> BuffEffects;

	UPROPERTY(EditAnywhere)
	EBuffTarget BuffTarget = EBuffTarget::AllyTower;

	UPROPERTY(EditAnywhere)
	float BuffInterval = 0.f;
	
	UPROPERTY(EditAnywhere)
	float BuffDuration = 0.f;
	
	UPROPERTY(EditAnywhere)
	float BuffCooldown = 0.f;
	
	UPROPERTY(EditAnywhere)
	float BuffRadius = 0.f;


};


USTRUCT(BlueprintType)
struct FTowerDataRow : public FTableRowBase
{
	GENERATED_BODY()


public:
	// 기본 정보.
	
	UPROPERTY(EditAnywhere)
	FString TowerName;

	UPROPERTY(EditAnywhere)
	FString Grade;

	UPROPERTY(EditAnywhere)
	int32 Level = 1;

	UPROPERTY(EditAnywhere)
	int32 MaxLevel = 3;

	UPROPERTY(EditAnywhere)
	FString TowerType;

	UPROPERTY(EditAnywhere)
	float GoldCost = 0.f;

	// 스탯.
	UPROPERTY(EditAnywhere)
	TArray<FBuffEffect> Stats;

	// 태크(Tag).
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer Elements;

	UPROPERTY(EditAnywhere)
	FGameplayTagContainer AttackTypes;

	UPROPERTY(EditAnywhere)
	FGameplayTagContainer Abilities;

	UPROPERTY(EditAnywhere)
	FGameplayTagContainer SynergyTags;
	

	// 버프.
	UPROPERTY(EditAnywhere)
	FBuffConfig BuffSetting;

	// 외형.
	

	UPROPERTY(EditAnywhere)
	UTexture2D* Icon = nullptr;

};