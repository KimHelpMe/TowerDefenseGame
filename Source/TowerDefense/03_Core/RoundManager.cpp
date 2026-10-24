#include "RoundManager.h"
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
	if (CurrentTimer > 0.f) return;

	if (CurrentRound > 0)
	{
		if (!bRoundSpawnComplete) return;

		TArray<AActor*> RemainingEnemies;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemy::StaticClass(), RemainingEnemies);
		if (!RemainingEnemies.IsEmpty()) return;

		const FString CurrentRowKey = FString::Printf(TEXT("Round_%02d"), CurrentRound);
		if (FRoundDataRow* Row = RoundDataTable->FindRow<FRoundDataRow>(FName(*CurrentRowKey), TEXT("")))
		{
			GameMode->Gold += Row->GoldReward;
			GameMode->AddTowerToken(Row->TowerTokenReward);
			GameMode->AddExp(Row->ExpReward);
		}

		// Last round cleared (MaxRound reached or no more round data): victory.
		const FString NextRowKey = FString::Printf(TEXT("Round_%02d"), CurrentRound + 1);
		if (CurrentRound >= MaxRound || !RoundDataTable->FindRow<FRoundDataRow>(FName(*NextRowKey), TEXT("")))
		{
			GetWorldTimerManager().ClearTimer(RoundTimerHandle);
			GameMode->TriggerVictory();
			return;
		}
	}

	StartRound(CurrentRound + 1);
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
		UE_LOG(LogTemp, Error, TEXT("Round spawning stopped because the enemy spawn path is invalid."));
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
