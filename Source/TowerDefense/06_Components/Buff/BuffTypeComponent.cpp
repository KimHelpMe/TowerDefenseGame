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
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));

		UKismetSystemLibrary::SphereOverlapActors(
			GetWorld(),
			Center,
			BuffRadius,
			ObjectTypes,
			ABaseTower::StaticClass(),
			IgnoreActors,
			OverlappedActors
		);

		// 오버랩은 다른 타워의 공격 사거리 Sphere(WorldDynamic)까지 잡으므로,
		// 타워 위치(수평 거리)가 버프 반경 안에 있는 경우만 대상으로 남긴다.
		const float RadiusSquared = FMath::Square(BuffRadius);
		OverlappedActors.RemoveAll([&Center, RadiusSquared](const AActor* Actor)
		{
			return !IsValid(Actor) || FVector::DistSquared2D(Center, Actor->GetActorLocation()) > RadiusSquared;
		});
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
