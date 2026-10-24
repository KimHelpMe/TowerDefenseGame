#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "FBuffEffect.h"
#include "EpicTowerDataRow.generated.h"

UENUM(BlueprintType)
enum class EEpicTowerType : uint8
{
	Unlock, // 언락형
	Fusion // 합성형
};


USTRUCT(BlueprintType)
struct FEpicTowerConditionEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> TowerClass = nullptr;

	UPROPERTY(EditAnywhere)
	int32 RequiredLevel = 1;
};

USTRUCT(BlueprintType)
struct FEpicTowerDataRow : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> EpicTowerClass = nullptr;

	UPROPERTY(EditAnywhere)
	EEpicTowerType EpicType = EEpicTowerType::Unlock;

	UPROPERTY(EditAnywhere)
	TArray<FEpicTowerConditionEntry> Conditions;

	UPROPERTY(EditAnywhere)
	FString NotifyText;
};






