

#include "GridTile.h"
#include "TowerDefenseLog.h"
#include "GridManager.h"
#include "TowerDefenseGameMode.h"
#include "Kismet/GameplayStatics.h"


AGridTile::AGridTile()
{

	PrimaryActorTick.bCanEverTick = false;

	TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
	RootComponent = TileMesh;

	// Outline.
	OutlineMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OutlineMesh"));
	OutlineMesh->SetupAttachment(RootComponent);
	OutlineMesh->SetVisibility(false);
	OutlineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

void AGridTile::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTowerDefense, Verbose, TEXT("GridTile BeginPlay 호출"));

	GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));

	if (TileMesh)
	{
		UE_LOG(LogTowerDefense, Verbose, TEXT("TileMesh 존재, 바인딩 시작"));
		TileMesh->OnClicked.AddDynamic(this, &AGridTile::OnTileClicked);
		TileMesh->OnBeginCursorOver.AddDynamic(this, &AGridTile::OnTileBeginCursorOver);
		TileMesh->OnEndCursorOver.AddDynamic(this, &AGridTile::OnTileEndCursorOver);
	}
	else
	{
		UE_LOG(LogTowerDefense, Warning, TEXT("TileMesh NULL"));
	}

	TileMaterial = UMaterialInstanceDynamic::Create(TileMesh->GetMaterial(0), this);
	TileMesh->SetMaterial(0, TileMaterial);
}


void AGridTile::SetHighlight(bool bOn)
{
	if (!TileMaterial) return;

	if (bOn)
	{
		UE_LOG(LogTowerDefense, Verbose, TEXT("SetHighlight ON"));
		TileMaterial->SetScalarParameterValue(TEXT("EmissiveStrength"), 1.0f);
		TileMaterial->SetScalarParameterValue(TEXT("OutlineStrength"), 1.0f);
		OutlineMesh->SetVisibility(true);

		if (bIsOccupied)
			TileMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(1.0f, 0.0f, 0.0f));
		else
			TileMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(0.0f, 1.0f, 0.0f));
	}
	else
	{
		UE_LOG(LogTowerDefense, Verbose, TEXT("SetHighlight OFF"));
		TileMaterial->SetScalarParameterValue(TEXT("EmissiveStrength"), 0.0f);
		TileMaterial->SetScalarParameterValue(TEXT("OutlineStrength"), 0.0f);
		OutlineMesh->SetVisibility(false);
	}
}

void AGridTile::OnTileClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
	if (bIsOccupied) return;

	ATowerDefenseGameMode* GameMode = Cast<ATowerDefenseGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!GameMode) return;
	if (!GameMode->bIsPlacementMode) return;

	if (GameMode->SpawnTowerSlot(GetActorLocation()))
	{
		bIsOccupied = true;
		SetHighlight(false);
	}

}

void AGridTile::OnTileBeginCursorOver(UPrimitiveComponent* TouchedComponent)
{
	ATowerDefenseGameMode* GameMode = Cast<ATowerDefenseGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!GameMode || !GameMode->bIsPlacementMode) return;
	if (!GridManager) return;
	GridManager->ShowPreview(GetActorLocation());
}

void AGridTile::OnTileEndCursorOver(UPrimitiveComponent* TouchedComponent)
{
	if (!GridManager) return;
	GridManager->HidePreview();
}
