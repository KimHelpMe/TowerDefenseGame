#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "MainMenuWidget.h"
#include "TowerDefenseGameMode.h"
#include "RoundManager.h"
#include "RoundDataRow.h"
#include "EnemySpawnSystem.h"
#include "Enemy.h"

namespace
{
	// 라운드 흐름: 남은 적과 무관한 진행, 스폰 중 스킵 차단, 토큰만 라운드 보상, 마지막 라운드 처치 후 승리.
	class FRoundFlowCheck : public IAutomationLatentCommand
	{
	public:
		explicit FRoundFlowCheck(FAutomationTestBase* InTest) : Test(InTest) {}

		virtual bool Update() override
		{
			UWorld* World = nullptr;
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				if (Context.WorldType == EWorldType::PIE) World = Context.World();
			}
			if (!World)
			{
				Test->AddError(TEXT("PIE world not found."));
				return true;
			}

			TArray<UUserWidget*> Menus;
			UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World, Menus, UMainMenuWidget::StaticClass(), false);
			for (UUserWidget* Menu : Menus) Menu->RemoveFromParent();
			UGameplayStatics::SetGamePaused(World, false);

			ATowerDefenseGameMode* GameMode = Cast<ATowerDefenseGameMode>(World->GetAuthGameMode());
			ARoundManager* Rounds = Cast<ARoundManager>(UGameplayStatics::GetActorOfClass(World, ARoundManager::StaticClass()));
			if (!Test->TestNotNull(TEXT("GameMode"), GameMode) || !Test->TestNotNull(TEXT("RoundManager"), Rounds)) return true;
			if (!IsValid(Rounds->SpawnSystem))
			{
				Test->AddWarning(TEXT("Skipped: the level has no EnemySpawnSystem for RoundManager."));
				return true;
			}

			// 라운드 자동 진행을 멈추고 테스트용 2라운드 데이터로 교체.
			Rounds->StopRounds();
			UDataTable* Table = NewObject<UDataTable>(GetTransientPackage());
			Table->RowStruct = FRoundDataRow::StaticStruct();
			FRoundDataRow Row{};
			FRoundEnemyEntry Entry{};
			Entry.EnemyClass = AEnemy::StaticClass();
			Entry.SpawnCount = 1;
			Entry.SpawnDelay = 0.1f;
			Row.EnemyEntries.Add(Entry);
			Row.GoldReward = 999.f;
			Row.ExpReward = 999.f;
			Row.TowerTokenReward = 2.f;
			Table->AddRow(TEXT("Round_01"), Row);
			Table->AddRow(TEXT("Round_02"), Row);
			Rounds->RoundDataTable = Table;
			Rounds->MaxRound = 2;
			Rounds->CurrentRound = 1;

			AEnemy* Leftover = World->SpawnActor<AEnemy>(AEnemy::StaticClass(), FVector(0.f, 0.f, -20000.f), FRotator::ZeroRotator);
			Test->TestTrue(TEXT("An enemy from round 1 is still alive"), Rounds->GetAliveEnemyCount() > 0);

			// 스폰 중에는 스킵/진행 불가.
			Rounds->bRoundSpawnComplete = false;
			Rounds->CurrentTimer = Rounds->RoundTimer - 60.f;
			Test->TestFalse(TEXT("Skip is blocked while the round is still spawning"), GameMode->CanSkip());
			Rounds->CurrentTimer = 1.f;
			Rounds->UpdateRoundTick();
			Test->TestEqual(TEXT("Timer end does not advance while the round is still spawning"), Rounds->CurrentRound, 1);

			// 스폰 완료 후에는 50초가 지나면 적이 남아 있어도 스킵 가능.
			Rounds->bRoundSpawnComplete = true;
			Rounds->CurrentTimer = Rounds->RoundTimer - 60.f;
			Test->TestTrue(TEXT("After 50s, skip is allowed even with enemies alive"), GameMode->CanSkip());
			Rounds->CurrentTimer = Rounds->RoundTimer - 10.f;
			Test->TestFalse(TEXT("Skip is not allowed before 50s have passed"), GameMode->CanSkip());

			// 타이머 종료 → 적이 남아 있어도 다음 라운드, 토큰만 지급.
			const float GoldBefore = GameMode->Gold;
			const float ExpBefore = GameMode->Exp;
			const int32 LevelBefore = GameMode->PlayerLevel;
			const float TokenBefore = GameMode->TowerToken;
			Rounds->CurrentTimer = 1.f;
			Rounds->UpdateRoundTick();
			Test->TestEqual(TEXT("Round advances on timer end with enemies still alive"), Rounds->CurrentRound, 2);
			Test->TestEqual(TEXT("Round transition gives the token reward"), GameMode->TowerToken - TokenBefore, 2.f);
			Test->TestEqual(TEXT("Round transition gives no gold (gold comes from kills)"), GameMode->Gold, GoldBefore);
			Test->TestTrue(TEXT("Round transition gives no EXP (EXP comes from kills)"), GameMode->Exp == ExpBefore && GameMode->PlayerLevel == LevelBefore);

			// 마지막 라운드: 적이 남아 있으면 승리하지 않고, 모두 처치하면 승리.
			Rounds->StopRounds(); // 테스트에서 실제 스폰 진행은 필요 없음(스폰 완료 상태로 둠).
			Test->TestTrue(TEXT("Round 2 is the final round"), Rounds->IsFinalRound());
			Rounds->UpdateRoundTick();
			Test->TestFalse(TEXT("Final round is not won while enemies are alive"), GameMode->bIsGameOver);

			TArray<AActor*> Enemies;
			UGameplayStatics::GetAllActorsOfClass(World, AEnemy::StaticClass(), Enemies);
			for (AActor* Enemy : Enemies) Enemy->Destroy();
			if (IsValid(Leftover)) Leftover->Destroy();
			Rounds->UpdateRoundTick();
			Test->TestTrue(TEXT("Final round is won once all enemies are gone"), GameMode->bIsGameOver && GameMode->bIsVictory);
			return true;
		}

	private:
		FAutomationTestBase* Test;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRoundFlowTest,
	"TowerDefense.Fixes.RoundFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRoundFlowTest::RunTest(const FString& Parameters)
{
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.f));
	ADD_LATENT_AUTOMATION_COMMAND(FRoundFlowCheck(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif
