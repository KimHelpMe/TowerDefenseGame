
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
	if (Targets.IsEmpty())
	{
		return;
	}

	FVector Center = Targets[0]->GetActorLocation();
	TArray<AActor*> OverlappedActors;

	// Object Types
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	// IgnoreActors
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(GetOwner());

	UTowerStatComponent* TowerStat = GetOwner()->GetComponentByClass<UTowerStatComponent>();
	if (!TowerStat) return;



	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		Center,
		TowerStat->GetStat(UStatComponent::GetTag("Stat.AttackRange")),
		ObjectTypes,
		nullptr,
		IgnoreActors,
		OverlappedActors
	);
		


	ATowerDefenseGameMode* GameMode = Cast<ATowerDefenseGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	float DamageBonus = 0.f;
	if (GameMode && TowerStat)
	{
		if (TowerStat->AttackTypes.HasTag(
			UStatComponent::GetTag("AttackTypes.Physical")))
		{
			DamageBonus = GameMode->GetUpgradeBonus(
				UStatComponent::GetTag("Upgrade.AttackDamage.Physical"));
		}
		
		else if (TowerStat->AttackTypes.HasTag(
			UStatComponent::GetTag("AttackTypes.Magic")))
		{
			DamageBonus = GameMode->GetUpgradeBonus(
				UStatComponent::GetTag("Upgrade.AttackDamage.Magic"));
		}
	}

	for (AActor* Actor : OverlappedActors)
	{
		AEnemy* enemy = Cast<AEnemy>(Actor);
		if (enemy && IsValid(enemy))
		{
			float FinalDamage = TowerStat->GetStat(UStatComponent::GetTag("Stat.AttackDamage")) * (1.f + DamageBonus);
			enemy->ReceiveDamage(FinalDamage, TowerStat);
		}
	}

	if (AreaEffect && OverlappedActors.Num() > 0)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			AreaEffect,
			Center
		);
	}



}


