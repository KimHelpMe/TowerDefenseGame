
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
	TObjectPtr<UDataTable> RoundDataTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentRound;

	UPROPERTY(EditAnywhere)
	TObjectPtr<AEnemySpawnSystem> SpawnSystem = nullptr;

	UPROPERTY()
	TObjectPtr<ATowerDefenseGameMode> GameMode;

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

	// True when the current round is the last one (MaxRound reached or no next round data).
	UFUNCTION(BlueprintCallable)
	bool IsFinalRound() const;

	// Enemies on the map that are not already dying.
	UFUNCTION(BlueprintCallable)
	int32 GetAliveEnemyCount() const;

	FTimerHandle RoundTimerHandle;

public:

	// Enemy Entry.

	UPROPERTY(EditAnywhere)
	TArray<FRoundEnemyEntry> PendingEntries;

	UPROPERTY(EditAnywhere)
	int32 CurrentEntryIndex;

	bool bRoundSpawnComplete = false;

	FTimerHandle EntryTimerHandle;

	UFUNCTION()
	void ProcessNextEntry();

protected:

	virtual void BeginPlay() override;

public:	

	virtual void Tick(float DeltaTime) override;

};
