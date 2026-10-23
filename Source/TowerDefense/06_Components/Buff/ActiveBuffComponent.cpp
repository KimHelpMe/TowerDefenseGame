
#include "ActiveBuffComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TowerDefenseGameMode.h"
#include "StatComponent.h"
#include "FBuffEffect.h"

UActiveBuffComponent::UActiveBuffComponent()
{
	Duration = 5.f;
	Cooldown = 10.f;
	bIsBuffActive = false;
}


void UActiveBuffComponent::ActivateBuff()
{
	if (bIsBuffActive == true) return;

	bIsBuffActive = true;
	ApplyBuff();

	ATowerDefenseGameMode* GameMode = Cast<ATowerDefenseGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	float DurationBonus = GameMode ? GameMode->GetUpgradeBonus(
		FGameplayTag::RequestGameplayTag("Upgrade.BuffDuration")) : 0.f;

	float FinalDuration = Duration + DurationBonus;

	GetWorld()->GetTimerManager().SetTimer(DurationTimerHandle, this, &UActiveBuffComponent::DeactivateBuff, FinalDuration, false);
}

void UActiveBuffComponent::DeactivateBuff()
{

	// 버프 제거.
	for (AActor* Target : PreviousTargets)
	{
		if (!IsValid(Target)) continue;
		UStatComponent* StatComp = Target->GetComponentByClass<UStatComponent>();
		if (StatComp)
		{
			StatComp->RemoveBuff(GetOwner());
		}
	}

	PreviousTargets.Empty();

	bIsBuffActive = false;
	GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, this, &UActiveBuffComponent::ActivateBuff, Cooldown, false);
}

void UActiveBuffComponent::ApplyBuff()
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



