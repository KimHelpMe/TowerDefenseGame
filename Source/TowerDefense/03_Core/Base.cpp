#include "Base.h"
#include "StatComponent.h"

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
	float FinalDamage = AmountDamage;
	if (AttackerStat)
	{
		// TODO : FinalDamage = StatComponent->CalculateDamage(AttackerStat) * AmountDamage;
	}

	BaseHp -= FinalDamage;
	UE_LOG(LogTemp, Warning, TEXT("Base HP: %f"), BaseHp);
	if (BaseHp <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Base Destroyed!"));
		Destroy();
	}
}