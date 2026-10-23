
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridTile.h"
#include "GridManager.generated.h"

UCLASS()
class TOWERDEFENSE_API AGridManager : public AActor
{
	GENERATED_BODY()
	
public:	

	AGridManager();

	UPROPERTY(VisibleAnywhere)
	TArray<AGridTile*> GridTiles;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* PreviewMesh;

	UPROPERTY()
	UMaterialInstanceDynamic* PreviewMaterial;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* PreviewMaterialBase;

public:

	UFUNCTION()
	void EnablePlacementMode();

	UFUNCTION()
	void DisablePlacementMode();

	UFUNCTION()
	void ShowPreview(FVector Location);

	UFUNCTION()
	void HidePreview();

	UFUNCTION()
	void OnTileSelected(AGridTile* Tile);

protected:

	virtual void BeginPlay() override;



};
