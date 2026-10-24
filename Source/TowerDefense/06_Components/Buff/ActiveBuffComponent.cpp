
#include "ActiveBuffComponent.h"
#include "TowerDefenseTags.h"
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
	UWorld* World = GetWorld();
	if (!World || !IsValid(GetOwner())) return;

	bIsBuffActive = true;
	ApplyBuff();

	ATowerDefenseGameMode* GameMode = Cast<ATowerDefenseGameMode>(
		UGameplayStatics::GetGameMode(World));
	const float DurationBonus = GameMode ? GameMode->GetUpgradeBonus(
		TDTags::Upgrade_BuffDuration) : 0.f;
	const float FinalDuration = FMath::Max(0.1f, Duration + DurationBonus);

	World->GetTimerManager().SetTimer(
		DurationTimerHandle, this, &UActiveBuffComponent::DeactivateBuff, FinalDuration, false);
}

void UActiveBuffComponent::DeactivateBuff()
{
	if (!bIsBuffActive) return;

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
	bIsBuffActive = false;

	if (Cooldown > 0.f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			CooldownTimerHandle, this, &UActiveBuffComponent::ActivateBuff, Cooldown, false);
	}
	else
	{
		ActivateBuff();
	}
}

void UActiveBuffComponent::ApplyBuff()
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

	// 범위 안 대상은 교체하고 재계산은 대상당 한 번만.
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

void UActiveBuffComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CooldownTimerHandle);
		World->GetTimerManager().ClearTimer(DurationTimerHandle);
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
