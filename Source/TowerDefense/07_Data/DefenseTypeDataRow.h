
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
	float PhysicalMultiplier = 1.f;

	UPROPERTY(EditAnywhere)
	float MagicMultiplier = 1.f;

	UPROPERTY(EditAnywhere)
	float ChaosMultiplier = 1.f;
};
