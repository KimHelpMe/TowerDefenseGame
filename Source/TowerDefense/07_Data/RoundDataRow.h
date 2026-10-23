
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
	int32 SpawnCount;

	UPROPERTY(EditAnywhere)
	float SpawnDelay;

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
	float GoldReward;

	UPROPERTY(EditAnywhere)
	float TowerTokenReward;

	UPROPERTY(EditAnywhere)
	float ExpReward;
};
