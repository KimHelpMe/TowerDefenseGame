
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ElementalDataRow.generated.h"

USTRUCT(BlueprintType)
struct FElementalDataRow : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
	TMap<FGameplayTag, float> Multipliers;

};
