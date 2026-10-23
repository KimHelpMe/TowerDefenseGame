
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DefenseTypeDataRow.generated.h"



USTRUCT(BlueprintType)
struct FDefenseTypeDataRow : public FTableRowBase
{
	GENERATED_BODY()

public:


	UPROPERTY(EditAnywhere)
	float PhysicalMultiplier;

	UPROPERTY(EditAnywhere)
	float MagicMultiplier;

	UPROPERTY(EditAnywhere)
	float ChaosMultiplier;
};
