#include "RoundManager.h"
#include "Kismet/GameplayStatics.h"
#include "EnemySpawnSystem.h"
#include "TowerDefenseGameMode.h"

ARoundManager::ARoundManager()
{
	PrimaryActorTick.bCanEverTick = true;
	RoundTimer = 100;
	CurrentTimer = RoundTimer;
	MaxRound = 30;
	CurrentRound = 0;
}



void ARoundManager::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnSystem = Cast<AEnemySpawnSystem>(UGameplayStatics::GetActorOfClass(GetWorld(), AEnemySpawnSystem::StaticClass()));
	GameMode = Cast<ATowerDefenseGameMode>(GetWorld()->GetAuthGameMode());
	if (SpawnSystem)
	{
		// 타이머 설정. 매초 UpdateRoundTick을 불러옴.
		GetWorldTimerManager().SetTimer(RoundTimerHandle, this, &ARoundManager::UpdateRoundTick, 1.0f, true);
	}
}

void ARoundManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}



void ARoundManager::UpdateRoundTick()
{
	CurrentTimer -= 1.f;

	if (CurrentTimer <= 0)
	{

		if (CurrentRound < MaxRound)
		{
			FString RowKeyString = FString::Printf(TEXT("Round_%02d"), CurrentRound);
			FRoundDataRow* Row = RoundDataTable->FindRow<FRoundDataRow>(FName(*RowKeyString), TEXT(""));
			if (Row && GameMode)
			{
				GameMode->Gold += Row->GoldReward;
				GameMode->AddTowerToken(Row->TowerTokenReward);
				GameMode->AddExp(Row->ExpReward);
			}

		}

		
		StartRound(CurrentRound + 1);
	}
}

void ARoundManager::StartRound(int32 RoundNumber)
{
	if (!RoundDataTable) return;

	if (RoundNumber > MaxRound)
	{
		return;
	}

	CurrentRound += 1;

	FString RowKeyString = FString::Printf(TEXT("Round_%02d"), RoundNumber);

	FName RowKey = FName(*RowKeyString);
	FRoundDataRow* Row = RoundDataTable->FindRow<FRoundDataRow>(RowKey, TEXT(""));
	CurrentTimer = RoundTimer;
	if (!Row) return;
	SpawnEnemiesForRound(Row);
}


void ARoundManager::SpawnEnemiesForRound(FRoundDataRow* RoundData)
{
	PendingEntries = RoundData->EnemyEntries;
	CurrentEntryIndex = 0;
	ProcessNextEntry();
}



void ARoundManager::ProcessNextEntry()
{
	if (PendingEntries.IsEmpty() || CurrentEntryIndex >= PendingEntries.Num()) return;

	// Take Entry.
	FRoundEnemyEntry& Entry = PendingEntries[CurrentEntryIndex];
	
	// SpawnSystem Setting.
	if (!SpawnSystem) return;
	SpawnSystem->EnemyClass = Entry.EnemyClass;
	SpawnSystem->SpawnCount = Entry.SpawnCount;
	SpawnSystem->SpawnDelay = Entry.SpawnDelay;
	SpawnSystem->CurrentSpawnCount = 0;
	SpawnSystem->StartSpawn();

	// Entry Timer Setting.
	float WaitTime = Entry.SpawnCount * Entry.SpawnDelay;
	GetWorldTimerManager().SetTimer(EntryTimerHandle, [this]()
		{
			CurrentEntryIndex++;
			ProcessNextEntry();
		}, WaitTime, false);
}


