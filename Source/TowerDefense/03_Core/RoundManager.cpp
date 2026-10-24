#include "RoundManager.h"
#include "TowerDefenseLog.h"
#include "Kismet/GameplayStatics.h"
#include "EnemySpawnSystem.h"
#include "EnemySplinePath.h"
#include "TowerDefenseGameMode.h"
#include "Enemy.h"

ARoundManager::ARoundManager()
{
	PrimaryActorTick.bCanEverTick = true;
	RoundTimer = 100;
	CurrentTimer = RoundTimer;
	MaxRound = 30;
	CurrentRound = 0;
	CurrentEntryIndex = 0;
	bRoundSpawnComplete = false;
}

void ARoundManager::BeginPlay()
{
	Super::BeginPlay();

	SpawnSystem = Cast<AEnemySpawnSystem>(UGameplayStatics::GetActorOfClass(GetWorld(), AEnemySpawnSystem::StaticClass()));
	GameMode = Cast<ATowerDefenseGameMode>(GetWorld()->GetAuthGameMode());
	if (IsValid(SpawnSystem) && RoundDataTable && IsValid(GameMode))
	{
		GetWorldTimerManager().SetTimer(RoundTimerHandle, this, &ARoundManager::UpdateRoundTick, 1.0f, true);
	}
}

void ARoundManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ARoundManager::UpdateRoundTick()
{
	if (!IsValid(GameMode) || GameMode->bIsGameOver || !RoundDataTable || !IsValid(SpawnSystem))
	{
		StopRounds();
		return;
	}

	CurrentTimer = FMath::Max(0.f, CurrentTimer - 1.f);

	// Final round: victory once it has finished spawning and the map has no enemies left
	// (including enemies left over from earlier, skipped rounds).
	if (CurrentRound > 0 && IsFinalRound())
	{
		if (bRoundSpawnComplete && GetAliveEnemyCount() == 0)
		{
			GetWorldTimerManager().ClearTimer(RoundTimerHandle);
			GameMode->TriggerVictory();
		}
		return;
	}

	if (CurrentTimer > 0.f) return;

	// Rounds advance on timer end or skip even if enemies remain (design: clear several rounds at once).
	// Starting the next round replaces the pending spawn entries, so wait until this round has finished spawning.
	if (CurrentRound > 0)
	{
		if (!bRoundSpawnComplete) return;

		// Only the token reward is given per round; gold and EXP come from each kill.
		const FString CurrentRowKey = FString::Printf(TEXT("Round_%02d"), CurrentRound);
		if (FRoundDataRow* Row = RoundDataTable->FindRow<FRoundDataRow>(FName(*CurrentRowKey), TEXT("")))
		{
			GameMode->AddTowerToken(Row->TowerTokenReward);
		}
	}

	StartRound(CurrentRound + 1);
}

bool ARoundManager::IsFinalRound() const
{
	if (CurrentRound >= MaxRound) return true;
	const FString NextRowKey = FString::Printf(TEXT("Round_%02d"), CurrentRound + 1);
	return !RoundDataTable || !RoundDataTable->FindRow<FRoundDataRow>(FName(*NextRowKey), TEXT(""));
}

int32 ARoundManager::GetAliveEnemyCount() const
{
	TArray<AActor*> Enemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemy::StaticClass(), Enemies);
	return Enemies.FilterByPredicate([](const AActor* Actor)
	{
		const AEnemy* Enemy = Cast<AEnemy>(Actor);
		return IsValid(Enemy) && !Enemy->bIsDead;
	}).Num();
}

void ARoundManager::StopRounds()
{
	GetWorldTimerManager().ClearTimer(RoundTimerHandle);
	GetWorldTimerManager().ClearTimer(EntryTimerHandle);
	if (IsValid(SpawnSystem))
	{
		SpawnSystem->StopSpawn();
	}
	PendingEntries.Empty();
	CurrentEntryIndex = 0;
	bRoundSpawnComplete = true;
}

void ARoundManager::StartRound(int32 RoundNumber)
{
	if (!RoundDataTable || !IsValid(SpawnSystem) || !IsValid(GameMode) || GameMode->bIsGameOver)
	{
		StopRounds();
		return;
	}
	if (RoundNumber > MaxRound)
	{
		GetWorldTimerManager().ClearTimer(RoundTimerHandle);
		return;
	}

	const FString RowKeyString = FString::Printf(TEXT("Round_%02d"), RoundNumber);
	const FName RowKey(*RowKeyString);
	FRoundDataRow* Row = RoundDataTable->FindRow<FRoundDataRow>(RowKey, TEXT(""));
	if (!Row)
	{
		GetWorldTimerManager().ClearTimer(RoundTimerHandle);
		return;
	}

	CurrentRound = RoundNumber;
	CurrentTimer = FMath::Max(0.f, RoundTimer);
	bRoundSpawnComplete = false;
	SpawnEnemiesForRound(Row);
}

void ARoundManager::SpawnEnemiesForRound(FRoundDataRow* RoundData)
{
	if (!RoundData)
	{
		bRoundSpawnComplete = true;
		return;
	}

	PendingEntries = RoundData->EnemyEntries;
	CurrentEntryIndex = 0;
	bRoundSpawnComplete = false;
	ProcessNextEntry();
}

void ARoundManager::ProcessNextEntry()
{
	if (!IsValid(SpawnSystem) || !IsValid(SpawnSystem->splinePath) || !IsValid(SpawnSystem->splinePath->splineComponent))
	{
		UE_LOG(LogTowerDefense, Error, TEXT("Round spawning stopped because the enemy spawn path is invalid."));
		StopRounds();
		return;
	}

	while (PendingEntries.IsValidIndex(CurrentEntryIndex))
	{
		const FRoundEnemyEntry& Entry = PendingEntries[CurrentEntryIndex];
		if (!Entry.EnemyClass || Entry.SpawnCount <= 0)
		{
			CurrentEntryIndex++;
			continue;
		}
		break;
	}

	if (!PendingEntries.IsValidIndex(CurrentEntryIndex))
	{
		bRoundSpawnComplete = true;
		return;
	}

	FRoundEnemyEntry& Entry = PendingEntries[CurrentEntryIndex];
	SpawnSystem->EnemyClass = Entry.EnemyClass;
	SpawnSystem->SpawnCount = Entry.SpawnCount;
	SpawnSystem->SpawnDelay = FMath::Max(0.01f, Entry.SpawnDelay);
	SpawnSystem->CurrentSpawnCount = 0;
	SpawnSystem->StartSpawn();

	const float WaitTime = Entry.SpawnCount * SpawnSystem->SpawnDelay + 0.05f;
	GetWorldTimerManager().SetTimer(EntryTimerHandle, [this]()
	{
		CurrentEntryIndex++;
		ProcessNextEntry();
	}, WaitTime, false);
}
