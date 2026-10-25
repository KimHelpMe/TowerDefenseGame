#include "AttackTypeComponent.h"
#include "BaseTower.h"
#include "Enemy.h"
#include "TowerDefenseTags.h"
#include "TowerStatComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

UAttackTypeComponent::UAttackTypeComponent()
{
	PrimaryComponentTick.bCanEverTick = false; // Tick does nothing; BP Event Tick re-enables it on compile.

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> HitEffectAsset(TEXT("/Game/Free_Magic/VFX_Niagara/NS_Free_Magic_Hit1.NS_Free_Magic_Hit1"));
	if (HitEffectAsset.Succeeded()) HitEffect = HitEffectAsset.Object;

	ElementHitColors.Add(TDTags::Element_Fire, FLinearColor(1.f, 0.09f, 0.f));
	ElementHitColors.Add(TDTags::Element_Water, FLinearColor(0.f, 0.35f, 1.f));
	ElementHitColors.Add(TDTags::Element_Wind, FLinearColor(0.15f, 1.f, 0.1f));
	ElementHitColors.Add(TDTags::Element_Earth, FLinearColor(0.8f, 0.45f, 0.05f));
}

void UAttackTypeComponent::BeginPlay()
{
	Super::BeginPlay();


	
}

void UAttackTypeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UAttackTypeComponent::ExecuteAttack(const TArray<AEnemy*>& Targets)
{
}

FVector UAttackTypeComponent::GetMuzzleLocation()
{
	ABaseTower* Tower = Cast<ABaseTower>(GetOwner());
	if (!Tower) return FVector::ZeroVector;
	
		return Tower->MuzzleComponent->GetComponentLocation();
	
}


FLinearColor UAttackTypeComponent::GetHitColor() const
{
	const UTowerStatComponent* TowerStat = GetOwner() ? GetOwner()->GetComponentByClass<UTowerStatComponent>() : nullptr;
	if (TowerStat)
	{
		for (const TPair<FGameplayTag, FLinearColor>& Entry : ElementHitColors)
		{
			if (TowerStat->Elements.HasTag(Entry.Key)) return Entry.Value;
		}
	}
	return NoElementHitColor;
}

UNiagaraComponent* UAttackTypeComponent::SpawnHitEffect(UWorld* World, UNiagaraSystem* Effect, const FVector& Location, const FLinearColor& Color)
{
	if (!World || !Effect) return nullptr;

	UNiagaraComponent* HitComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, Effect, Location);
	if (!HitComp) return nullptr;

	// NS_Free_Magic_Hit1의 원래 HDR 밝기(섬광·연기·나선 50, 불꽃 100)를 유지하고 색상만 바꾼다.
	auto Bright = [&Color](float Intensity) { return FLinearColor(Color.R * Intensity, Color.G * Intensity, Color.B * Intensity, Color.A); };
	HitComp->SetVariableLinearColor(TEXT("Color_Flash"), Bright(50.f));
	HitComp->SetVariableLinearColor(TEXT("Color_Smoke"), Bright(50.f));
	HitComp->SetVariableLinearColor(TEXT("Color_Sparks1"), Bright(100.f));
	HitComp->SetVariableLinearColor(TEXT("Color_Spiral1"), Bright(50.f));
	return HitComp;
}
