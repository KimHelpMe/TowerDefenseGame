#include "EnemySplinePath.h"


AEnemySplinePath::AEnemySplinePath()
{
	PrimaryActorTick.bCanEverTick = true;

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

