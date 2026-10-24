

#include "GridManager.h"
#include "TowerDefenseLog.h"
#include "Kismet/GameplayStatics.h"

AGridManager::AGridManager()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	PreviewMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMesh"));
	PreviewMesh->SetupAttachment(RootComponent);
	PreviewMesh->SetVisibility(false);
	PreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


void AGridManager::BeginPlay()
{
	Super::BeginPlay();
	
	// 레벨의 모든 AGridTile 수집.
	TArray<AActor*> FoundTiles;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGridTile::StaticClass(), FoundTiles);

	for (AActor* Actor : FoundTiles)
	{
		AGridTile* Tile = Cast<AGridTile>(Actor);
		if (Tile) GridTiles.Add(Tile);
	}

	// PreviewMaterial Dynamic Instance create.
	if (PreviewMaterialBase)
	{
		UE_LOG(LogTowerDefense, Verbose, TEXT("PreviewMaterialBase 존재"));
		PreviewMaterial = UMaterialInstanceDynamic::Create(PreviewMaterialBase, this);
		PreviewMesh->SetMaterial(0, PreviewMaterial);
	}
	else
	{
		UE_LOG(LogTowerDefense, Warning, TEXT("PreviewMaterialBase NULL"));
	}

}

void AGridManager::EnablePlacementMode()
{
	for (AGridTile* Tile : GridTiles)
	{
		if (Tile) Tile->SetHighlight(true);
	}

}


void AGridManager::DisablePlacementMode()
{
	for (AGridTile* Tile : GridTiles)
	{
		if (Tile) Tile->SetHighlight(false);
	}
	HidePreview();
}

void AGridManager::ShowPreview(FVector Location)
{
	if (!PreviewMesh) return;
	PreviewMesh->SetWorldLocation(Location);
	PreviewMesh->SetVisibility(true);
}

void AGridManager::HidePreview()
{
	if (!PreviewMesh) return;
	PreviewMesh->SetVisibility(false);
}

void AGridManager::OnTileSelected(AGridTile* Tile)
{
	if (!Tile) return;
	HidePreview();
	DisablePlacementMode();
}