
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
	UWorld* World = GetWorld();
	if (!World || !IsValid(GetOwner())) return;

	bIsBuffActive = true;
	ApplyBuff();

	ATowerDefenseGameMode* GameMode = Cast<ATowerDefenseGameMode>(
		UGameplayStatics::GetGameMode(World));
	const float DurationBonus = GameMode ? GameMode->GetUpgradeBonus(
		FGameplayTag::RequestGameplayTag("Upgrade.BuffDuration")) : 0.f;
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
