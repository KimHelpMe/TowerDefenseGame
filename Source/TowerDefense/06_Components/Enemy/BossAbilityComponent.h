
#pragma once

#include "CoreMinimal.h"
#include "EnemyAbilityComponent.h"
#include "BossAbilityComponent.generated.h"

class AEnemy;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TOWERDEFENSE_API UBossAbilityComponent : public UEnemyAbilityComponent
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere)
	int32 CurrentPhase = 1;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AEnemy> MinionClass;

	UPROPERTY(EditAnywhere)
	int32 MinionSpawnCount;


public:

	UFUNCTION()
	void OnPhaseChanged(int32 NewPhase);

	UFUNCTION()
	void SpawnMinions();

	UFUNCTION()
	void ApplyPhaseBuffs(int32 Phase);


};

