
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoundDataRow.h"
#include "RoundManager.generated.h"

class AEnemySpawnSystem;
class ATowerDefenseGameMode;

UCLASS()
class TOWERDEFENSE_API ARoundManager : public AActor
{
	GENERATED_BODY()
	
public:	

	ARoundManager();


public:

	UPROPERTY(EditAnywhere)
	UDataTable* RoundDataTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentRound;

	UPROPERTY(EditAnywhere)
	AEnemySpawnSystem* SpawnSystem = nullptr;

	UPROPERTY()
	ATowerDefenseGameMode* GameMode;

public:

	UFUNCTION()
	void StartRound(int32 RoundNumber);

	UFUNCTION(BlueprintCallable)
	void StopRounds();

	// Finish Round -> Get Reward.

	void SpawnEnemiesForRound(FRoundDataRow* RoundData);

public:

	// Round Timer.

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RoundTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxRound;

    UFUNCTION()
	void UpdateRoundTick();

	FTimerHandle RoundTimerHandle;

public:

	// Enemy Entry.

	UPROPERTY(EditAnywhere)
	TArray<FRoundEnemyEntry> PendingEntries;

	UPROPERTY(EditAnywhere)
	int32 CurrentEntryIndex;

	FTimerHandle EntryTimerHandle;

	UFUNCTION()
	void ProcessNextEntry();

protected:

	virtual void BeginPlay() override;

public:	

	virtual void Tick(float DeltaTime) override;

};
