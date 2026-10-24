#include "AttackTower.h"
#include "TowerDefenseTags.h"
#include "TowerStatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TowerDefenseGameMode.h"

AAttackTower::AAttackTower()
{
	PrimaryActorTick.bCanEverTick = true;
}


void AAttackTower::BeginPlay()
{
	Super::BeginPlay();

	RefreshAttackTimer();
}

void AAttackTower::RefreshAttackTimer()
{
	if (!GetWorld()) return;

	UTowerStatComponent* TowerStat = GetComponentByClass<UTowerStatComponent>();
	ATowerDefenseGameMode* GameMode = Cast<ATowerDefenseGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	const float BaseAttackSpeed = TowerStat ? TowerStat->GetStat(TDTags::Stat_AttackSpeed) : 1.f;
	const float AttackSpeedBonus = GameMode ? GameMode->GetUpgradeBonus(TDTags::Upgrade_AttackSpeed) : 0.f;
	const float SafeAttackSpeedBonus = FMath::IsFinite(AttackSpeedBonus) ? AttackSpeedBonus : 0.f;
	const float FinalAttackSpeed = FMath::IsFinite(BaseAttackSpeed)
		? FMath::Max(0.01f, BaseAttackSpeed * (1.f - SafeAttackSpeedBonus))
		: 1.f;

	// 간격이 그대로면 타이머를 다시 걸지 않는다(버프 재적용마다 공격 주기가 리셋되는 것 방지).
	if (GetWorldTimerManager().IsTimerActive(AttackTimerHandle)
		&& FMath::IsNearlyEqual(AppliedAttackInterval, FinalAttackSpeed))
	{
		return;
	}
	AppliedAttackInterval = FinalAttackSpeed;

	GetWorldTimerManager().SetTimer(
		AttackTimerHandle,
		this,
		&AAttackTower::HandleAttackTimer,
		FinalAttackSpeed,
		true);
}

void AAttackTower::HandleAttackTimer()
{
	// BlueprintNativeEvent 진입점으로 호출해야 BP에서 Attack을 override했을 때도 그 구현이 실행된다.
	Attack();
}
