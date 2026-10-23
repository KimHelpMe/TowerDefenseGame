#include "PassiveBuffComponent.h"
#include "StatComponent.h"
#include "FBuffEffect.h"

UPassiveBuffComponent::UPassiveBuffComponent()
{

	BuffInterval = 1.f;

}


void UPassiveBuffComponent::ApplyBuff()
{
	const FName SourceKey = GetBuffSourceKey();
	for (AActor* Target : PreviousTargets)
	{
		if (!IsValid(Target)) continue;
		if (UStatComponent* StatComp = Target->GetComponentByClass<UStatComponent>())
		{
			StatComp->RemoveBuffByKey(SourceKey);
		}
	}

	TArray<AActor*> CurrentTargets = GetTargetsInRange();
	for (AActor* Target : CurrentTargets)
	{
		if (!IsValid(Target)) continue;
		UStatComponent* StatComp = Target->GetComponentByClass<UStatComponent>();
		if (!StatComp) continue;

		for (const FBuffEffect& Effect : BuffEffects)
		{
			FBuffEffect NewBuff;
			NewBuff.StatTag = Effect.StatTag;
			NewBuff.Value = Effect.Value;
			NewBuff.ValueType = Effect.ValueType;
			NewBuff.Source = GetOwner();
			NewBuff.SourceKey = SourceKey;
			StatComp->ModifyStat(NewBuff);
		}
	}
	PreviousTargets = CurrentTargets;
}

void UPassiveBuffComponent::BeginPlay()
{
	Super::BeginPlay();
	const float Interval = FMath::Max(0.1f, BuffInterval);
	GetWorld()->GetTimerManager().SetTimer(BuffTimerHandle, this, &UPassiveBuffComponent::ApplyBuff, Interval, true);
}

void UPassiveBuffComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BuffTimerHandle);
	}

	const FName SourceKey = GetBuffSourceKey();
	for (AActor* Target : PreviousTargets)
	{
		if (!IsValid(Target)) continue;
		if (UStatComponent* StatComp = Target->GetComponentByClass<UStatComponent>())
		{
			StatComp->RemoveBuffByKey(SourceKey);
		}
	}
	PreviousTargets.Empty();

	Super::EndPlay(EndPlayReason);
}