#include "TowerRangeDecalComponent.h"
#include "BaseTower.h"
#include "Components/DecalComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
    bool IsFiniteColor(const FLinearColor& Color)
    {
        return FMath::IsFinite(Color.R) && FMath::IsFinite(Color.G) && FMath::IsFinite(Color.B) && FMath::IsFinite(Color.A);
    }
}

UTowerRangeDecalComponent::UTowerRangeDecalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    PrimaryComponentTick.TickInterval = 0.05f;
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(
        TEXT("/Game/05_Materials/M_TowerRangeOutline.M_TowerRangeOutline"));
    if (Material.Succeeded()) OutlineMaterial = Material.Object;
}

void UTowerRangeDecalComponent::BeginPlay()
{
    Super::BeginPlay();
    RefreshDecal();
}

void UTowerRangeDecalComponent::SetRangeColor(FLinearColor NewColor)
{
    if (!IsFiniteColor(NewColor)) return;
    RangeColor = NewColor;
    RefreshDecal();
}

void UTowerRangeDecalComponent::SetRangeOpacity(float NewOpacity)
{
    if (!FMath::IsFinite(NewOpacity)) return;
    Opacity = FMath::Clamp(NewOpacity, 0.f, 1.f);
    RefreshDecal();
}

void UTowerRangeDecalComponent::SetRangeVisible(bool bVisible)
{
    bRangeVisible = bVisible;
    SetComponentTickEnabled(bVisible);
    RefreshDecal();
}

void UTowerRangeDecalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    RefreshDecal();
}

void UTowerRangeDecalComponent::RefreshDecal()
{
    ABaseTower* Tower = Cast<ABaseTower>(GetOwner());
    if (!IsValid(Tower) || !IsValid(Tower->RangeDecalComponent)) return;
    TargetDecal = Tower->RangeDecalComponent;

    if (OutlineMaterial && (!DynamicMaterial || AppliedMaterial != OutlineMaterial))
    {
        DynamicMaterial = UMaterialInstanceDynamic::Create(OutlineMaterial, this);
        AppliedMaterial = OutlineMaterial;
    }
    if (DynamicMaterial)
    {
        TargetDecal->SetDecalMaterial(DynamicMaterial);
        const FLinearColor SafeColor = !IsFiniteColor(RangeColor) ? FLinearColor::White : RangeColor;
        DynamicMaterial->SetVectorParameterValue(TEXT("RangeColor"), SafeColor);
        const float SafeOpacity = FMath::IsFinite(Opacity) ? Opacity : 1.f;
        DynamicMaterial->SetScalarParameterValue(TEXT("RangeOpacity"), FMath::Clamp(SafeOpacity * SafeColor.A, 0.f, 1.f));
    }

    const float Radius = Tower->GetRangeRadius();
    const bool bValidRadius = FMath::IsFinite(Radius) && Radius > 0.f;
    TargetDecal->SetVisibility(bRangeVisible && bValidRadius && IsValid(DynamicMaterial));
    if (!bRangeVisible || !bValidRadius) return;

    // Source image's outer radius is 500 px in a 1024 px canvas. Compensate its transparent margin.
    const float Extent = Radius * (512.f / 500.f);
    const float Depth = FMath::IsFinite(ProjectionDepth) ? FMath::Max(1.f, ProjectionDepth) : 2000.f;
    const float Height = FMath::IsFinite(ProjectionHeight) ? ProjectionHeight : 250.f;
    TargetDecal->SetAbsolute(false, false, true);
    TargetDecal->SetWorldScale3D(FVector::OneVector);
    TargetDecal->DecalSize = FVector(Depth, Extent, Extent);
    TargetDecal->SetWorldLocation(Tower->GetActorLocation() + FVector(0.f, 0.f, Height));
    TargetDecal->SetWorldRotation(FRotator(-90.f, 0.f, 0.f));
    TargetDecal->SetFadeScreenSize(0.f);
    TargetDecal->SetSortOrder(20);
}
