#include "StatComponent.h"
#include "TowerDefenseLog.h"
#include "FBuffEffect.h"

UStatComponent::UStatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UStatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	//RecalculateStats(); // BaseStats → CurrentStats 복사
}


// Stat
float UStatComponent::GetStat(FGameplayTag StatTag) const
{
	const float* Value = CurrentStats.Find(StatTag);
	return Value ? *Value : 0.0f;
}

float UStatComponent::GetBaseStat(FGameplayTag StatTag) const
{
	const float* Value = BaseStats.Find(StatTag);
	return Value ? *Value : 0.0f;
}

void UStatComponent::SetBaseStat(FGameplayTag StatTag, float Value)
{
	BaseStats.Add(StatTag, Value);
	RecalculateStats();
}


void UStatComponent::ModifyStat(FBuffEffect BuffEffect)
{
	ActiveBuffs.Add(BuffEffect);
	RecalculateStats();
}

void UStatComponent::RemoveBuff(AActor* Source)
{
	ActiveBuffs.RemoveAll([Source](const FBuffEffect& Buff)
		{
			return Buff.Source == Source;
		});

	RecalculateStats();
}

void UStatComponent::RemoveBuffByKey(FName SourceKey)
{
	if (SourceKey.IsNone()) return;

	ActiveBuffs.RemoveAll([SourceKey](const FBuffEffect& Buff)
		{
			return Buff.SourceKey == SourceKey;
		});

	RecalculateStats();
}

void UStatComponent::RecalculateStats()
{
	CurrentStats = BaseStats;

	// 1. Flat만 순회.
	for (const FBuffEffect& Buff : ActiveBuffs)
	{
		
		if (Buff.ValueType == EBuffValueType::Flat)
		{
			float* Current = CurrentStats.Find(Buff.StatTag);
		
			if (Current)
			{
				*Current += Buff.Value;
			}
		}
	}

	// 2. Percent만 순회.
	for (const FBuffEffect& Buff : ActiveBuffs)
	{
		if(Buff.ValueType == EBuffValueType::Percent)
		{
			float* Current = CurrentStats.Find(Buff.StatTag);

			if (Current)
			{
				*Current += (*Current) * Buff.Value;
			}
		}
	}


	const FGameplayTag HPTag = GetTag("Stat.HP");
	const float CurrentHP = GetStat(HPTag);
	const float MaxHP = GetStat(GetTag("Stat.MaxHP"));

	if (FMath::IsFinite(MaxHP) && MaxHP > 0.f && CurrentHP > MaxHP)
	{
		BaseStats.Add(HPTag, MaxHP);
		CurrentStats.Add(HPTag, MaxHP);
	}

}

// 데미지
void UStatComponent::ApplyDamage(float Amount)
{
	if (!FMath::IsFinite(Amount) || Amount <= 0.f) return;

	const FGameplayTag HPTag = GetTag("Stat.HP");
	const float CurrentHP = GetStat(HPTag);
	if (!FMath::IsFinite(CurrentHP) || CurrentHP <= 0.f) return;

	const float NewHP = FMath::Max(0.f, CurrentHP - Amount);
	UE_LOG(LogTowerDefense, Verbose, TEXT("ApplyDamage: CurrentHP=%f, Amount=%f"), CurrentHP, Amount);

	if (NewHP <= 0.f)
	{
		BaseStats.Add(HPTag, 0.f);
		CurrentStats.Add(HPTag, 0.f);
		OnDeath.Broadcast();
		return;
	}

	SetBaseStat(HPTag, NewHP);
}
