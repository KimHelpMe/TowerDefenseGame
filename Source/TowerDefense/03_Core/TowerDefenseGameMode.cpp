#include "TowerDefenseGameMode.h"
#include "MainMenuWidget.h"
#include "BattleResultWidget.h"
#include "FantasyUITheme.h"
#include "Kismet/GameplayStatics.h"
#include "TowerSlot.h"
#include "BaseTower.h"
#include "AttackTower.h"
#include "TowerStatComponent.h"
#include "ManaComponent.h"
#include "StatComponent.h"


ATowerDefenseGameMode::ATowerDefenseGameMode()
{
	PrimaryActorTick.bCanEverTick = true;



	// 재화(TowerToken, Gold)
	TowerToken = 3.f;
	Gold = 500.f;
	Exp = 0;
	MaxExp = 0.f;
	PlayerLevel = 1;
	bIsGameOver = false;


	// 타워 슬롯(Tower Slot)
	MaxTowerSlot = 30;
	CurrentTowerSlot = 0;
	bIsPlacementMode = false;
}


void ATowerDefenseGameMode::BeginPlay()
{
	Super::BeginPlay();

FFantasyUITheme::ApplyToViewport(GetWorld());
    FTimerHandle UIRefreshHandle;
    GetWorldTimerManager().SetTimer(UIRefreshHandle, [this]() { FFantasyUITheme::ApplyToViewport(GetWorld()); }, 0.5f, true);

	RoundManager = Cast<ARoundManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ARoundManager::StaticClass()));

	GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
	
	CommandManager = Cast<ACommandManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ACommandManager::StaticClass()));
	
	if (LevelDataTable)
	{
		FLevelDataRow* Row = LevelDataTable->FindRow<FLevelDataRow>(TEXT("Level_01"), TEXT(""));
		if (Row) MaxExp = Row->MaxExp;
	}

	OnExpChanged.Broadcast(PlayerLevel, Exp, MaxExp);

	if (UGameplayStatics::ParseOption(OptionsString, TEXT("StartBattle")) == TEXT("1")) return;
	UClass* WidgetClass = StaticLoadClass(
		UMainMenuWidget::StaticClass(),
		nullptr,
		TEXT("/Game/03_UI/WBP_MainMenu.WBP_MainMenu_C"));
	if (!WidgetClass) WidgetClass = UMainMenuWidget::StaticClass();
	if (UMainMenuWidget* MainMenuWidget = CreateWidget<UMainMenuWidget>(GetWorld(), WidgetClass))
	{
		MainMenuWidget->AddToViewport(100);
	}

}

void ATowerDefenseGameMode::AddExp(float Amount)
{
	if (!FMath::IsFinite(Amount) || Amount <= 0.f) return;

	Exp = FMath::Max(0.f, Exp + Amount);
	if (!LevelDataTable)
	{
		OnExpChanged.Broadcast(PlayerLevel, Exp, MaxExp);
		return;
	}

	while (true)
	{
		const FString RowKeyString = FString::Printf(TEXT("Level_%02d"), PlayerLevel);
		FLevelDataRow* Row = LevelDataTable->FindRow<FLevelDataRow>(FName(*RowKeyString), TEXT(""));
		if (!Row || !FMath::IsFinite(Row->MaxExp) || Row->MaxExp <= 0.f)
		{
			MaxExp = Row ? FMath::Max(0.f, Row->MaxExp) : 0.f;
			OnExpChanged.Broadcast(PlayerLevel, Exp, MaxExp);
			return;
		}

		MaxExp = Row->MaxExp;
		if (Exp < MaxExp)
		{
			OnExpChanged.Broadcast(PlayerLevel, Exp, MaxExp);
			return;
		}

		const int32 NextLevel = PlayerLevel + 1;
		const FString NextRowKey = FString::Printf(TEXT("Level_%02d"), NextLevel);
		FLevelDataRow* NextRow = LevelDataTable->FindRow<FLevelDataRow>(FName(*NextRowKey), TEXT(""));
		if (!NextRow || !FMath::IsFinite(NextRow->MaxExp) || NextRow->MaxExp <= 0.f)
		{
			Exp = MaxExp;
			OnExpChanged.Broadcast(PlayerLevel, Exp, MaxExp);
			return;
		}

		Exp -= MaxExp;
		PlayerLevel = NextLevel;
		AddTowerToken(Row->TokenReward);
	}
}



void ATowerDefenseGameMode::TriggerGameOver()
{
	if (bIsGameOver) return;

	bIsGameOver = true;
	if (RoundManager)
	{
		RoundManager->StopRounds();
	}

    OnGameOver.Broadcast();
    ShowBattleResult();

}

void ATowerDefenseGameMode::TriggerVictory()
{
	if (bIsGameOver) return;

	bIsGameOver = true;
	bIsVictory = true;
	if (RoundManager)
	{
		RoundManager->StopRounds();
	}

	OnVictory.Broadcast();
	ShowBattleResult();
}

void ATowerDefenseGameMode::ShowBattleResult()
{
    UClass* ResultClass=LoadClass<UBattleResultWidget>(nullptr,TEXT("/Game/03_UI/WBP_BattleResult.WBP_BattleResult_C"));
    if (ResultClass) if (UBattleResultWidget* Result=CreateWidget<UBattleResultWidget>(GetWorld(),ResultClass)) Result->AddToViewport(100);
}

// 라운드 스킵(Skip) 관련
bool ATowerDefenseGameMode::CanSkip()
{
	if (!RoundManager) return false;
	// 50초 경과 후 남은 적과 관계없이 스킵 가능. 단 다음 라운드를 시작하면 이번 라운드의 남은 스폰이
	// 사라지므로 스폰이 끝난 뒤에만 허용(라운드 0의 대기 시간은 스폰이 없으므로 그대로 허용).
	const bool bSpawnFinished = RoundManager->CurrentRound == 0 || RoundManager->bRoundSpawnComplete;
	if (RoundManager->CurrentTimer <= RoundManager->RoundTimer - 50 && !RoundManager->IsFinalRound() && bSpawnFinished)
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

bool ATowerDefenseGameMode::SpawnTowerSlot(FVector Location)
{
	if (bIsGameOver || !TowerSlotClass || !IsValid(GridManager) || TowerToken < 1.f || CurrentTowerSlot >= MaxTowerSlot)
	{
		return false;
	}

	AGridTile* TargetTile = nullptr;
	for (AGridTile* Tile : GridManager->GridTiles)
	{
		if (IsValid(Tile) && FVector::Dist(Tile->GetActorLocation(), Location) < 10.f)
		{
			TargetTile = Tile;
			break;
		}
	}

	if (!IsValid(TargetTile) || TargetTile->bIsOccupied || !GetWorld()) return false;

	ATowerSlot* TowerSlot = GetWorld()->SpawnActor<ATowerSlot>(
		TowerSlotClass,
		Location,
		FRotator::ZeroRotator);
	if (!IsValid(TowerSlot)) return false;

	TowerSlot->OwnerTile = TargetTile;
	TargetTile->bIsOccupied = true;
	TargetTile->SetHighlight(false);

	AddTowerToken(-1.f);
	CurrentTowerSlot++;
	bIsPlacementMode = false;
	GridManager->DisablePlacementMode();
	return true;
}



// 인게임 업그레이드(업그레이드용)
void ATowerDefenseGameMode::ApplyUpgrade(FBuffEffect Effect)
{
	if (!Effect.StatTag.IsValid() || !FMath::IsFinite(Effect.Value)) return;

	InGameUpgrades.Add(Effect);

	const FGameplayTag AttackRangeTag = UStatComponent::GetTag("Upgrade.AttackRange");
	const FGameplayTag AttackSpeedTag = UStatComponent::GetTag("Upgrade.AttackSpeed");
	const FGameplayTag ManaRegenTag = UStatComponent::GetTag("Upgrade.ManaRegen");
	if (Effect.StatTag != AttackRangeTag && Effect.StatTag != AttackSpeedTag && Effect.StatTag != ManaRegenTag) return;
	if (!GetWorld()) return;

	TArray<AActor*> Towers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseTower::StaticClass(), Towers);
	for (AActor* Actor : Towers)
	{
		ABaseTower* Tower = Cast<ABaseTower>(Actor);
		if (!IsValid(Tower)) continue;

		if (Effect.StatTag == AttackRangeTag)
		{
			if (UTowerStatComponent* TowerStat = Tower->GetComponentByClass<UTowerStatComponent>())
			{
				TowerStat->RecalculateStats();
			}
		}
		else if (Effect.StatTag == AttackSpeedTag)
		{
			if (AAttackTower* AttackTower = Cast<AAttackTower>(Tower))
			{
				AttackTower->RefreshAttackTimer();
			}
		}
		else if (Effect.StatTag == ManaRegenTag)
		{
			if (UManaComponent* ManaComponent = Tower->GetComponentByClass<UManaComponent>())
			{
				ManaComponent->RefreshRegenTimer();
			}
		}
	}
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
