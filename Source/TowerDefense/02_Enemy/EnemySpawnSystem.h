#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawnSystem.generated.h"

class AEnemySplinePath;
class AEnemy;


UCLASS()
class TOWERDEFENSE_API AEnemySpawnSystem : public AActor
{
	GENERATED_BODY()
	
public:	

	AEnemySpawnSystem();

	//스폰할 클래스 지정
	UPROPERTY(EditAnywhere)
	TSubclassOf<AEnemy> EnemyClass;

	//스폰 위치 좌표
	UPROPERTY(EditAnywhere)
	TObjectPtr<AEnemySplinePath> splinePath;

	//스폰 간격
	UPROPERTY(EditAnywhere)
	float SpawnDelay;

	// 라운드 당 스폰할 수
	UPROPERTY(EditAnywhere)
	int32 SpawnCount;

	UPROPERTY(EditAnywhere)
	int32 CurrentSpawnCount;

protected:

	virtual void BeginPlay() override;

public:	

	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void SpawnEnemy();

	UFUNCTION()
	void StartSpawn();

	UFUNCTION()
	void StopSpawn();

private:

	//타이머 제어에 필요한 핸들
	FTimerHandle SpawnTimerHandle;
};
