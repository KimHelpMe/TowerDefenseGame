#include "PassiveBuffComponent.h"
#include "StatComponent.h"
#include "FBuffEffect.h"

UPassiveBuffComponent::UPassiveBuffComponent()
{

	BuffInterval = 1.f;

}


void UPassiveBuffComponent::ApplyBuff()
{
	// 1. 이전 대상 버프 제거.
	for (AActor* Target : PreviousTargets)
	{
		if (!IsValid(Target)) continue;
		UStatComponent* StatComp = Target->GetComponentByClass<UStatComponent>();

		if (StatComp)
		{
		  StatComp->RemoveBuff(GetOwner());
		}
	}

	// 2. 현재 범위 내 대상 버프 적용.
	TArray<AActor*> CurrentTargets = GetTargetsInRange();
	for (AActor* Target : CurrentTargets)
	{
		UStatComponent* StatComp = Target->GetComponentByClass<UStatComponent>();
		if (StatComp)
		{
			for (FBuffEffect& Effect : BuffEffects)
			{
				FBuffEffect NewBuff;
				NewBuff.StatTag = Effect.StatTag;
				NewBuff.Value = Effect.Value;
				NewBuff.ValueType = Effect.ValueType;
				NewBuff.Source = GetOwner();
				StatComp->ModifyStat(NewBuff);
			}
		}

	}
	PreviousTargets = CurrentTargets;
}

void UPassiveBuffComponent::BeginPlay()
{
	Super::BeginPlay();
	GetWorld()->GetTimerManager().SetTimer(BuffTimerHandle, this, &UPassiveBuffComponent::ApplyBuff, BuffInterval, true);
}