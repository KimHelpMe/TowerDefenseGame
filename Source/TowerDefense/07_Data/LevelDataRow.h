
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "LevelDataRow.generated.h"


USTRUCT(BlueprintType)
struct FLevelDataRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxExp = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TokenReward = 0.f;
};