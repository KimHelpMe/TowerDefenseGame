#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TowerRangeDecalComponent.generated.h"

class UDecalComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

/** Controls the selected tower's outline decal without changing its gameplay range. */
UCLASS(ClassGroup=(Rendering), meta=(BlueprintSpawnableComponent))
class TOWERDEFENSE_API UTowerRangeDecalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTowerRangeDecalComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tower Range|Appearance")
    FLinearColor RangeColor = FLinearColor(0.08f, 1.f, 0.7f, 1.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tower Range|Appearance", meta=(ClampMin="0", ClampMax="1"))
    float Opacity = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tower Range|Appearance")
    TObjectPtr<UMaterialInterface> OutlineMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tower Range|Projection", meta=(ClampMin="1", Units="cm"))
    float ProjectionDepth = 2000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tower Range|Projection", meta=(Units="cm"))
    float ProjectionHeight = 250.f;

    UFUNCTION(BlueprintCallable, Category="Tower Range")
    void SetRangeColor(FLinearColor NewColor);

    UFUNCTION(BlueprintCallable, Category="Tower Range")
    void SetRangeOpacity(float NewOpacity);

    UFUNCTION(BlueprintCallable, Category="Tower Range")
    void SetRangeVisible(bool bVisible);

    UFUNCTION(BlueprintPure, Category="Tower Range")
    bool IsRangeVisible() const { return bRangeVisible; }

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void RefreshDecal();

    UPROPERTY(Transient)
    TObjectPtr<UDecalComponent> TargetDecal;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInterface> AppliedMaterial;

    bool bRangeVisible = false;
};
