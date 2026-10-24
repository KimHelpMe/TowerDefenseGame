#include "EnemySplinePath.h"


AEnemySplinePath::AEnemySplinePath()
{
	PrimaryActorTick.bCanEverTick = false; // Tick does nothing; BP Event Tick re-enables it on compile.

	splineComponent	= CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
	RootComponent = splineComponent;
}


void AEnemySplinePath::BeginPlay()
{
	Super::BeginPlay();
	
}


void AEnemySplinePath::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

