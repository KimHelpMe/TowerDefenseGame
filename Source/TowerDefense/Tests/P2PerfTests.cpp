#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "UObject/UObjectIterator.h"
#include "TowerDefenseLog.h"
#include "MainMenuWidget.h"
#include "TowerDefenseGameMode.h"
#include "Projectile.h"
#include "BaseTower.h"
#include "TowerSlot.h"
#include "Base.h"
#include "EnemySpawnSystem.h"
#include "EnemySplinePath.h"
#include "SynergyManager.h"
#include "EpicTowerManager.h"
#include "GachaManager.h"
#include "RoundManager.h"
#include "UpgradeItem.h"
#include "StatComponent.h"
#include "AttackTypeComponent.h"
#include "BuffTypeComponent.h"
#include "EnemyAbilityComponent.h"
#include "ShieldAbilityComponent.h"
#include "PassiveBuffComponent.h"

namespace
{
	// PIE 월드에서 P2(성능) 변경이 동작을 바꾸지 않았는지 확인한다.
	class FP2PerfCheck : public IAutomationLatentCommand
	{
	public:
		explicit FP2PerfCheck(FAutomationTestBase* InTest) : Test(InTest) {}

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
				TArray<UUserWidget*> Menus;
				UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World.Get(), Menus, UMainMenuWidget::StaticClass(), false);
				for (UUserWidget* Menu : Menus) Menu->RemoveFromParent();
				UGameplayStatics::SetGamePaused(World.Get(), false);

				CheckLogCategory();
				CheckDisabledTicks();
				CheckReplaceBuffsByKey();
				CheckPassiveReapply();
				SpawnMovingProjectile();
				Step = 1;
				NextStepTime = Now + 0.3;
				return false;
			}

			if (!World.IsValid())
			{
				Test->AddError(TEXT("PIE ended during the P2 test."));
				return true;
			}
			CheckProjectileMoved();
			return true;
		}

	private:
		void CheckLogCategory()
		{
			Test->TestTrue(TEXT("[P2-21] LogTowerDefense hides Verbose traces by default"),
				!LogTowerDefense.IsSuppressed(ELogVerbosity::Log) && LogTowerDefense.IsSuppressed(ELogVerbosity::Verbose));
		}

		void CheckDisabledTicks()
		{
			struct FTickCase { const TCHAR* Name; bool bCanEverTick; };
			const FTickCase Actors[] = {
				{ TEXT("Projectile"), GetDefault<AProjectile>()->PrimaryActorTick.bCanEverTick },
				{ TEXT("EnemySpawnSystem"), GetDefault<AEnemySpawnSystem>()->PrimaryActorTick.bCanEverTick },
				{ TEXT("EnemySplinePath"), GetDefault<AEnemySplinePath>()->PrimaryActorTick.bCanEverTick },
				{ TEXT("Base"), GetDefault<ABase>()->PrimaryActorTick.bCanEverTick },
				{ TEXT("EpicTowerManager"), GetDefault<AEpicTowerManager>()->PrimaryActorTick.bCanEverTick },
				{ TEXT("RoundManager"), GetDefault<ARoundManager>()->PrimaryActorTick.bCanEverTick },
				{ TEXT("SynergyManager"), GetDefault<ASynergyManager>()->PrimaryActorTick.bCanEverTick },
				{ TEXT("GachaManager"), GetDefault<AGachaManager>()->PrimaryActorTick.bCanEverTick },
				{ TEXT("TowerSlot"), GetDefault<ATowerSlot>()->PrimaryActorTick.bCanEverTick },
				{ TEXT("UpgradeItem"), GetDefault<AUpgradeItem>()->PrimaryActorTick.bCanEverTick },
				{ TEXT("TowerDefenseGameMode"), GetDefault<ATowerDefenseGameMode>()->PrimaryActorTick.bCanEverTick },
				{ TEXT("StatComponent"), GetDefault<UStatComponent>()->PrimaryComponentTick.bCanEverTick },
				{ TEXT("AttackTypeComponent"), GetDefault<UAttackTypeComponent>()->PrimaryComponentTick.bCanEverTick },
				{ TEXT("BuffTypeComponent"), GetDefault<UBuffTypeComponent>()->PrimaryComponentTick.bCanEverTick },
				{ TEXT("EnemyAbilityComponent"), GetDefault<UEnemyAbilityComponent>()->PrimaryComponentTick.bCanEverTick },
				{ TEXT("ShieldAbilityComponent"), GetDefault<UShieldAbilityComponent>()->PrimaryComponentTick.bCanEverTick },
			};
			for (const FTickCase& Case : Actors)
			{
				Test->TestFalse(FString::Printf(TEXT("[P2-26] %s no longer ticks"), Case.Name), Case.bCanEverTick);
			}
			Test->TestTrue(TEXT("[P2-26] Towers still tick (turret rotation)"), GetDefault<ABaseTower>()->PrimaryActorTick.bCanEverTick);

			// 정보: BP 하위 클래스 중 Tick이 다시 켜진 수(연결된 Event Tick이 있는 BP).
			int32 BlueprintProjectiles = 0;
			int32 TickingBlueprintProjectiles = 0;
			for (TObjectIterator<UClass> It; It; ++It)
			{
				if (!It->IsChildOf(AProjectile::StaticClass()) || *It == AProjectile::StaticClass() || It->HasAnyClassFlags(CLASS_Abstract)) continue;
				++BlueprintProjectiles;
				if (It->GetDefaultObject<AProjectile>()->PrimaryActorTick.bCanEverTick) ++TickingBlueprintProjectiles;
			}
			Test->AddInfo(FString::Printf(TEXT("[P2-26] Loaded projectile BPs: %d, still ticking via Event Tick: %d"), BlueprintProjectiles, TickingBlueprintProjectiles));
		}

		void CheckReplaceBuffsByKey()
		{
			ATowerSlot* Holder = World->SpawnActor<ATowerSlot>(ATowerSlot::StaticClass(), FVector(0.f, 0.f, -20000.f), FRotator::ZeroRotator);
			if (!Test->TestNotNull(TEXT("[P2-25] Holder spawned"), Holder)) return;
			UStatComponent* Stat = NewObject<UStatComponent>(Holder);
			const FGameplayTag Damage = UStatComponent::GetTag(TEXT("Stat.AttackDamage"));
			Stat->SetBaseStat(Damage, 10.f);

			const FName Key(TEXT("P2Key"));
			FBuffEffect Buff{};
			Buff.StatTag = Damage;
			Buff.Value = 5.f;
			Buff.ValueType = EBuffValueType::Flat;

			Stat->ReplaceBuffsByKey(Key, { Buff });
			Test->TestEqual(TEXT("[P2-25] ReplaceBuffsByKey adds the buff"), Stat->GetStat(Damage), 15.f);

			Stat->ReplaceBuffsByKey(Key, { Buff });
			Test->TestEqual(TEXT("[P2-25] Replacing with the same buff does not stack"), Stat->ActiveBuffs.Num(), 1);
			Test->TestEqual(TEXT("[P2-25] Stat unchanged after same-buff replace"), Stat->GetStat(Damage), 15.f);

			Buff.Value = 8.f;
			Stat->ReplaceBuffsByKey(Key, { Buff });
			Test->TestEqual(TEXT("[P2-25] Replacing with a different value updates the stat"), Stat->GetStat(Damage), 18.f);

			Stat->ReplaceBuffsByKey(Key, {});
			Test->TestEqual(TEXT("[P2-25] Replacing with no buffs removes them"), Stat->GetStat(Damage), 10.f);

			Stat->RemoveBuffByKey(TEXT("MissingKey"));
			Test->TestEqual(TEXT("[P2-25] Removing a missing key leaves stats intact"), Stat->GetStat(Damage), 10.f);
			Holder->Destroy();
		}

		void CheckPassiveReapply()
		{
			const FVector Origin(20000.f, 20000.f, -20000.f);
			ABaseTower* Source = World->SpawnActor<ABaseTower>(ABaseTower::StaticClass(), Origin, FRotator::ZeroRotator);
			ABaseTower* Target = World->SpawnActor<ABaseTower>(ABaseTower::StaticClass(), Origin + FVector(100.f, 0.f, 0.f), FRotator::ZeroRotator);
			if (!Test->TestNotNull(TEXT("[P2-25] Buff towers spawned"), Source) || !Target) return;

			UPassiveBuffComponent* Passive = NewObject<UPassiveBuffComponent>(Source);
			Passive->BuffRadius = 300.f;
			Passive->BuffTarget = EBuffTarget::AllyTower;
			FBuffEffect Effect{};
			Effect.StatTag = UStatComponent::GetTag(TEXT("Stat.AttackDamage"));
			Effect.Value = 3.f;
			Effect.ValueType = EBuffValueType::Flat;
			Passive->BuffEffects = { Effect, Effect };
			Source->AddInstanceComponent(Passive);
			Passive->RegisterComponent();

			PassiveSource = Source;
			PassiveTarget = Target;
			PassiveComponent = Passive;
		}

		void CheckPassiveAfterOverlap()
		{
			if (!PassiveComponent.IsValid() || !PassiveTarget.IsValid()) return;
			PassiveComponent->ApplyBuff();
			PassiveComponent->ApplyBuff();
			PassiveComponent->ApplyBuff();
			const int32 Count = PassiveTarget->TowerStatComponent->ActiveBuffs.Num();
			Test->TestEqual(TEXT("[P2-25] Repeated Passive application keeps exactly its two effects on the target"), Count, 2);
			if (PassiveSource.IsValid()) PassiveSource->Destroy();
			if (PassiveTarget.IsValid()) PassiveTarget->Destroy();
		}

		void SpawnMovingProjectile()
		{
			const FVector Start(0.f, 0.f, 60000.f);
			AProjectile* Projectile = World->SpawnActor<AProjectile>(AProjectile::StaticClass(), Start, FRotator(90.f, 0.f, 0.f));
			if (Test->TestNotNull(TEXT("[P2-26] Projectile spawned"), Projectile))
			{
				MovingProjectile = Projectile;
				ProjectileStart = Start;
			}
		}

		void CheckProjectileMoved()
		{
			CheckPassiveAfterOverlap();
			if (!MovingProjectile.IsValid())
			{
				Test->AddError(TEXT("[P2-26] Projectile disappeared before it could be checked"));
				return;
			}
			const float Distance = FVector::Dist(MovingProjectile->GetActorLocation(), ProjectileStart);
			Test->TestTrue(FString::Printf(TEXT("[P2-26] Projectile still moves without actor Tick (%.0f cm)"), Distance), Distance > 100.f);
			MovingProjectile->Destroy();
		}

		FAutomationTestBase* Test;
		TWeakObjectPtr<UWorld> World;
		TWeakObjectPtr<AProjectile> MovingProjectile;
		FVector ProjectileStart = FVector::ZeroVector;
		TWeakObjectPtr<ABaseTower> PassiveSource;
		TWeakObjectPtr<ABaseTower> PassiveTarget;
		TWeakObjectPtr<UPassiveBuffComponent> PassiveComponent;
		int32 Step = 0;
		double NextStepTime = 0.;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FP2PerfTest,
	"TowerDefense.Fixes.P2",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FP2PerfTest::RunTest(const FString& Parameters)
{
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.f));
	ADD_LATENT_AUTOMATION_COMMAND(FP2PerfCheck(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif
