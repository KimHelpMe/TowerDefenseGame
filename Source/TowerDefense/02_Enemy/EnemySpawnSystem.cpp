#include "EnemySpawnSystem.h"
#include "Enemy.h"
#include "EnemySplinePath.h"


AEnemySpawnSystem::AEnemySpawnSystem()
{
 	
	PrimaryActorTick.bCanEverTick = false; // Tick does nothing; BP Event Tick re-enables it on compile.

	// 기본값 설정
	SpawnDelay = 1.0f;
	SpawnCount = 50;
	CurrentSpawnCount = 0;
}


void AEnemySpawnSystem::BeginPlay()
{
	Super::BeginPlay();


}


void AEnemySpawnSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AEnemySpawnSystem::SpawnEnemy()
{
	// 유효한지 확인하기.
	if (!EnemyClass) return;
	if (!splinePath) return;

	// 스폰 위치, 회전값
	FVector SpawnLocation = splinePath->splineComponent->GetLocationAtDistanceAlongSpline(0, ESplineCoordinateSpace::World);
	FRotator SpawnRotation = splinePath->splineComponent->GetRotationAtDistanceAlongSpline(0, ESplineCoordinateSpace::World);

	//액터 스폰
	AEnemy* spawnedEnemy = GetWorld()->SpawnActor<AEnemy>(EnemyClass, SpawnLocation, SpawnRotation);
	if (spawnedEnemy)
	{
		spawnedEnemy->splinePath = splinePath;
	}

	CurrentSpawnCount += 1;
	if (CurrentSpawnCount >= SpawnCount)
	{
		StopSpawn();
	}
}

void AEnemySpawnSystem::StartSpawn()
{
	// 타이머 설정
	GetWorldTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&AEnemySpawnSystem::SpawnEnemy,
		SpawnDelay,
		true);
}



void AEnemySpawnSystem::StopSpawn()
{
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
}
