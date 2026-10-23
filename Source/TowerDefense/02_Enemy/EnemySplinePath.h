#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"

#include "EnemySplinePath.generated.h"

UCLASS()
class TOWERDEFENSE_API AEnemySplinePath : public AActor
{
	GENERATED_BODY()
	
public:	

	AEnemySplinePath();

public:

	UPROPERTY(VisibleAnywhere)
	USplineComponent* splineComponent;

protected:

	virtual void BeginPlay() override;

public:	

	virtual void Tick(float DeltaTime) override;

};
