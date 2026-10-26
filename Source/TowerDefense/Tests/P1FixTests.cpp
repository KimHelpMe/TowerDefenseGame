#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/TextBlock.h"
#include "MainMenuWidget.h"
#include "BattleResultWidget.h"
#include "TowerInfoWidget.h"
#include "TowerRangeDecalComponent.h"
#include "GachaWidget.h"
#include "TowerDefenseGameMode.h"
#include "CommandManager.h"
#include "Base.h"
#include "BaseTower.h"
#include "BuffTower.h"
#include "MagicTower.h"
#include "Enemy.h"
#include "BossEnemy.h"
#include "SynergyManager.h"
#include "EpicTowerManager.h"
#include "GachaManager.h"
#include "TowerSlot.h"
#include "EnemyStatComponent.h"
#include "TowerStatComponent.h"
#include "BossAbilityComponent.h"
#include "ShieldAbilityComponent.h"
#include "SplitAbilityComponent.h"
#include "PassiveBuffComponent.h"
#include "AreaAttackComponent.h"
#include "AirAbilityComponent.h"
#include "Components/SphereComponent.h"
#include "EnemyDataRow.h"
#include "TowerDataRow.h"
#include "SynergyDataRow.h"
#include "ElementalDataRow.h"
#include "DefenseTypeDataRow.h"
#include "EpicTowerDataRow.h"

namespace
{
	// PIE 월드에서 P1(게임 로직) 수정 사항을 확인한다.
	class FP1FixCheck : public IAutomationLatentCommand
	{
	public:
		explicit FP1FixCheck(FAutomationTestBase* InTest) : Test(InTest) {}

		virtual bool Update() override
		{
			const double Now = FPlatformTime::Seconds();
			if (Now < NextStepTime) return false;

			if (Step == 0)
			{
				for (const FWorldContext& Context : GEngine->GetWorldContexts())
				{
					if (Context.WorldType == EWorldType::PIE) World = Context.World();
				}
				if (!World.IsValid())
				{
					Test->AddError(TEXT("PIE world not found."));
					return true;
				}
				GameMode = Cast<ATowerDefenseGameMode>(World->GetAuthGameMode());
				if (!Test->TestNotNull(TEXT("TowerDefenseGameMode is active"), GameMode.Get())) return true;

				TArray<UUserWidget*> Menus;
				UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World.Get(), Menus, UMainMenuWidget::StaticClass(), false);
				for (UUserWidget* Menu : Menus) Menu->RemoveFromParent();
				UGameplayStatics::SetGamePaused(World.Get(), false);

				CheckSynergySameStep();      // P1-5
				CheckEnemyHPRowOrder();      // P1-6
				CheckLevelUpBuffCopy();      // P1-7
				CheckPurgeDoubleReward();    // P1-8
				CheckSlotReset();            // P1-9
				CheckFusionValidation();     // P1-10
				SetupBuffRange();            // P1-11 (overlap query is checked next step)
				CheckGachaDoubleOpen();      // P1-14
				CheckSingleTowerInfo();      // P1-15
				CheckSplitSpacing();         // P1-16
				CheckNegativeDamage();       // P1-17
				CheckChaosDamageBypassesMatchups();
				CheckBossPhases();           // P1-18
				SetupExplosionRadius();      // P1-19② (overlap query is checked next step)
				CheckPassiveInterval();      // P1-19③
				Step = 1;
				NextStepTime = Now + 0.3;
				return false;
			}

			if (!World.IsValid())
			{
				Test->AddError(TEXT("PIE ended during the P1 test."));
				return true;
			}

			CheckBuffRange();                // P1-11
			CheckExplosionRadius();          // P1-19②
			CheckLevelUpRegistration();      // P1-19④ (after the buff-range towers are gone)
			CheckVictoryAndHeal();           // P1-13, P1-20
			return true;
		}

	private:
		FVector TestLocation(float X = 0.f, float Y = 0.f) const
		{
			// 레벨의 게임플레이 영역과 겹치지 않는 위치.
			return FVector(10000.f + X, 10000.f + Y, -20000.f);
		}

		template <typename T>
		T* SpawnTowerDeferred(UClass* Class, const FVector& Location, TFunctionRef<void(T*)> Configure)
		{
			const FTransform Transform(Location);
			T* Tower = World->SpawnActorDeferred<T>(Class, Transform);
			if (!Tower) return nullptr;
			Configure(Tower);
			Tower->FinishSpawning(Transform);
			return Tower;
		}

		static bool HasBuffKey(const UStatComponent* Stat, FName Key)
		{
			return Stat && Stat->ActiveBuffs.ContainsByPredicate([Key](const FBuffEffect& Buff) { return Buff.SourceKey == Key; });
		}

		void CheckSynergySameStep()
		{
			ASynergyManager* SynergyManager = Cast<ASynergyManager>(UGameplayStatics::GetActorOfClass(World.Get(), ASynergyManager::StaticClass()));
			if (!SynergyManager) SynergyManager = World->SpawnActor<ASynergyManager>(ASynergyManager::StaticClass(), TestLocation(), FRotator::ZeroRotator);
			if (!Test->TestNotNull(TEXT("[P1-5] SynergyManager available"), SynergyManager)) return;

			const FGameplayTag Tag = UStatComponent::GetTag(TEXT("Synergy.Class.Archer"));
			UDataTable* Table = NewObject<UDataTable>(GetTransientPackage());
			Table->RowStruct = FSynergyDataRow::StaticStruct();
			FSynergyDataRow Row{};
			Row.SynergyTag = Tag;
			FBuffEffect Effect{};
			Effect.StatTag = UStatComponent::GetTag(TEXT("Stat.AttackDamage"));
			Effect.Value = 10.f;
			Effect.ValueType = EBuffValueType::Flat;
			FSynergyStep Step1;
			Step1.RequireCount = 2;
			Step1.Effects.Add(Effect);
			FSynergyStep Step2;
			Step2.RequireCount = 4;
			Step2.Effects.Add(Effect);
			Row.Steps = { Step1, Step2 };
			Table->AddRow(TEXT("TestSynergy"), Row);
			const FName Key(TEXT("Synergy_TestSynergy"));

			UDataTable* SavedTable = SynergyManager->SynergyDataTable;
			SynergyManager->SynergyDataTable = Table;

			TArray<ABaseTower*> Towers;
			for (int32 Index = 0; Index < 3; ++Index)
			{
				Towers.Add(SpawnTowerDeferred<ABaseTower>(ABaseTower::StaticClass(), TestLocation(0.f, Index * 100.f),
					[&Tag](ABaseTower* Tower) { Tower->TowerStatComponent->SynergyTags.AddTag(Tag); }));
			}
			if (Towers.Contains(nullptr))
			{
				Test->AddError(TEXT("[P1-5] Synergy towers failed to spawn"));
			}
			else
			{
				Test->TestTrue(TEXT("[P1-5] First two towers get the step-1 synergy buff"),
					HasBuffKey(Towers[0]->TowerStatComponent, Key) && HasBuffKey(Towers[1]->TowerStatComponent, Key));
				Test->TestTrue(TEXT("[P1-5] Third tower joining the same step also gets the synergy buff"),
					HasBuffKey(Towers[2]->TowerStatComponent, Key));
				const int32 BuffCount = Towers[0]->TowerStatComponent->ActiveBuffs.FilterByPredicate(
					[&Key](const FBuffEffect& Buff) { return Buff.SourceKey == Key; }).Num();
				Test->TestEqual(TEXT("[P1-5] Refreshing does not stack the same synergy buff"), BuffCount, 1);
			}

			for (ABaseTower* Tower : Towers) if (Tower) Tower->Destroy();
			SynergyManager->SynergyDataTable = SavedTable;
			SynergyManager->UpdateSynergies();
		}

		void CheckEnemyHPRowOrder()
		{
			UDataTable* Table = NewObject<UDataTable>(GetTransientPackage());
			Table->RowStruct = FEnemyDataRow::StaticStruct();
			const FGameplayTag HPTag = UStatComponent::GetTag(TEXT("Stat.HP"));
			const FGameplayTag MaxHPTag = UStatComponent::GetTag(TEXT("Stat.MaxHP"));

			FEnemyDataRow HPFirst{};
			HPFirst.Stats.Add(FBuffEffect{ HPTag, 500.f, EBuffValueType::Flat });
			HPFirst.Stats.Add(FBuffEffect{ MaxHPTag, 500.f, EBuffValueType::Flat });
			Table->AddRow(TEXT("HPFirst"), HPFirst);

			FEnemyDataRow MaxOnly{};
			MaxOnly.Stats.Add(FBuffEffect{ MaxHPTag, 800.f, EBuffValueType::Flat });
			Table->AddRow(TEXT("MaxOnly"), MaxOnly);

			ATowerSlot* Holder = World->SpawnActor<ATowerSlot>(ATowerSlot::StaticClass(), TestLocation(), FRotator::ZeroRotator);
			if (!Test->TestNotNull(TEXT("[P1-6] Holder spawned"), Holder)) return;

			UEnemyStatComponent* Stat = NewObject<UEnemyStatComponent>(Holder);
			Stat->EnemyDataTable = Table;
			Stat->EnemyRowName = TEXT("HPFirst");
			Stat->InitFromDataTable();
			Test->TestEqual(TEXT("[P1-6] HP listed before MaxHP keeps 500 instead of clamping to 100"), Stat->GetStat(HPTag), 500.f);

			UEnemyStatComponent* MaxOnlyStat = NewObject<UEnemyStatComponent>(Holder);
			MaxOnlyStat->EnemyDataTable = Table;
			MaxOnlyStat->EnemyRowName = TEXT("MaxOnly");
			MaxOnlyStat->InitFromDataTable();
			Test->TestEqual(TEXT("[P1-6] Row with only MaxHP starts at full HP"), MaxOnlyStat->GetStat(HPTag), 800.f);

			Holder->Destroy();
		}

		void CheckLevelUpBuffCopy()
		{
			UDataTable* Table = NewObject<UDataTable>(GetTransientPackage());
			Table->RowStruct = FTowerDataRow::StaticStruct();
			FTowerDataRow Row{};
			Row.MaxLevel = 3;
			Row.GoldCost = 0.f;
			Table->AddRow(TEXT("TestTower_Lv1"), Row);
			Table->AddRow(TEXT("TestTower_Lv2"), Row);

			ATowerSlot* Slot = World->SpawnActor<ATowerSlot>(ATowerSlot::StaticClass(), TestLocation(500.f), FRotator::ZeroRotator);
			ABaseTower* OldTower = SpawnTowerDeferred<ABaseTower>(ABaseTower::StaticClass(), TestLocation(500.f),
				[Table](ABaseTower* Tower)
				{
					Tower->TowerStatComponent->TowerDataTable = Table;
					Tower->TowerStatComponent->TowerRowName = TEXT("TestTower");
					Tower->TowerStatComponent->CurrentLevel = 1;
					Tower->NextLevelClass = ABaseTower::StaticClass();
				});
			if (!Test->TestNotNull(TEXT("[P1-7] Slot spawned"), Slot) || !Test->TestNotNull(TEXT("[P1-7] Tower spawned"), OldTower)) return;
			OldTower->OwnerSlot = Slot;
			Slot->InstalledTower = OldTower;

			// 다른 타워의 Active 버프가 걸린 상태를 재현.
			const FName ActiveKey(TEXT("BuffComponent_P1Test"));
			FBuffEffect Buff{};
			Buff.StatTag = UStatComponent::GetTag(TEXT("Stat.AttackDamage"));
			Buff.Value = 5.f;
			Buff.ValueType = EBuffValueType::Flat;
			Buff.SourceKey = ActiveKey;
			OldTower->TowerStatComponent->ModifyStat(Buff);

			OldTower->TowerStatComponent->LevelUp();
			ABaseTower* NewTower = Slot->InstalledTower;
			if (Test->TestTrue(TEXT("[P1-7] Level up replaced the tower"), IsValid(NewTower) && NewTower != OldTower))
			{
				Test->TestEqual(TEXT("[P1-7] New tower is level 2"), NewTower->TowerStatComponent->CurrentLevel, 2);
				Test->TestFalse(TEXT("[P1-7] Another tower's Active buff is not copied to the new tower"), HasBuffKey(NewTower->TowerStatComponent, ActiveKey));
				NewTower->Destroy();
			}
			Slot->Destroy();
		}

		void CheckPurgeDoubleReward()
		{
			AEnemy* Enemy = World->SpawnActor<AEnemy>(AEnemy::StaticClass(), TestLocation(1000.f), FRotator::ZeroRotator);
			if (!Test->TestNotNull(TEXT("[P1-8] Enemy spawned"), Enemy)) return;

			const float GoldBefore = GameMode->Gold;
			Enemy->OnEnemyDeath();
			Enemy->OnEnemyDeath(); // Purge가 죽어가는 적에게 다시 호출하던 경로.
			Test->TestEqual(TEXT("[P1-8] Gold reward is paid only once per enemy"), GameMode->Gold - GoldBefore, Enemy->GoldReward);
		}

		void CheckSlotReset()
		{
			ATowerSlot* Slot = World->SpawnActor<ATowerSlot>(ATowerSlot::StaticClass(), TestLocation(1500.f), FRotator::ZeroRotator);
			if (!Test->TestNotNull(TEXT("[P1-9] Slot spawned"), Slot)) return;

			Slot->SavedRollResult = { ABaseTower::StaticClass(), ABaseTower::StaticClass(), ABaseTower::StaticClass() };
			Slot->ReloadCount = 0;
			Slot->bCanAbandon = true;
			Slot->ResetGachaState();

			const ATowerSlot* Defaults = GetDefault<ATowerSlot>();
			Test->TestTrue(TEXT("[P1-9] Saved roll result is cleared"), Slot->SavedRollResult.IsEmpty());
			Test->TestEqual(TEXT("[P1-9] Reload count returns to the class default"), Slot->ReloadCount, Defaults->ReloadCount);
			Test->TestTrue(TEXT("[P1-9] Abandon flag returns to the class default"), Slot->bCanAbandon == Defaults->bCanAbandon);
			Slot->Destroy();
		}

		void CheckFusionValidation()
		{
			AEpicTowerManager* EpicManager = Cast<AEpicTowerManager>(UGameplayStatics::GetActorOfClass(World.Get(), AEpicTowerManager::StaticClass()));
			if (!EpicManager) EpicManager = World->SpawnActor<AEpicTowerManager>(AEpicTowerManager::StaticClass(), TestLocation(), FRotator::ZeroRotator);
			AGachaManager* Gacha = Cast<AGachaManager>(UGameplayStatics::GetActorOfClass(World.Get(), AGachaManager::StaticClass()));
			if (!Gacha) Gacha = World->SpawnActor<AGachaManager>(AGachaManager::StaticClass(), TestLocation(), FRotator::ZeroRotator);
			if (!Test->TestNotNull(TEXT("[P1-10] EpicTowerManager available"), EpicManager) || !Test->TestNotNull(TEXT("[P1-10] GachaManager available"), Gacha)) return;

			// 에픽 = ABuffTower, 재료 = AMagicTower 1개.
			UDataTable* Table = NewObject<UDataTable>(GetTransientPackage());
			Table->RowStruct = FEpicTowerDataRow::StaticStruct();
			FEpicTowerDataRow Row{};
			Row.EpicTowerClass = ABuffTower::StaticClass();
			Row.EpicType = EEpicTowerType::Fusion;
			FEpicTowerConditionEntry Condition{};
			Condition.TowerClass = AMagicTower::StaticClass();
			Condition.RequiredLevel = 1;
			Row.Conditions.Add(Condition);
			Table->AddRow(TEXT("TestFusion"), Row);

			UDataTable* SavedTable = EpicManager->EpicTowerDataTable;
			ATowerSlot* SavedSlot = Gacha->CurrentSlot;
			const TArray<TSubclassOf<AActor>> SavedRoll = Gacha->CurrentRollResult;
			const TArray<TSubclassOf<AActor>> SavedPending = Gacha->PendingFusionTowers;
			EpicManager->EpicTowerDataTable = Table;

			ATowerSlot* Slot = World->SpawnActor<ATowerSlot>(ATowerSlot::StaticClass(), TestLocation(2000.f), FRotator::ZeroRotator);
			if (!Test->TestNotNull(TEXT("[P1-10] Slot spawned"), Slot)) return;

			auto CountBuffTowers = [this]()
			{
				TArray<AActor*> Found;
				UGameplayStatics::GetAllActorsOfClass(World.Get(), ABuffTower::StaticClass(), Found);
				return Found.Num();
			};

			// 재료 없음 → 설치되지 않고 합성 카드가 취소되어야 함.
			const int32 BuffTowersBefore = CountBuffTowers();
			Gacha->PendingFusionTowers = { ABuffTower::StaticClass() };
			Gacha->CurrentRollResult = { ABuffTower::StaticClass() };
			Gacha->CurrentSlot = Slot;
			Gacha->SelectItem(0);
			Test->TestEqual(TEXT("[P1-10] Without materials the epic tower is not installed"), CountBuffTowers(), BuffTowersBefore);
			Test->TestFalse(TEXT("[P1-10] Without materials the fusion card is cancelled"), Gacha->PendingFusionTowers.Contains(ABuffTower::StaticClass()));
			Test->TestFalse(TEXT("[P1-10] Slot stays empty"), Slot->bIsOccupied);

			// 재료 있음 → 설치되고 재료가 소비되어야 함.
			AMagicTower* Material = World->SpawnActor<AMagicTower>(AMagicTower::StaticClass(), TestLocation(2000.f, 300.f), FRotator::ZeroRotator);
			Gacha->PendingFusionTowers = { ABuffTower::StaticClass() };
			Gacha->CurrentRollResult = { ABuffTower::StaticClass() };
			Gacha->CurrentSlot = Slot;
			Gacha->SelectItem(0);
			Test->TestTrue(TEXT("[P1-10] With materials the epic tower is installed in the slot"), IsValid(Slot->InstalledTower) && Slot->InstalledTower->IsA<ABuffTower>());
			Test->TestFalse(TEXT("[P1-10] The material tower is consumed"), IsValid(Material));
			Test->TestFalse(TEXT("[P1-10] Pending fusion entry is removed after fusing"), Gacha->PendingFusionTowers.Contains(ABuffTower::StaticClass()));

			if (IsValid(Slot->InstalledTower)) Slot->InstalledTower->Destroy();
			if (IsValid(Material)) Material->Destroy();
			Slot->Destroy();
			EpicManager->EpicTowerDataTable = SavedTable;
			Gacha->CurrentSlot = IsValid(SavedSlot) ? SavedSlot : nullptr;
			Gacha->CurrentRollResult = SavedRoll;
			Gacha->PendingFusionTowers = SavedPending;
		}

		void SetupBuffRange()
		{
			BuffSource = World->SpawnActor<ABaseTower>(ABaseTower::StaticClass(), TestLocation(3000.f), FRotator::ZeroRotator);
			NearTower = World->SpawnActor<ABaseTower>(ABaseTower::StaticClass(), TestLocation(3200.f), FRotator::ZeroRotator);
			FarTower = World->SpawnActor<ABaseTower>(ABaseTower::StaticClass(), TestLocation(3600.f), FRotator::ZeroRotator);
			if (!BuffSource.IsValid() || !NearTower.IsValid() || !FarTower.IsValid())
			{
				Test->AddError(TEXT("[P1-11] Buff range towers failed to spawn"));
				return;
			}
			// 600 떨어진 타워의 공격 사거리(500)는 버프 반경(300)과 겹치지만 타워 자체는 범위 밖.
			FarTower->TowerAttackRangeComponent->SetSphereRadius(500.f);

			UPassiveBuffComponent* Buff = NewObject<UPassiveBuffComponent>(BuffSource.Get());
			Buff->BuffRadius = 300.f;
			Buff->BuffTarget = EBuffTarget::AllyTower;
			BuffSource->AddInstanceComponent(Buff);
			Buff->RegisterComponent();
			BuffComponent = Buff;
		}

		void CheckBuffRange()
		{
			if (!BuffComponent.IsValid()) return;
			const TArray<AActor*> Targets = BuffComponent->GetTargetsInRange();
			Test->TestTrue(TEXT("[P1-11] Tower inside the buff radius is a target"), Targets.Contains(NearTower.Get()));
			Test->TestFalse(TEXT("[P1-11] Tower whose attack-range sphere merely touches the radius is not a target"), Targets.Contains(FarTower.Get()));
			for (TWeakObjectPtr<ABaseTower> Tower : { BuffSource, NearTower, FarTower }) if (Tower.IsValid()) Tower->Destroy();
		}

		void CheckGachaDoubleOpen()
		{
			AGachaManager* Gacha = Cast<AGachaManager>(UGameplayStatics::GetActorOfClass(World.Get(), AGachaManager::StaticClass()));
			if (!Gacha || !Gacha->GachaWidgetClass)
			{
				Test->AddWarning(TEXT("[P1-14] Skipped: no GachaManager with a widget class in this level."));
				return;
			}
			ATowerSlot* SavedSlot = Gacha->CurrentSlot;
			ATowerSlot* SlotA = World->SpawnActor<ATowerSlot>(ATowerSlot::StaticClass(), TestLocation(4000.f), FRotator::ZeroRotator);
			ATowerSlot* SlotB = World->SpawnActor<ATowerSlot>(ATowerSlot::StaticClass(), TestLocation(4100.f), FRotator::ZeroRotator);

			Gacha->OpenGacha(SlotA);
			Gacha->OpenGacha(SlotB);
			TArray<UUserWidget*> GachaWidgets;
			UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World.Get(), GachaWidgets, UGachaWidget::StaticClass(), true);
			Test->TestEqual(TEXT("[P1-14] Only one gacha widget is on screen"), GachaWidgets.Num(), 1);
			Test->TestTrue(TEXT("[P1-14] The first slot stays the current slot"), Gacha->CurrentSlot == SlotA);

			Gacha->CloseGacha();
			Gacha->CurrentSlot = IsValid(SavedSlot) ? SavedSlot : nullptr;
			SlotA->Destroy();
			SlotB->Destroy();
		}

		void CheckSingleTowerInfo()
		{
			ABaseTower* TowerA = World->SpawnActor<ABaseTower>(ABaseTower::StaticClass(), TestLocation(4500.f), FRotator::ZeroRotator);
			ABaseTower* TowerB = World->SpawnActor<ABaseTower>(ABaseTower::StaticClass(), TestLocation(4700.f), FRotator::ZeroRotator);
			if (!Test->TestNotNull(TEXT("[P1-15] Towers spawned"), TowerA) || !TowerB) return;

			TowerA->OnTowerClicked(nullptr, FKey());
			TowerB->OnTowerClicked(nullptr, FKey());
			TArray<UUserWidget*> InfoWidgets;
			UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World.Get(), InfoWidgets, UTowerInfoWidget::StaticClass(), true);
			if (InfoWidgets.IsEmpty())
			{
				Test->AddWarning(TEXT("[P1-15] Skipped: tower info widget class could not be created."));
			}
			else
			{
				Test->TestEqual(TEXT("[P1-15] Only one tower info panel is open"), InfoWidgets.Num(), 1);
				Test->TestFalse(TEXT("[P1-15] The previous tower's range decal is hidden"), TowerA->TowerRangeDecal->IsRangeVisible());
			}
			if (TowerB->TowerInfoWidget) TowerB->TowerInfoWidget->CloseWidget();
			TowerA->Destroy();
			TowerB->Destroy();
		}

		void CheckSplitSpacing()
		{
			AEnemy* Owner = World->SpawnActor<AEnemy>(AEnemy::StaticClass(), TestLocation(5000.f), FRotator::ZeroRotator);
			if (!Test->TestNotNull(TEXT("[P1-16] Enemy spawned"), Owner)) return;
			Owner->CurrentSplinePoint = 500.f;

			USplitAbilityComponent* Split = NewObject<USplitAbilityComponent>(Owner);
			Split->SplitEnemyClass = AEnemy::StaticClass();
			Split->SplitCount = 3;

			TArray<AActor*> Before;
			UGameplayStatics::GetAllActorsOfClass(World.Get(), AEnemy::StaticClass(), Before);
			Split->OnEnemyDeathAbility();
			TArray<AActor*> After;
			UGameplayStatics::GetAllActorsOfClass(World.Get(), AEnemy::StaticClass(), After);

			TArray<float> Points;
			for (AActor* Actor : After)
			{
				if (Before.Contains(Actor)) continue;
				Points.Add(CastChecked<AEnemy>(Actor)->CurrentSplinePoint);
				Actor->Destroy();
			}
			Points.Sort();
			Test->TestEqual(TEXT("[P1-16] Split spawns SplitCount children"), Points.Num(), 3);
			if (Points.Num() == 3)
			{
				Test->TestTrue(FString::Printf(TEXT("[P1-16] Children are staggered along the spline (%.0f, %.0f, %.0f)"), Points[0], Points[1], Points[2]),
					FMath::IsNearlyEqual(Points[0], 400.f) && FMath::IsNearlyEqual(Points[1], 450.f) && FMath::IsNearlyEqual(Points[2], 500.f));
			}
			Owner->Destroy();
		}

		void CheckNegativeDamage()
		{
			UShieldAbilityComponent* Shield = NewObject<UShieldAbilityComponent>(GetTransientPackage());
			const float ShieldBefore = Shield->ShieldAmount;
			Test->TestEqual(TEXT("[P1-17] Negative damage passes nothing through"), Shield->AbsorbDamage(-50.f), 0.f);
			Test->TestEqual(TEXT("[P1-17] Negative damage does not increase the shield"), Shield->ShieldAmount, ShieldBefore);
		}

		void CheckBossPhases()
		{
			const FTransform Transform(TestLocation(5500.f));
			ABossEnemy* Boss = World->SpawnActorDeferred<ABossEnemy>(ABossEnemy::StaticClass(), Transform);
			if (!Test->TestNotNull(TEXT("[P1-18] Boss spawned"), Boss)) return;

			UEnemyStatComponent* Stat = NewObject<UEnemyStatComponent>(Boss, TEXT("TestBossStat"));
			Stat->SetBaseStat(UStatComponent::GetTag(TEXT("Stat.MaxHP")), 1000.f);
			Stat->SetBaseStat(UStatComponent::GetTag(TEXT("Stat.HP")), 1000.f);
			UBossAbilityComponent* Ability = NewObject<UBossAbilityComponent>(Boss, TEXT("TestBossAbility"));
			Boss->AddInstanceComponent(Stat);
			Boss->AddInstanceComponent(Ability);
			Stat->RegisterComponent();
			Ability->RegisterComponent();
			Boss->FinishSpawning(Transform);

			// 100% → 5% 한 번에.
			Boss->ReceiveDamage(950.f, nullptr);
			Test->TestEqual(TEXT("[P1-18] One hit from above 50% to below 10% reaches phase 3"), Ability->CurrentPhase, 3);
			Boss->Destroy();
		}

		void CheckChaosDamageBypassesMatchups()
		{
			if (!GameMode.IsValid()) return;

			UDataTable* SavedDefenseTable = GameMode->DefenseTypeDataTable;
			UDataTable* SavedElementalTable = GameMode->ElementalDataTable;

			UDataTable* DefenseTable = NewObject<UDataTable>(GetTransientPackage());
			DefenseTable->RowStruct = FDefenseTypeDataRow::StaticStruct();
			FDefenseTypeDataRow DivineDefense{};
			DivineDefense.PhysicalMultiplier = 0.25f;
			DivineDefense.MagicMultiplier = 0.5f;
			DivineDefense.ChaosMultiplier = 1.5f;
			DefenseTable->AddRow(FName(TEXT("DefenseType.Divine")), DivineDefense);

			UDataTable* ElementalTable = NewObject<UDataTable>(GetTransientPackage());
			ElementalTable->RowStruct = FElementalDataRow::StaticStruct();
			const FGameplayTag FireTag = UStatComponent::GetTag(TEXT("Element.Fire"));
			FElementalDataRow FireRow{};
			FireRow.Multipliers.Add(FireTag, 0.5f);
			ElementalTable->AddRow(FName(TEXT("Element.Fire")), FireRow);

			GameMode->DefenseTypeDataTable = DefenseTable;
			GameMode->ElementalDataTable = ElementalTable;

			const FTransform Transform(TestLocation(12000.f));
			AEnemy* Enemy = World->SpawnActorDeferred<AEnemy>(AEnemy::StaticClass(), Transform);
			if (!Test->TestNotNull(TEXT("[Chaos] Enemy spawned"), Enemy))
			{
				GameMode->DefenseTypeDataTable = SavedDefenseTable;
				GameMode->ElementalDataTable = SavedElementalTable;
				return;
			}

			UEnemyStatComponent* EnemyStats = NewObject<UEnemyStatComponent>(Enemy, TEXT("ChaosTestEnemyStats"));
			Enemy->AddInstanceComponent(EnemyStats);
			EnemyStats->DefenseTypes.AddTag(UStatComponent::GetTag(TEXT("DefenseType.Divine")));
			EnemyStats->Elements.AddTag(FireTag);
			EnemyStats->ElementResistRate.Add(FireTag, 0.5f);
			EnemyStats->SetBaseStat(UStatComponent::GetTag(TEXT("Stat.MaxHP")), 1000.f);
			EnemyStats->SetBaseStat(UStatComponent::GetTag(TEXT("Stat.HP")), 1000.f);
			EnemyStats->SetBaseStat(UStatComponent::GetTag(TEXT("Stat.Defense")), 100.f);
			EnemyStats->RegisterComponent();
			Enemy->FinishSpawning(Transform);

			UStatComponent* AttackerStats = NewObject<UStatComponent>(Enemy, TEXT("ChaosTestAttackerStats"));
			AttackerStats->AttackTypes.AddTag(UStatComponent::GetTag(TEXT("AttackTypes.Chaos")));
			AttackerStats->Elements.AddTag(FireTag);

			const float DefensePenetration = GameMode->GetUpgradeBonus(TDTags::Upgrade_DefensePenetration);
			const float EffectiveDefense = FMath::Max(0.f, 100.f * (1.f - DefensePenetration));
			const float ArmorMultiplier = 100.f / (100.f + EffectiveDefense);
			Enemy->ReceiveDamage(100.f, AttackerStats);
			Test->TestTrue(TEXT("[Chaos] Type, element matchup and resistance are ignored while numeric defense remains"),
				FMath::IsNearlyEqual(EnemyStats->GetStat(UStatComponent::GetTag(TEXT("Stat.HP"))),
					1000.f - 100.f * ArmorMultiplier, 0.01f));

			EnemyStats->SetBaseStat(UStatComponent::GetTag(TEXT("Stat.HP")), 1000.f);
			AttackerStats->AttackTypes.Reset();
			AttackerStats->AttackTypes.AddTag(UStatComponent::GetTag(TEXT("AttackTypes.Physical")));
			const float FireResistancePenetration = GameMode->GetUpgradeBonus(
				UStatComponent::GetTag(TEXT("Upgrade.ElementPenetration.Fire")));
			Enemy->ReceiveDamage(100.f, AttackerStats);
			Test->TestTrue(TEXT("[Chaos] Physical control still applies element, type and resistance multipliers"),
				FMath::IsNearlyEqual(EnemyStats->GetStat(UStatComponent::GetTag(TEXT("Stat.HP"))),
					1000.f - 100.f * 0.5f * 0.25f * ArmorMultiplier
						* (1.f - FMath::Max(0.f, 0.5f - FireResistancePenetration)), 0.01f));

			Enemy->Destroy();
			GameMode->DefenseTypeDataTable = SavedDefenseTable;
			GameMode->ElementalDataTable = SavedElementalTable;
		}

		AEnemy* SpawnTestEnemy(const FVector& Location, bool bAir)
		{
			const FTransform Transform(Location);
			AEnemy* Enemy = World->SpawnActorDeferred<AEnemy>(AEnemy::StaticClass(), Transform);
			if (!Enemy) return nullptr;

			UEnemyStatComponent* Stat = NewObject<UEnemyStatComponent>(Enemy);
			Stat->SetBaseStat(UStatComponent::GetTag(TEXT("Stat.MaxHP")), 100.f);
			Stat->SetBaseStat(UStatComponent::GetTag(TEXT("Stat.HP")), 100.f);
			Enemy->AddInstanceComponent(Stat);
			Stat->RegisterComponent();

			// 오버랩 쿼리(ECC_Pawn)에 잡히도록 충돌체 추가.
			USphereComponent* Body = NewObject<USphereComponent>(Enemy);
			Body->SetupAttachment(Enemy->GetRootComponent());
			Body->SetSphereRadius(30.f);
			Body->SetCollisionObjectType(ECC_Pawn);
			Body->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			Body->SetCollisionResponseToAllChannels(ECR_Overlap);
			Enemy->AddInstanceComponent(Body);
			Body->RegisterComponent();

			if (bAir)
			{
				UAirAbilityComponent* Air = NewObject<UAirAbilityComponent>(Enemy);
				Enemy->AddInstanceComponent(Air);
				Air->RegisterComponent();
			}
			Enemy->FinishSpawning(Transform);
			return Enemy;
		}

		void SetupExplosionRadius()
		{
			const FVector Center = TestLocation(7000.f);
			AreaTower = SpawnTowerDeferred<ABaseTower>(ABaseTower::StaticClass(), TestLocation(7000.f, 3000.f), [](ABaseTower* Tower)
			{
				Tower->TowerStatComponent->Abilities.AddTag(UStatComponent::GetTag(TEXT("Ability.AttackGround")));
			});
			if (!AreaTower.IsValid())
			{
				Test->AddError(TEXT("[P1-19②] Area tower failed to spawn"));
				return;
			}
			AreaTower->TowerStatComponent->SetBaseStat(UStatComponent::GetTag(TEXT("Stat.AttackDamage")), 10.f);
			AreaTower->TowerStatComponent->SetBaseStat(UStatComponent::GetTag(TEXT("Stat.AttackRange")), 800.f);

			UAreaAttackComponent* Area = NewObject<UAreaAttackComponent>(AreaTower.Get());
			AreaTower->AddInstanceComponent(Area);
			Area->RegisterComponent();
			AreaComponent = Area;

			AreaPrimary = SpawnTestEnemy(Center, false);
			AreaNear = SpawnTestEnemy(Center + FVector(150.f, 0.f, 0.f), false);     // 폭발 반경(200) 안
			AreaFar = SpawnTestEnemy(Center + FVector(400.f, 0.f, 0.f), false);      // 사거리(800) 안, 폭발 반경 밖
			AreaAir = SpawnTestEnemy(Center + FVector(0.f, 100.f, 0.f), true);       // 반경 안이지만 공중
		}

		void CheckExplosionRadius()
		{
			if (!AreaComponent.IsValid() || !AreaPrimary.IsValid() || !AreaNear.IsValid() || !AreaFar.IsValid() || !AreaAir.IsValid())
			{
				Test->AddError(TEXT("[P1-19②] Explosion test actors are missing"));
				return;
			}
			const FGameplayTag HPTag = UStatComponent::GetTag(TEXT("Stat.HP"));
			auto HP = [&HPTag](AEnemy* Enemy) { return Enemy->EnemyStatComponent ? Enemy->EnemyStatComponent->GetStat(HPTag) : -1.f; };

			AreaComponent->ExecuteAttack({ AreaPrimary.Get() });
			Test->TestEqual(TEXT("[P1-19②] Primary target takes explosion damage"), HP(AreaPrimary.Get()), 90.f);
			Test->TestEqual(TEXT("[P1-19②] Ground enemy inside ExplosionRadius takes damage"), HP(AreaNear.Get()), 90.f);
			Test->TestEqual(TEXT("[P1-19②] Enemy inside attack range but outside ExplosionRadius is not hit"), HP(AreaFar.Get()), 100.f);
			Test->TestEqual(TEXT("[P1-19②] Ground-only tower's explosion does not hit air enemies"), HP(AreaAir.Get()), 100.f);

			for (TWeakObjectPtr<AEnemy> Enemy : { AreaPrimary, AreaNear, AreaFar, AreaAir }) if (Enemy.IsValid()) Enemy->Destroy();
			if (AreaTower.IsValid()) AreaTower->Destroy();
		}

		void CheckPassiveInterval()
		{
			UDataTable* Table = NewObject<UDataTable>(GetTransientPackage());
			Table->RowStruct = FTowerDataRow::StaticStruct();
			FTowerDataRow Row{};
			Row.MaxLevel = 3;
			Row.BuffSetting.BuffType = EBuffComponentType::Passive;
			Row.BuffSetting.BuffTarget = EBuffTarget::AllyTower;
			Row.BuffSetting.BuffInterval = 3.f;
			Row.BuffSetting.BuffRadius = 300.f;
			Table->AddRow(TEXT("PassiveTower_Lv1"), Row);

			ABaseTower* Tower = World->SpawnActor<ABaseTower>(ABaseTower::StaticClass(), TestLocation(8000.f), FRotator::ZeroRotator);
			if (!Test->TestNotNull(TEXT("[P1-19③] Tower spawned"), Tower)) return;

			// Passive 컴포넌트가 먼저 BeginPlay해 기본 주기(1초)로 타이머가 걸린 상황.
			UPassiveBuffComponent* Passive = NewObject<UPassiveBuffComponent>(Tower);
			Tower->AddInstanceComponent(Passive);
			Passive->RegisterComponent();
			FTimerManager& Timers = World->GetTimerManager();
			Test->TestEqual(TEXT("[P1-19③] Passive timer starts with the default 1s interval"), Timers.GetTimerRate(Passive->BuffTimerHandle), 1.f);

			Tower->TowerStatComponent->TowerDataTable = Table;
			Tower->TowerStatComponent->TowerRowName = TEXT("PassiveTower");
			Tower->TowerStatComponent->CurrentLevel = 1;
			Tower->TowerStatComponent->InitFromDataTable();
			Test->TestEqual(TEXT("[P1-19③] Data-table BuffInterval (3s) is applied to the running timer"), Timers.GetTimerRate(Passive->BuffTimerHandle), 3.f);

			Tower->Destroy();
		}

		void CheckLevelUpRegistration()
		{
			AEpicTowerManager* EpicManager = Cast<AEpicTowerManager>(UGameplayStatics::GetActorOfClass(World.Get(), AEpicTowerManager::StaticClass()));
			if (!Test->TestNotNull(TEXT("[P1-19④] EpicTowerManager available"), EpicManager)) return;
			AGachaManager* Gacha = Cast<AGachaManager>(UGameplayStatics::GetActorOfClass(World.Get(), AGachaManager::StaticClass()));

			// 해금 조건: AMagicTower 2개. 필드에는 레벨업할 1개만 둔다.
			UDataTable* EpicTable = NewObject<UDataTable>(GetTransientPackage());
			EpicTable->RowStruct = FEpicTowerDataRow::StaticStruct();
			FEpicTowerDataRow EpicRow{};
			EpicRow.EpicTowerClass = ABuffTower::StaticClass();
			EpicRow.EpicType = EEpicTowerType::Unlock;
			FEpicTowerConditionEntry Condition{};
			Condition.TowerClass = AMagicTower::StaticClass();
			Condition.RequiredLevel = 1;
			EpicRow.Conditions = { Condition, Condition };
			EpicTable->AddRow(TEXT("TestUnlock"), EpicRow);

			UDataTable* TowerTable = NewObject<UDataTable>(GetTransientPackage());
			TowerTable->RowStruct = FTowerDataRow::StaticStruct();
			FTowerDataRow TowerRow{};
			TowerRow.MaxLevel = 3;
			TowerTable->AddRow(TEXT("TestMagic_Lv1"), TowerRow);
			TowerTable->AddRow(TEXT("TestMagic_Lv2"), TowerRow);

			UDataTable* SavedTable = EpicManager->EpicTowerDataTable;
			EpicManager->EpicTowerDataTable = EpicTable;
			EpicManager->UnlockedEpicTowers.Remove(ABuffTower::StaticClass());

			ATowerSlot* Slot = World->SpawnActor<ATowerSlot>(ATowerSlot::StaticClass(), TestLocation(9000.f), FRotator::ZeroRotator);
			AMagicTower* OldTower = SpawnTowerDeferred<AMagicTower>(AMagicTower::StaticClass(), TestLocation(9000.f), [TowerTable](AMagicTower* Tower)
			{
				Tower->TowerStatComponent->TowerDataTable = TowerTable;
				Tower->TowerStatComponent->TowerRowName = TEXT("TestMagic");
				Tower->TowerStatComponent->CurrentLevel = 1;
				Tower->NextLevelClass = AMagicTower::StaticClass();
			});
			if (Slot && OldTower)
			{
				OldTower->OwnerSlot = Slot;
				Slot->InstalledTower = OldTower;
				OldTower->TowerStatComponent->LevelUp();
				Test->TestTrue(TEXT("[P1-19④] Level up replaced the tower"), IsValid(Slot->InstalledTower) && Slot->InstalledTower != OldTower);
				Test->TestFalse(TEXT("[P1-19④] Leveling one tower does not satisfy a two-tower epic condition"),
					EpicManager->UnlockedEpicTowers.Contains(ABuffTower::StaticClass()));

				// 대조군: 실제로 두 번째 타워가 생기면 해금되어야 한다.
				AMagicTower* Second = World->SpawnActor<AMagicTower>(AMagicTower::StaticClass(), TestLocation(9000.f, 300.f), FRotator::ZeroRotator);
				Test->TestTrue(TEXT("[P1-19④] Control: a real second tower does unlock the epic"),
					EpicManager->UnlockedEpicTowers.Contains(ABuffTower::StaticClass()));
				if (Second) Second->Destroy();
				if (IsValid(Slot->InstalledTower)) Slot->InstalledTower->Destroy();
			}
			else
			{
				Test->AddError(TEXT("[P1-19④] Level-up test actors failed to spawn"));
			}
			if (Slot) Slot->Destroy();

			EpicManager->UnlockedEpicTowers.Remove(ABuffTower::StaticClass());
			if (Gacha) Gacha->EpicPool.Remove(ABuffTower::StaticClass());
			EpicManager->EpicTowerDataTable = SavedTable;
		}

		void CheckVictoryAndHeal()
		{
			ABase* Base = Cast<ABase>(UGameplayStatics::GetActorOfClass(World.Get(), ABase::StaticClass()));
			ACommandManager* Command = Cast<ACommandManager>(UGameplayStatics::GetActorOfClass(World.Get(), ACommandManager::StaticClass()));

			GameMode->TriggerVictory();
			Test->TestTrue(TEXT("[P1-13] TriggerVictory ends the game as a victory"), GameMode->bIsGameOver && GameMode->bIsVictory);

			TArray<UUserWidget*> Results;
			UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World.Get(), Results, UBattleResultWidget::StaticClass(), true);
			if (Test->TestEqual(TEXT("[P1-13] Battle result widget is shown"), Results.Num(), 1))
			{
				const UTextBlock* Title = Cast<UTextBlock>(Results[0]->GetWidgetFromName(TEXT("ResultTitle")));
				Test->TestTrue(TEXT("[P1-13] Result title says victory"), Title && Title->GetText().ToString() == TEXT("승리"));
			}

			if (Base && Command)
			{
				Base->BaseHp = Base->BaseMaxHp * 0.5f;
				const float HPBefore = Base->BaseHp;
				Command->HealCooldownRemaining = 0.f;
				Command->HealBase();
				Test->TestEqual(TEXT("[P1-20] Heal does nothing after the game is over"), Base->BaseHp, HPBefore);
			}
			else
			{
				Test->AddWarning(TEXT("[P1-20] Skipped heal check: no Base or CommandManager in this level."));
			}
		}

		FAutomationTestBase* Test;
		TWeakObjectPtr<UWorld> World;
		TWeakObjectPtr<ATowerDefenseGameMode> GameMode;
		TWeakObjectPtr<ABaseTower> BuffSource;
		TWeakObjectPtr<ABaseTower> NearTower;
		TWeakObjectPtr<ABaseTower> FarTower;
		TWeakObjectPtr<UPassiveBuffComponent> BuffComponent;
		TWeakObjectPtr<ABaseTower> AreaTower;
		TWeakObjectPtr<UAreaAttackComponent> AreaComponent;
		TWeakObjectPtr<AEnemy> AreaPrimary;
		TWeakObjectPtr<AEnemy> AreaNear;
		TWeakObjectPtr<AEnemy> AreaFar;
		TWeakObjectPtr<AEnemy> AreaAir;
		int32 Step = 0;
		double NextStepTime = 0.;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FP1FixesTest,
	"TowerDefense.Fixes.P1",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FP1FixesTest::RunTest(const FString& Parameters)
{
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.f));
	ADD_LATENT_AUTOMATION_COMMAND(FP1FixCheck(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif
