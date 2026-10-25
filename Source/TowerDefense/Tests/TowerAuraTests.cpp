#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "MainMenuWidget.h"
#include "BaseTower.h"

namespace
{
	// 나이아가라 이펙트가 지정된 타워는 오라가 끝나지 않고 계속 재생되어야 한다.
	// 공격 시 스폰되는 원본 이펙트(폭발 등)는 한 번 재생 후 끝나야 한다.
	class FTowerAuraCheck : public IAutomationLatentCommand
	{
	public:
		explicit FTowerAuraCheck(FAutomationTestBase* InTest) : Test(InTest) {}

		virtual bool Update() override
		{
			const double Now = FPlatformTime::Seconds();
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

				SpawnTowersWithEffects();

				// 공격 이펙트로도 쓰이는 원본(반복 없음)은 그대로 한 번만 재생되어야 한다.
				if (UNiagaraSystem* OneShot = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/Free_Magic/VFX_Niagara/NS_Free_Magic_Attack1.NS_Free_Magic_Attack1")))
				{
					OneShotEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(World.Get(), OneShot, FVector(0.f, 0.f, -30000.f));
				}

				StartTime = Now;
				Step = 1;
				return false;
			}

			if (!World.IsValid())
			{
				Test->AddError(TEXT("PIE ended during the tower aura test."));
				return true;
			}

			// 가장 긴 루프 길이(5초)를 넘겨 기다린 뒤 확인.
			if (Now - StartTime < 7.0) return false;

			int32 Active = 0;
			for (const TPair<FString, TWeakObjectPtr<ABaseTower>>& Entry : Towers)
			{
				const ABaseTower* Tower = Entry.Value.Get();
				const bool bActive = Tower && Tower->TowerEffect && Tower->TowerEffect->IsActive();
				Active += bActive ? 1 : 0;
				Test->TestTrue(FString::Printf(TEXT("[Aura] %s keeps its tower effect playing after 7s"), *Entry.Key), bActive);
			}
			Test->AddInfo(FString::Printf(TEXT("[Aura] %d / %d towers still playing their aura after 7s"), Active, Towers.Num()));
			Test->TestTrue(TEXT("[Aura] Found towers with a Niagara tower effect"), Towers.Num() > 0);

			Test->TestFalse(TEXT("[Aura] Original attack effect (NS_Free_Magic_Attack1) still finishes after one play"),
				OneShotEffect.IsValid() && OneShotEffect->IsActive());

			for (TPair<FString, TWeakObjectPtr<ABaseTower>>& Entry : Towers)
			{
				if (Entry.Value.IsValid()) Entry.Value->Destroy();
			}
			return true;
		}

	private:
		void SpawnTowersWithEffects()
		{
			// 타워 BP 중 TowerEffect에 나이아가라 시스템이 지정된 것만 스폰.
			IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
			TArray<FAssetData> Assets;
			Registry.GetAssetsByPath(FName(TEXT("/Game/01_BluePrints/00_TowerBP")), Assets, true);

			int32 Index = 0;
			for (const FAssetData& Asset : Assets)
			{
				if (!Asset.AssetName.ToString().StartsWith(TEXT("BP_")) || Asset.AssetName.ToString().Contains(TEXT("Projectile"))) continue;
				const FString ClassPath = Asset.GetObjectPathString() + TEXT("_C");
				UClass* TowerClass = LoadClass<ABaseTower>(nullptr, *ClassPath);
				if (!TowerClass) continue;

				const ABaseTower* Defaults = TowerClass->GetDefaultObject<ABaseTower>();
				if (!Defaults->TowerEffect || !Defaults->TowerEffect->GetAsset()) continue;

				const FVector Location(30000.f + (Index % 8) * 1500.f, 30000.f + (Index / 8) * 1500.f, -20000.f);
				++Index;
				if (ABaseTower* Tower = World->SpawnActor<ABaseTower>(TowerClass, Location, FRotator::ZeroRotator))
				{
					Towers.Add(Asset.AssetName.ToString(), Tower);
				}
			}
		}

		FAutomationTestBase* Test;
		TWeakObjectPtr<UWorld> World;
		TMap<FString, TWeakObjectPtr<ABaseTower>> Towers;
		TWeakObjectPtr<UNiagaraComponent> OneShotEffect;
		double StartTime = 0.;
		int32 Step = 0;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTowerAuraTest,
	"TowerDefense.Fixes.TowerAura",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTowerAuraTest::RunTest(const FString& Parameters)
{
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.f));
	ADD_LATENT_AUTOMATION_COMMAND(FTowerAuraCheck(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif
