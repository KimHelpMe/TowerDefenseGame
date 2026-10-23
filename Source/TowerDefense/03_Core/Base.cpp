#include "Base.h"
#include "StatComponent.h"
#include "TowerDefenseGameMode.h"

ABase::ABase()
{
 	
	PrimaryActorTick.bCanEverTick = true;

	BaseHp = 100.f;
	BaseMaxHp = BaseHp;

	BaseMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMeshCompoent"));
	RootComponent = BaseMeshComponent;

}


void ABase::BeginPlay()
{
	Super::BeginPlay();
	
}


void ABase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABase::ReceiveDamage(float AmountDamage, UStatComponent* AttackerStat)
{
	if (bIsDefeated) return;

	float FinalDamage = AmountDamage;
	if (AttackerStat)
	{
		// TODO : FinalDamage = StatComponent->CalculateDamage(AttackerStat) * AmountDamage;
	}

	BaseHp = FMath::Clamp(BaseHp - FMath::Max(0.f, FinalDamage), 0.f, FMath::Max(0.f, BaseMaxHp));
	UE_LOG(LogTemp, Warning, TEXT("Base HP: %f"), BaseHp);
	if (BaseHp <= 0.f)
	{
		bIsDefeated = true;
		UE_LOG(LogTemp, Warning, TEXT("Base Destroyed!"));
		if (ATowerDefenseGameMode* GameMode = Cast<ATowerDefenseGameMode>(GetWorld()->GetAuthGameMode()))
		{
			GameMode->TriggerGameOver();
		}
		Destroy();
	}
}