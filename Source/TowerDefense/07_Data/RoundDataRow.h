
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RoundDataRow.generated.h"

class AEnemy;

USTRUCT(BlueprintType)
struct FRoundEnemyEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSubclassOf<AEnemy> EnemyClass;

	UPROPERTY(EditAnywhere)
	int32 SpawnCount = 1;

	UPROPERTY(EditAnywhere)
	float SpawnDelay = 1.f;

};

USTRUCT(BlueprintType)
struct FRoundDataRow : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
	TArray<FRoundEnemyEntry> EnemyEntries;

	// Reward.

	UPROPERTY(EditAnywhere)
	float GoldReward = 0.f;

	UPROPERTY(EditAnywhere)
	float TowerTokenReward = 0.f;

	UPROPERTY(EditAnywhere)
	float ExpReward = 0.f;
};
