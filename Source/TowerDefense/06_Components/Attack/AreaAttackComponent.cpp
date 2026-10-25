
#include "AreaAttackComponent.h"
#include "TowerDefenseTags.h"
#include "Enemy.h"
#include "AirAbilityComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TowerStatComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
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

	// 물리/마법 공격 타입에 따른 업그레이드 보너스 적용.
	const float DamageBonus = GameMode ? GameMode->GetAttackDamageBonus(TowerStat) : 0.f;

	const float BaseDamage = TowerStat->GetStat(TDTags::Stat_AttackDamage);
	const float FinalDamage = FMath::IsFinite(BaseDamage)
		? FMath::Max(0.f, BaseDamage * (1.f + DamageBonus))
		: 0.f;
	// 타워가 공격할 수 있는 대상(공중/지상)만 폭발 피해를 받는다.
	const bool bCanHitAir = TowerStat->Abilities.HasTag(TDTags::Ability_AttackAir);
	const bool bCanHitGround = TowerStat->Abilities.HasTag(TDTags::Ability_AttackGround);
	for (AActor* Actor : OverlappedActors)
	{
		AEnemy* Enemy = Cast<AEnemy>(Actor);
		if (!IsValid(Enemy) || Enemy->bIsDead) continue;

		const bool bIsAir = Enemy->GetComponentByClass<UAirAbilityComponent>() != nullptr;
		if (bIsAir ? !bCanHitAir : !bCanHitGround) continue;

		Enemy->ReceiveDamage(FinalDamage, TowerStat);
	}

	if (OverlappedActors.Num() > 0)
	{
		SpawnAreaEffect(Center);
	}
}

UNiagaraComponent* UAreaAttackComponent::SpawnAreaEffect(const FVector& Center) const
{
	if (!AreaEffect || !GetWorld()) return nullptr;

	UNiagaraComponent* EffectComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), AreaEffect, Center);
	if (!EffectComp) return nullptr;

	// 이펙트에 지정된 Scale_All 기본값에 폭발 반경 비율을 곱해 범위만큼 넓게 펼친다.
	static const FNiagaraVariable ScaleAllVar(FNiagaraTypeDefinition::GetFloatDef(), TEXT("User.Scale_All"));
	const TOptional<float> DefaultScale = AreaEffect->GetExposedParameters().GetParameterOptionalValue<float>(ScaleAllVar);
	if (DefaultScale.IsSet())
	{
		const float Radius = FMath::IsFinite(ExplosionRadius) ? FMath::Max(0.f, ExplosionRadius) : 0.f;
		EffectComp->SetVariableFloat(TEXT("Scale_All"), DefaultScale.GetValue() * Radius / FMath::Max(1.f, AreaEffectBaseRadius));
	}
	return EffectComp;
}
