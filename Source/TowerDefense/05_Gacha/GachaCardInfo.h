#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/Texture2D.h"
#include "GachaCardInfo.generated.h"

USTRUCT(BlueprintType)
struct FGachaCardInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString Name;

    UPROPERTY(BlueprintReadWrite)
    FString Grade;

    UPROPERTY(BlueprintReadWrite)
    UTexture2D* Icon = nullptr;

    UPROPERTY(BlueprintReadWrite)
    FString AttackType;

    UPROPERTY(BlueprintReadWrite)
    FGameplayTagContainer SynergyTags;

    UPROPERTY(BlueprintReadWrite)
    FString Description;

    UPROPERTY(BlueprintReadWrite)
    bool bIsUpgrade = false;
};