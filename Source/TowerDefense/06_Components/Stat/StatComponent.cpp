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

	const int32 Removed = ActiveBuffs.RemoveAll([SourceKey](const FBuffEffect& Buff)
		{
			return Buff.SourceKey == SourceKey;
		});

	// 지울 버프가 없었으면 재계산(사거리 Sphere 갱신 등)을 생략.
	if (Removed > 0)
	{
		RecalculateStats();
	}
}

void UStatComponent::ReplaceBuffsByKey(FName SourceKey, const TArray<FBuffEffect>& NewBuffs)
{
	if (SourceKey.IsNone()) return;

	// 이미 같은 버프가 걸려 있으면 아무것도 하지 않는다(주기적 재적용 시 대부분 이 경우).
	TArray<const FBuffEffect*> Existing;
	for (const FBuffEffect& Buff : ActiveBuffs)
	{
		if (Buff.SourceKey == SourceKey) Existing.Add(&Buff);
	}
	bool bSame = Existing.Num() == NewBuffs.Num();
	for (int32 Index = 0; bSame && Index < NewBuffs.Num(); ++Index)
	{
		const FBuffEffect& A = *Existing[Index];
		const FBuffEffect& B = NewBuffs[Index];
		bSame = A.StatTag == B.StatTag && A.ValueType == B.ValueType && A.Source == B.Source
			&& FMath::IsNearlyEqual(A.Value, B.Value);
	}
	if (bSame) return;

	// 바뀐 경우에만 교체하고 재계산은 한 번만 한다.
	ActiveBuffs.RemoveAll([SourceKey](const FBuffEffect& Buff)
		{
			return Buff.SourceKey == SourceKey;
		});
	for (FBuffEffect Buff : NewBuffs)
	{
		Buff.SourceKey = SourceKey;
		ActiveBuffs.Add(Buff);
	}
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
