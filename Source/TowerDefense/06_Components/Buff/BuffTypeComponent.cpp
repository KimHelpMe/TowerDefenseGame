#include "BuffTypeComponent.h"
#include "Enemy.h"
#include "BaseTower.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

UBuffTypeComponent::UBuffTypeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

FName UBuffTypeComponent::GetBuffSourceKey() const
{
	return FName(*FString::Printf(TEXT("BuffComponent_%u"), GetUniqueID()));
}

void UBuffTypeComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UBuffTypeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UBuffTypeComponent::ApplyBuff()
{
}

TArray<AActor*> UBuffTypeComponent::GetTargetsInRange()
{
	FVector Center = GetOwner()->GetActorLocation();
	TArray<AActor*> OverlappedActors;



	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(GetOwner());

	if (BuffTarget == EBuffTarget::AllyTower)
	{
		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));

		UKismetSystemLibrary::SphereOverlapActors(
			GetWorld(),
			Center,
			BuffRadius,
			ObjectTypes,
			ABaseTower::StaticClass(),
			IgnoreActors,
			OverlappedActors
		);

	}

	if (BuffTarget == EBuffTarget::Enemy)
	{
		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));


		UKismetSystemLibrary::SphereOverlapActors(
			GetWorld(),
			Center,
			BuffRadius,
			ObjectTypes,
			AEnemy::StaticClass(),
			IgnoreActors,
			OverlappedActors


		);
}
	return OverlappedActors;
}
