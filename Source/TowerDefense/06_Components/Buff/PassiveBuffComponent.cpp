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
	TArray<AActor*> CurrentTargets = GetTargetsInRange();

	// 범위를 벗어난 대상만 버프 제거.
	for (AActor* Target : PreviousTargets)
	{
		if (!IsValid(Target) || CurrentTargets.Contains(Target)) continue;
		if (UStatComponent* StatComp = Target->GetComponentByClass<UStatComponent>())
		{
			StatComp->RemoveBuffByKey(SourceKey);
		}
	}

	TArray<FBuffEffect> NewBuffs;
	for (const FBuffEffect& Effect : BuffEffects)
	{
		FBuffEffect NewBuff;
		NewBuff.StatTag = Effect.StatTag;
		NewBuff.Value = Effect.Value;
		NewBuff.ValueType = Effect.ValueType;
		NewBuff.Source = GetOwner();
		NewBuff.SourceKey = SourceKey;
		NewBuffs.Add(NewBuff);
	}

	// 범위 안 대상은 교체(이미 같은 버프면 재계산 없이 넘어감).
	for (AActor* Target : CurrentTargets)
	{
		if (!IsValid(Target)) continue;
		if (UStatComponent* StatComp = Target->GetComponentByClass<UStatComponent>())
		{
			StatComp->ReplaceBuffsByKey(SourceKey, NewBuffs);
		}
	}
	PreviousTargets = CurrentTargets;
}

void UPassiveBuffComponent::BeginPlay()
{
	Super::BeginPlay();
	RestartBuffTimer();
}

void UPassiveBuffComponent::RestartBuffTimer()
{
	// BeginPlay 전이면 BeginPlay에서 현재 BuffInterval로 설정된다.
	UWorld* World = GetWorld();
	if (!World || !HasBegunPlay()) return;

	const float Interval = FMath::IsFinite(BuffInterval) ? FMath::Max(0.1f, BuffInterval) : 1.f;
	World->GetTimerManager().SetTimer(BuffTimerHandle, this, &UPassiveBuffComponent::ApplyBuff, Interval, true);
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