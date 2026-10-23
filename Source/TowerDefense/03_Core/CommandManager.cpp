

#include "CommandManager.h"
#include "Base.h"
#include "BossEnemy.h"
#include "Enemy.h"
#include "Kismet/GameplayStatics.h"


ACommandManager::ACommandManager()
{
 
	PrimaryActorTick.bCanEverTick = true;

	HealCooldown = 300.f;
	HealCooldownRemaining = 0.f;

	PurgeCooldown = 600.f;
	PurgeCooldownRemaining = 0.f;

	CurrentSpeedIndex = 0;

}

void ACommandManager::BeginPlay()
{
	Super::BeginPlay();

	Base = Cast<ABase>(UGameplayStatics::GetActorOfClass(GetWorld(), ABase::StaticClass()));

}


void ACommandManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TickCooldowns(DeltaTime);
}



bool ACommandManager::CanHeal()
{
	if (HealCooldownRemaining <= 0) return true;
	else return false;
}

bool ACommandManager::CanPurge()
{
	if (PurgeCooldownRemaining <= 0) return true;
	else return false;
}

void ACommandManager::HealBase()
{
	if (CanHeal() == false) return;
	if (!Base) return;

	float HealAmount = Base->BaseMaxHp * 0.3f;
	Base->BaseHp = FMath::Min(Base->BaseHp + HealAmount, Base->BaseMaxHp);

	HealCooldownRemaining = HealCooldown;
}

void ACommandManager::PurgeAllEnemies()
{

	if (CanPurge() == false) return;
	
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemy::StaticClass(), OutActors);

	for (AActor* Actor : OutActors)
	{
		if (Cast<ABossEnemy>(Actor)) continue;

		AEnemy* Enemy = Cast<AEnemy>(Actor);
		if (Enemy) Enemy->OnEnemyDeath();

	}
	PurgeCooldownRemaining = PurgeCooldown;
}

void ACommandManager::SetGameSpeed(int32 SpeedIndex)
{
	TArray<float> SpeedOptions = { 1.f, 2.f, 3.f };

	if (SpeedOptions.IsValidIndex(SpeedIndex))
	{
		CurrentSpeedIndex = SpeedIndex;
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), SpeedOptions[SpeedIndex]);
	}

}



void ACommandManager::TickCooldowns(float DeltaTime)
{
	if (HealCooldownRemaining > 0)
	{
		HealCooldownRemaining = FMath::Max(0.f, HealCooldownRemaining - DeltaTime);
	}

	if (PurgeCooldownRemaining > 0)
	{
		PurgeCooldownRemaining = FMath::Max(0.f, PurgeCooldownRemaining - DeltaTime);
	}
	OnCooldownUpdated.Broadcast(HealCooldownRemaining, PurgeCooldownRemaining);
}
