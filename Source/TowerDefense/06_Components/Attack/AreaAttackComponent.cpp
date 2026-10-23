
#include "AreaAttackComponent.h"
#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TowerStatComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "TowerDefenseGameMode.h"


UAreaAttackComponent::UAreaAttackComponent()
{
}

void UAreaAttackComponent::ExecuteAttack(TArray<AEnemy*> Targets)
{
	AEnemy* PrimaryTarget = nullptr;
	for (AEnemy* Target : Targets)
	{
		if (IsValid(Target) && !Target->bIsDead)
		{
			PrimaryTarget = Target;
			break;
		}
	}
	if (!IsValid(PrimaryTarget)) return;

	UTowerStatComponent* TowerStat = GetOwner() ? GetOwner()->GetComponentByClass<UTowerStatComponent>() : nullptr;
	if (!IsValid(TowerStat) || !GetWorld()) return;

	ATowerDefenseGameMode* GameMode = Cast<ATowerDefenseGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	const float BaseRadius = TowerStat->GetStat(UStatComponent::GetTag("Stat.AttackRange"));
	const float AttackRangeBonus = GameMode
		? GameMode->GetUpgradeBonus(UStatComponent::GetTag("Upgrade.AttackRange"))
		: 0.f;
	const float SafeAttackRangeBonus = FMath::IsFinite(AttackRangeBonus) ? AttackRangeBonus : 0.f;
	const float AttackRadius = FMath::IsFinite(BaseRadius)
		? FMath::Max(0.f, BaseRadius * (1.f + SafeAttackRangeBonus))
		: 0.f;
	const FVector Center = PrimaryTarget->GetActorLocation();
	TArray<AActor*> OverlappedActors;

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(GetOwner());

	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		Center,
		AttackRadius,
		ObjectTypes,
		nullptr,
		IgnoreActors,
		OverlappedActors);

	float DamageBonus = 0.f;
	if (GameMode)
	{
		if (TowerStat->AttackTypes.HasTag(UStatComponent::GetTag("AttackTypes.Physical")))
		{
			DamageBonus = GameMode->GetUpgradeBonus(UStatComponent::GetTag("Upgrade.AttackDamage.Physical"));
		}
		else if (TowerStat->AttackTypes.HasTag(UStatComponent::GetTag("AttackTypes.Magic")))
		{
			DamageBonus = GameMode->GetUpgradeBonus(UStatComponent::GetTag("Upgrade.AttackDamage.Magic"));
		}
	}
	if (!FMath::IsFinite(DamageBonus)) DamageBonus = 0.f;

	const float BaseDamage = TowerStat->GetStat(UStatComponent::GetTag("Stat.AttackDamage"));
	const float FinalDamage = FMath::IsFinite(BaseDamage)
		? FMath::Max(0.f, BaseDamage * (1.f + DamageBonus))
		: 0.f;
	for (AActor* Actor : OverlappedActors)
	{
		AEnemy* Enemy = Cast<AEnemy>(Actor);
		if (IsValid(Enemy) && !Enemy->bIsDead)
		{
			Enemy->ReceiveDamage(FinalDamage, TowerStat);
		}
	}

	if (AreaEffect && OverlappedActors.Num() > 0)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), AreaEffect, Center);
	}
}
