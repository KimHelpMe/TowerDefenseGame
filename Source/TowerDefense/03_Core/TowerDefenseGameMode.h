#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FBuffEffect.h"
#include "RoundManager.h"
#include "GridManager.h"
#include "CommandManager.h"
#include "LevelDataRow.h"
#include "TowerDefenseGameMode.generated.h"


class AEnemySpawnSystem;
class ATowerSlot;
class ARoundManager;
class AGridManager;
class ACommandManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTokenChanged, float, NewToken);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnExpChanged, int32, NewLevel, float, NewExp, float, NewMaxExp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameOver);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVictory);

UCLASS()
class TOWERDEFENSE_API ATowerDefenseGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	ATowerDefenseGameMode();

	// 재화 부분(TowerToken 및 Gold)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TowerToken;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Gold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Exp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxExp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlayerLevel;

	UPROPERTY(BlueprintAssignable)
	FOnTokenChanged OnTokenChanged;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game State")
	bool bIsGameOver;

	UPROPERTY(BlueprintAssignable, Category = "Game State")
	FOnGameOver OnGameOver;

	UFUNCTION(BlueprintCallable, Category = "Game State")
	void TriggerGameOver();

	// 마지막 라운드를 정리하면 true. 결과 위젯이 승리/패배 문구를 고를 때 사용.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game State")
	bool bIsVictory = false;

	UPROPERTY(BlueprintAssignable, Category = "Game State")
	FOnVictory OnVictory;

	UFUNCTION(BlueprintCallable, Category = "Game State")
	void TriggerVictory();

	// Level.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* LevelDataTable;

	UFUNCTION()
	void AddExp(float Amount);

	UPROPERTY(BlueprintAssignable)
	FOnExpChanged OnExpChanged;

	// 스킵(Skip) 부분
	UFUNCTION(BlueprintCallable)
	bool CanSkip();

	UFUNCTION(BlueprintCallable)
	void SkipRound();

	// 본진 힐링, 필살기, 배속
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ACommandManager* CommandManager;

public:

	// 속성 상성 및 방어 상성.
	UPROPERTY(EditAnywhere)
	UDataTable* DefenseTypeDataTable;

	UPROPERTY(EditAnywhere)
	UDataTable* ElementalDataTable;

public:

	// 인게임 업그레이드(업그레이드용)

	UPROPERTY(BlueprintReadWrite)
	TArray<FBuffEffect> InGameUpgrades;

	UFUNCTION(BlueprintCallable)
	void ApplyUpgrade(FBuffEffect Effect);

	UFUNCTION(BlueprintCallable)
	float GetUpgradeBonus(FGameplayTag StatTag);

	UFUNCTION(BlueprintCallable)
	void AddTowerToken(float Amount);


protected:

	virtual void BeginPlay() override;

	// 게임 종료(승리/패배) 결과 위젯 표시.
	void ShowBattleResult();

public:


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ARoundManager* RoundManager;



	// GridManager.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AGridManager* GridManager;

public:

	// 타워 스폰 관련(TowerSlot).
	// 타워 수량.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MaxTowerSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int CurrentTowerSlot;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ATowerSlot> TowerSlotClass;
 
	// 설치 모드 여부.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPlacementMode; 

	// 설치 모드 ON.
	UFUNCTION(BlueprintCallable)
	void EnablePlacementMode();

	// 슬롯 스폰 + 설치모드 OFF.
	UFUNCTION(BlueprintCallable)
	bool SpawnTowerSlot(FVector Location);



};
