#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridTile.generated.h"

class AGridManager;


UCLASS()
class TOWERDEFENSE_API AGridTile : public AActor
{
	GENERATED_BODY()
	
public:	

	AGridTile();


	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* TileMesh;

	UPROPERTY()
	UMaterialInstanceDynamic* TileMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* OutlineMesh;

	UPROPERTY(VisibleAnywhere)
	AGridManager* GridManager;

public:


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOccupied;

	UFUNCTION()
	void SetHighlight(bool bOn);

	UFUNCTION()
	void OnTileClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

	UFUNCTION()
	void OnTileBeginCursorOver(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
	void OnTileEndCursorOver(UPrimitiveComponent* TouchedComponent);


protected:

	virtual void BeginPlay() override;



};
