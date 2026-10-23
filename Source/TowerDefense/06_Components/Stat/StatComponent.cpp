#include "StatComponent.h"
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


	float CurrentHP = GetStat(GetTag("Stat.HP"));
	float MaxHP = GetStat(GetTag("Stat.MaxHP"));

	if (MaxHP > 0 && CurrentHP > MaxHP)
	{
		CurrentHP = MaxHP;
		BaseStats.Add(GetTag("Stat.HP"), MaxHP);
	}

}

// 데미지
void UStatComponent::ApplyDamage(float Amount)
{
	
	float CurrentHP = GetStat(GetTag("Stat.HP"));
	UE_LOG(LogTemp, Warning, TEXT("ApplyDamage: CurrentHP=%f, Amount=%f"), CurrentHP, Amount);
	float NewHP = CurrentHP - Amount;
	
	if (NewHP <= 0)
	{
		// 사망
		OnDeath.Broadcast();
		return;
	}

	// 살아 있으면 HP 저장.
	SetBaseStat(GetTag("Stat.HP"), NewHP);
}