#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "UObject/UObjectGlobals.h"
#include "MainMenuWidget.h"
#include "BaseTower.h"
#include "SynergyManager.h"
#include "GachaManager.h"
#include "TowerSlot.h"
#include "Projectile.h"

namespace
{
	// PIE 월드에서 P0 수정 사항(시너지 해제, 슬롯 포인터, 투사체 수명, IsValid 가드)을 확인한다.
	class FP0FixCheck : public IAutomationLatentCommand
	{
	public:
		explicit FP0FixCheck(FAutomationTestBase* InTest) : Test(InTest) {}

		virtual bool Update() override
		{
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

				CheckSynergyUnregister();
				CheckGachaSlotPointer();
				CheckTurretTarget();
				SpawnProjectiles();
				StartRealTime = FPlatformTime::Seconds();
				Step = 1;
				return false;
			}

			if (!World.IsValid())
			{
				Test->AddError(TEXT("PIE ended during the P0 test."));
				return true;
			}

			// 투사체 수명(3초)이 지날 때까지 월드를 진행시킨다.
			if (World->GetTimeSeconds() < ProjectileSpawnTime + 3.5f)
			{
				if (FPlatformTime::Seconds() - StartRealTime > 20.0)
				{
					Test->AddError(TEXT("PIE world did not advance; projectile lifespan could not be checked."));
					return true;
				}
				return false;
			}

			Test->TestFalse(TEXT("[P0-3] Native projectile is destroyed after its lifespan"), NativeProjectile.IsValid());
			if (bSpawnedBlueprintProjectile)
			{
				Test->TestFalse(TEXT("[P0-3] Blueprint projectile is destroyed after its lifespan"), BlueprintProjectile.IsValid());
			}
			return true;
		}

	private:
		FVector TestLocation(float OffsetY = 0.f) const
		{
			// 레벨의 게임플레이 영역과 겹치지 않는 위치.
			return FVector(0.f, OffsetY, -20000.f);
		}

		void CheckSynergyUnregister()
		{
			ASynergyManager* SynergyManager = Cast<ASynergyManager>(
				UGameplayStatics::GetActorOfClass(World.Get(), ASynergyManager::StaticClass()));
			if (!SynergyManager)
			{
				SynergyManager = World->SpawnActor<ASynergyManager>(ASynergyManager::StaticClass(), TestLocation(), FRotator::ZeroRotator);
			}
			if (!Test->TestNotNull(TEXT("[P0-1] SynergyManager available"), SynergyManager)) return;

			ABaseTower* Tower = World->SpawnActor<ABaseTower>(ABaseTower::StaticClass(), TestLocation(), FRotator::ZeroRotator);
			if (!Test->TestNotNull(TEXT("[P0-1] Tower spawned"), Tower)) return;
			Test->TestTrue(TEXT("[P0-1] Spawned tower registers itself"), SynergyManager->FieldTowers.Contains(Tower));

			Tower->Destroy();
			Test->TestFalse(TEXT("[P0-1] Destroyed tower is unregistered by EndPlay"), SynergyManager->FieldTowers.Contains(Tower));

			// 해제 없이 파괴된 타워가 목록에 남은 상황을 재현: GC 후 원소가 null이 된다.
			ABaseTower* StaleTower = World->SpawnActor<ABaseTower>(ABaseTower::StaticClass(), TestLocation(), FRotator::ZeroRotator);
			StaleTower->Destroy();
			SynergyManager->FieldTowers.Add(StaleTower);
			CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS, true);
			Test->TestTrue(TEXT("[P0-1] GC nulls the stale FieldTowers entry (crash precondition)"), SynergyManager->FieldTowers.Contains(nullptr));

			ABaseTower* NextTower = World->SpawnActor<ABaseTower>(ABaseTower::StaticClass(), TestLocation(), FRotator::ZeroRotator);
			Test->TestFalse(TEXT("[P0-1] Registering after GC prunes null entries without crashing"), SynergyManager->FieldTowers.Contains(nullptr));
			Test->TestTrue(TEXT("[P0-1] New tower is registered"), SynergyManager->FieldTowers.Contains(NextTower));
			if (NextTower) NextTower->Destroy();
		}

		void CheckGachaSlotPointer()
		{
			AGachaManager* GachaManager = Cast<AGachaManager>(
				UGameplayStatics::GetActorOfClass(World.Get(), AGachaManager::StaticClass()));
			if (!GachaManager)
			{
				GachaManager = World->SpawnActor<AGachaManager>(AGachaManager::StaticClass(), TestLocation(), FRotator::ZeroRotator);
			}
			if (!Test->TestNotNull(TEXT("[P0-2] GachaManager available"), GachaManager)) return;

			ATowerSlot* const SavedSlot = GachaManager->CurrentSlot;
			ATowerSlot* Slot = World->SpawnActor<ATowerSlot>(ATowerSlot::StaticClass(), TestLocation(), FRotator::ZeroRotator);
			if (!Test->TestNotNull(TEXT("[P0-2] Slot spawned"), Slot)) return;

			GachaManager->CurrentSlot = Slot;
			Slot->Destroy();
			CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS, true);
			Test->TestTrue(TEXT("[P0-2] GC clears CurrentSlot after the slot is destroyed"), GachaManager->CurrentSlot == nullptr);

			GachaManager->CurrentSlot = IsValid(SavedSlot) ? SavedSlot : nullptr;
		}

		void CheckTurretTarget()
		{
			ABaseTower* Tower = World->SpawnActor<ABaseTower>(ABaseTower::StaticClass(), TestLocation(), FRotator::ZeroRotator);
			UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
			if (!Test->TestNotNull(TEXT("[P0-4] Tower spawned"), Tower) || !Test->TestNotNull(TEXT("[P0-4] Cube mesh loaded"), Cube)) return;
			Tower->TurretStaticMeshComponent->SetStaticMesh(Cube);

			// 타워 기준 +Y(Yaw 90) 방향에 타깃 배치.
			ATowerSlot* Target = World->SpawnActor<ATowerSlot>(ATowerSlot::StaticClass(), TestLocation(1000.f), FRotator::ZeroRotator);
			if (!Test->TestNotNull(TEXT("[P0-4] Target spawned"), Target)) return;

			Tower->SetTurretTarget(Target);
			Tower->UpdateTurretRotation(10.f);
			const float YawToTarget = Tower->TurretStaticMeshComponent->GetComponentRotation().Yaw;
			Test->TestTrue(FString::Printf(TEXT("[P0-4] Turret faces a live target (yaw %.1f)"), YawToTarget), FMath::IsNearlyEqual(YawToTarget, 90.f, 1.f));

			Target->Destroy();
			Tower->UpdateTurretRotation(10.f);
			const float YawAfterDestroy = Tower->TurretStaticMeshComponent->GetComponentRotation().Yaw;
			Test->TestTrue(FString::Printf(TEXT("[P0-4] Turret ignores a destroyed target and returns to default (yaw %.1f)"), YawAfterDestroy), FMath::IsNearlyEqual(YawAfterDestroy, 0.f, 1.f));

			Tower->Destroy();
		}

		void SpawnProjectiles()
		{
			// 위쪽으로 발사해 아무것도 맞지 않는 상황 = 빗나간 탄.
			const FRotator Up(90.f, 0.f, 0.f);
			const FVector High(0.f, 0.f, 50000.f);

			AProjectile* GuardProjectile = World->SpawnActor<AProjectile>(AProjectile::StaticClass(), High, Up);
			if (Test->TestNotNull(TEXT("[P0-4] Projectile spawned for OnHit guard"), GuardProjectile))
			{
				GuardProjectile->OnHit(nullptr, nullptr, nullptr, 0, false, FHitResult());
				Test->TestTrue(TEXT("[P0-4] OnHit with a null OtherActor is ignored without crashing"), IsValid(GuardProjectile));
				GuardProjectile->Destroy();
			}

			AProjectile* Native = World->SpawnActor<AProjectile>(AProjectile::StaticClass(), High, Up);
			if (Test->TestNotNull(TEXT("[P0-3] Native projectile spawned"), Native))
			{
				Test->TestEqual(TEXT("[P0-3] InitialLifeSpan default is 3 seconds"), Native->InitialLifeSpan, 3.f);
				Test->TestTrue(TEXT("[P0-3] Lifespan timer is running"), Native->GetLifeSpan() > 0.f);
				NativeProjectile = Native;
			}

			if (UClass* BlueprintClass = LoadClass<AProjectile>(nullptr, TEXT("/Game/01_BluePrints/BP_Projectile_Fire.BP_Projectile_Fire_C")))
			{
				AProjectile* Blueprint = World->SpawnActor<AProjectile>(BlueprintClass, High + FVector(500.f, 0.f, 0.f), Up);
				if (Test->TestNotNull(TEXT("[P0-3] Blueprint projectile spawned"), Blueprint))
				{
					Test->TestEqual(TEXT("[P0-3] Blueprint projectile inherits the 3 second lifespan"), Blueprint->InitialLifeSpan, 3.f);
					BlueprintProjectile = Blueprint;
					bSpawnedBlueprintProjectile = true;
				}
			}

			ProjectileSpawnTime = World->GetTimeSeconds();
		}

		FAutomationTestBase* Test;
		TWeakObjectPtr<UWorld> World;
		TWeakObjectPtr<AProjectile> NativeProjectile;
		TWeakObjectPtr<AProjectile> BlueprintProjectile;
		bool bSpawnedBlueprintProjectile = false;
		float ProjectileSpawnTime = 0.f;
		double StartRealTime = 0.;
		int32 Step = 0;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FP0FixesTest,
	"TowerDefense.Fixes.P0",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FP0FixesTest::RunTest(const FString& Parameters)
{
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.f));
	ADD_LATENT_AUTOMATION_COMMAND(FP0FixCheck(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif
