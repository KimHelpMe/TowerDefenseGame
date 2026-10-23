#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FBuffEffect.generated.h"

UENUM(BlueprintType)
enum class EBuffValueType : uint8
{
    Flat,
    Percent
};

USTRUCT(BlueprintType)
struct FBuffEffect
{
    GENERATED_BODY()
   
    UPROPERTY(EditAnywhere)
    FGameplayTag StatTag;

    UPROPERTY(EditAnywhere)
    float Value;

    UPROPERTY(EditAnywhere)
    EBuffValueType ValueType;

    UPROPERTY()
    AActor* Source = nullptr;

    UPROPERTY()
    FName SourceKey = NAME_None;


};