
#include "AreaAttackComponent.h"
#include "Enemy.h"
#include "AirAbilityComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TowerStatComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "TowerDefenseGameMode.h"


UAreaAttackComponent::UAreaAttackComponent()
{
}

void UAreaAttackComponent::ExecuteAttack(const TArray<AEnemy*>& Targets)
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
	// 폭발 반경은 공격 사거리와 별도 값을 사용(사거리 전체를 덮지 않도록).
	const float AttackRadius = FMath::IsFinite(ExplosionRadius) ? FMath::Max(0.f, ExplosionRadius) : 0.f;
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
	// 타워가 공격할 수 있는 대상(공중/지상)만 폭발 피해를 받는다.
	const bool bCanHitAir = TowerStat->Abilities.HasTag(UStatComponent::GetTag("Ability.AttackAir"));
	const bool bCanHitGround = TowerStat->Abilities.HasTag(UStatComponent::GetTag("Ability.AttackGround"));
	for (AActor* Actor : OverlappedActors)
	{
		AEnemy* Enemy = Cast<AEnemy>(Actor);
		if (!IsValid(Enemy) || Enemy->bIsDead) continue;

		const bool bIsAir = Enemy->GetComponentByClass<UAirAbilityComponent>() != nullptr;
		if (bIsAir ? !bCanHitAir : !bCanHitGround) continue;

		Enemy->ReceiveDamage(FinalDamage, TowerStat);
	}

	if (AreaEffect && OverlappedActors.Num() > 0)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), AreaEffect, Center);
	}
}
