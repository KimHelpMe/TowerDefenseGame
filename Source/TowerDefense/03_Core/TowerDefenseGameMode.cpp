#include "TowerDefenseGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "TowerSlot.h"


ATowerDefenseGameMode::ATowerDefenseGameMode()
{
	PrimaryActorTick.bCanEverTick = true;



	// 재화(TowerToken, Gold)
	TowerToken = 3.f;
	Gold = 500.f;
	Exp = 0;
	MaxExp = 0.f;
	PlayerLevel = 1;


	// 타워 슬롯(Tower Slot)
	MaxTowerSlot = 30;
	CurrentTowerSlot = 0;
	bIsPlacementMode = false;
}


void ATowerDefenseGameMode::BeginPlay()
{
	Super::BeginPlay();

	RoundManager = Cast<ARoundManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ARoundManager::StaticClass()));

	GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
	
	CommandManager = Cast<ACommandManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ACommandManager::StaticClass()));
	
	if (LevelDataTable)
	{
		FLevelDataRow* Row = LevelDataTable->FindRow<FLevelDataRow>(TEXT("Level_01"), TEXT(""));
		if (Row) MaxExp = Row->MaxExp;
	}

	OnExpChanged.Broadcast(PlayerLevel, Exp, MaxExp);
}

void ATowerDefenseGameMode::AddExp(float Amount)
{
	Exp += Amount;
	if (!LevelDataTable) return;

	FString RowKeyString = FString::Printf(TEXT("Level_%02d"), PlayerLevel);
	FLevelDataRow* Row = LevelDataTable->FindRow<FLevelDataRow>(FName(*RowKeyString), TEXT(""));

	if (!Row) return;
	if (Exp >= Row->MaxExp)
	{
		PlayerLevel++;
		Exp = 0;
		AddTowerToken(Row->TokenReward);
		FString NextRowKey = FString::Printf(TEXT("Level_%02d"), PlayerLevel);
		FLevelDataRow* NextRow = LevelDataTable->FindRow<FLevelDataRow>(FName(*NextRowKey), TEXT(""));
		if (NextRow) MaxExp = NextRow->MaxExp;
		OnExpChanged.Broadcast(PlayerLevel, Exp, MaxExp);  // 레벨업 후 새 MaxExp
	}
	else
	{
		OnExpChanged.Broadcast(PlayerLevel, Exp, Row->MaxExp);  // 일반 경험치 증가
	}
}



// 라운드 스킵(Skip) 관련
bool ATowerDefenseGameMode::CanSkip()
{
	if (!RoundManager) return false;
	if (RoundManager->CurrentTimer <= RoundManager->RoundTimer - 50 && RoundManager->CurrentRound < 30)
	{
		return true;
	}

	else
	{
		return false;
	}
	
}

void ATowerDefenseGameMode::SkipRound()
{
	if (CanSkip())
	{
		RoundManager->CurrentTimer = 0;
	}
}


void ATowerDefenseGameMode::AddTowerToken(float Amount)
{
	TowerToken += Amount;
	OnTokenChanged.Broadcast(TowerToken);

}


// 타워 슬롯 관련(TowerSlot)
void ATowerDefenseGameMode::EnablePlacementMode()
{
	bIsPlacementMode = true;
	if (GridManager) GridManager->EnablePlacementMode();
}

void ATowerDefenseGameMode::SpawnTowerSlot(FVector Location)
{
	if (!TowerSlotClass) return;
	if (TowerToken > 0)
	{
		if (CurrentTowerSlot < MaxTowerSlot)
		{
			ATowerSlot* TowerSlot = GetWorld()->SpawnActor<ATowerSlot>(TowerSlotClass, Location, FRotator::ZeroRotator);

			if (GridManager)
			{
				for (AGridTile* Tile : GridManager->GridTiles)
				{
					if (Tile && FVector::Dist(Tile->GetActorLocation(), Location) < 10.f)
					{
						TowerSlot->OwnerTile = Tile;
						break;
					}
				}
				if (!TowerSlot->OwnerTile)
				{
				}
			}

			AddTowerToken(-1);
			CurrentTowerSlot += 1;
			bIsPlacementMode = false;
			if (GridManager) GridManager->DisablePlacementMode();
		}
	}
}



// 인게임 업그레이드(업그레이드용)
void ATowerDefenseGameMode::ApplyUpgrade(FBuffEffect Effect)
{
	InGameUpgrades.Add(Effect);
}

float ATowerDefenseGameMode::GetUpgradeBonus(FGameplayTag StatTag)
{
	float TotalBonus = 0.f;
	for (FBuffEffect& Effect : InGameUpgrades)
	{
		if (Effect.StatTag == StatTag)
		{
			TotalBonus += Effect.Value;
		}

	}
	UE_LOG(LogTemp, Warning, TEXT("GetUpgradeBonus: %s = %f"), *StatTag.ToString(), TotalBonus);
	return TotalBonus;
}

