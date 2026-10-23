
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "LevelDataRow.generated.h"


USTRUCT(BlueprintType)
struct FLevelDataRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxExp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TokenReward;
};