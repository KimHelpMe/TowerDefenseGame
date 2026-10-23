#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "FBuffEffect.h"
#include "Engine/Texture2D.h"
#include "SynergyDataRow.generated.h"

USTRUCT(BlueprintType)
struct FSynergyStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequireCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FBuffEffect> Effects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString EffectDesc;
};


// DataTable Row 구조체.
USTRUCT(BlueprintType)
struct FSynergyDataRow : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag SynergyTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSynergyStep> Steps;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SynergyDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor BadgeColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* SynergyIcon = nullptr;
};
