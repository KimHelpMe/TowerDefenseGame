
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "FBuffEffect.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "Materials/MaterialInstance.h"
#include "Engine/Texture2D.h"
#include "Engine/SkeletalMesh.h"
#include "EnemyDataRow.generated.h"


USTRUCT(BlueprintType)
struct FEnemyDataRow : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
	FString EnemyName;

	UPROPERTY(EditAnywhere)
	FString EnemyGrade;

	UPROPERTY(EditAnywhere)
	TArray<FBuffEffect> Stats;

	UPROPERTY(EditAnywhere)
	FGameplayTagContainer Elements;

	UPROPERTY(EditAnywhere)
	FGameplayTagContainer DefenseTypes;

	UPROPERTY(EditAnywhere)
	TMap<FGameplayTag, float> ElementResistRate;

	UPROPERTY(EditAnywhere)
	float GoldReward;

	UPROPERTY(EditAnywhere)
	float ExpReward;

	// 외형
	UPROPERTY(EditAnywhere)
	USkeletalMesh* EnemyMesh = nullptr;

	UPROPERTY(EditAnywhere)
	UMaterialInstance* EnemyMaterial = nullptr;

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* EnemyEffect = nullptr;

	UPROPERTY(EditAnywhere)
	UTexture2D* Icon = nullptr;

	UPROPERTY(EditAnywhere)
	USoundBase* Sound = nullptr;

	// 공중 여부.

	UPROPERTY(EditAnywhere)
	bool bIsAirUnit = false;

};