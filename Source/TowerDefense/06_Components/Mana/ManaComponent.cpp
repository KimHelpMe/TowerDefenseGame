#include "ManaComponent.h"
#include "TowerDefenseGameMode.h"
#include "StatComponent.h"
#include "Kismet/GameplayStatics.h"

UManaComponent::UManaComponent()
{
	CurrentMana = 0.f;
	MaxMana = 100.f;
	ManaRegenRate = 10.f;
	ManaRegenInterval = 1.f;
	ManaCost = 100.f;

}

void UManaComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!FMath::IsFinite(MaxMana) || MaxMana <= 0.f) MaxMana = 100.f;
	if (!FMath::IsFinite(CurrentMana)) CurrentMana = 0.f;
	CurrentMana = FMath::Clamp(CurrentMana, 0.f, MaxMana);
	if (!FMath::IsFinite(ManaRegenRate)) ManaRegenRate = 0.f;
	ManaRegenRate = FMath::Max(0.f, ManaRegenRate);
	if (!FMath::IsFinite(ManaCost)) ManaCost = MaxMana;
	ManaCost = FMath::Clamp(ManaCost, 0.f, MaxMana);
	if (!FMath::IsFinite(ManaRegenInterval) || ManaRegenInterval <= 0.f) ManaRegenInterval = 1.f;

	ATowerDefenseGameMode* GameMode = Cast<ATowerDefenseGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	const float ManaRegenBonus = GameMode ? GameMode->GetUpgradeBonus(
		UStatComponent::GetTag("Upgrade.ManaRegen")) : 0.f;
	const float SafeManaRegenBonus = FMath::IsFinite(ManaRegenBonus) ? ManaRegenBonus : 0.f;
	const float FinalManaRegenInterval = FMath::Max(0.01f, ManaRegenInterval * (1.f - SafeManaRegenBonus));

	GetWorld()->GetTimerManager().SetTimer(
		ManaRegenTimerHandle,
		this,
		&UManaComponent::RegenMana,
		FinalManaRegenInterval,
		true);
}

void UManaComponent::RegenMana()
{
	if (!FMath::IsFinite(MaxMana) || MaxMana <= 0.f) return;
	if (!FMath::IsFinite(CurrentMana)) CurrentMana = 0.f;

	CurrentMana = FMath::Clamp(CurrentMana + ManaRegenRate, 0.f, MaxMana);
	UE_LOG(LogTemp, Warning, TEXT("CurrentMana: %f / %f"), CurrentMana, MaxMana);
	OnManaChanged.Broadcast(CurrentMana / MaxMana);

	if (CurrentMana >= MaxMana && bAutoFire)
	{
		OnManaFull.Broadcast();
	}
}

void UManaComponent::ConsumeMana()
{
	if (!FMath::IsFinite(MaxMana) || MaxMana <= 0.f) return;

	CurrentMana = FMath::Clamp(CurrentMana - ManaCost, 0.f, MaxMana);
	OnManaChanged.Broadcast(CurrentMana / MaxMana);
}
