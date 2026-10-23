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

	// ManaRegenInterval을 보너스에 따라 줄임.
	ATowerDefenseGameMode* GameMode = Cast<ATowerDefenseGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	float ManaRegenBonus = GameMode ? GameMode->GetUpgradeBonus(
		UStatComponent::GetTag("Upgrade.ManaRegen")) : 0.f;

	float FinalManaRegenInterval = ManaRegenInterval * (1.f - ManaRegenBonus);

	GetWorld()->GetTimerManager().SetTimer(ManaRegenTimerHandle, this, &UManaComponent::RegenMana, FinalManaRegenInterval, true);
}

void UManaComponent::RegenMana()
{
	CurrentMana += ManaRegenRate;
	CurrentMana = FMath::Clamp(CurrentMana, 0.f, MaxMana);
	UE_LOG(LogTemp, Warning, TEXT("CurrentMana: %f / %f"), CurrentMana, MaxMana);

	// 마나 변경 시 항상 broadcast
	OnManaChanged.Broadcast(CurrentMana / MaxMana);

	if (CurrentMana >= MaxMana)
	{
		if (bAutoFire)  // == true 생략 가능
		{
			OnManaFull.Broadcast();
		}
		// bAutoFire = false면 아무것도 안 함 (UI는 OnManaChanged로 이미 업데이트됨)
	}
}

void UManaComponent::ConsumeMana()
{
	CurrentMana -= ManaCost;
	CurrentMana = FMath::Clamp(CurrentMana, 0.f, MaxMana);
	OnManaChanged.Broadcast(CurrentMana / MaxMana);
}