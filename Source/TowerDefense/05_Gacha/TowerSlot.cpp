#include "TowerSlot.h"
#include "BaseTower.h"
#include "GachaManager.h"
#include "Kismet/GameplayStatics.h"

ATowerSlot::ATowerSlot()
{
	PrimaryActorTick.bCanEverTick = false; // Tick does nothing; BP Event Tick re-enables it on compile.

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	TowerSlotMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TowerSlotComponent"));
	TowerSlotMesh->SetupAttachment(RootComponent);

	bIsOccupied = false;
	ReloadCount = 2;
	bCanAbandon = false;
	InstalledTower = nullptr;

}

void ATowerSlot::BeginPlay()
{
	Super::BeginPlay();
	
	TowerSlotMesh->OnClicked.AddDynamic(this, &ATowerSlot::OnClickedSelect);
	GachaManager = Cast<AGachaManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGachaManager::StaticClass()));

}


void ATowerSlot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATowerSlot::OnClickedSelect(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
	if (!GachaManager) return;
	if (bIsOccupied)
    {
        if (IsValid(InstalledTower)) InstalledTower->OnTowerClicked(TouchedComponent, ButtonPressed);
        return;
    }
	GachaManager->OpenGacha(this);
}

void ATowerSlot::ResetGachaState()
{
	const ATowerSlot* Defaults = GetClass()->GetDefaultObject<ATowerSlot>();
	SavedRollResult.Reset();
	ReloadCount = Defaults->ReloadCount;
	bCanAbandon = Defaults->bCanAbandon;
}

