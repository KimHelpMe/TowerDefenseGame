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
#include "TowerStatComponent.h"
#include "AttackTypeComponent.h"
#include "AreaAttackComponent.h"
#include "TowerDefenseTags.h"

namespace
{
	const TCHAR* HitEffectPath = TEXT("/Game/Free_Magic/VFX_Niagara/NS_Free_Magic_Hit1.NS_Free_Magic_Hit1");

	// 공격 타워의 피격 이펙트 색이 속성에 맞는지, 범위 이펙트가 폭발 반경만큼 넓어지는지,
	// 피격 이펙트가 한 번 재생 후 끝나는지 확인.
	class FHitEffectCheck : public IAutomationLatentCommand
	{
	public:
		explicit FHitEffectCheck(FAutomationTestBase* InTest) : Test(InTest) {}

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

				SpawnAttackTowers();
				CheckHitColors();
				SpawnEffects();

				StartTime = Now;
				Step = 1;
				return false;
			}

			if (!World.IsValid())
			{
				Test->AddError(TEXT("PIE ended during the hit effect test."));
				return true;
			}

			if (Step == 1 && Now - StartTime >= 0.6)
			{
				LogAreaEffectSizes();
				Step = 2;
			}

			// 피격 이펙트(Hit1)는 반복 없이 한 번 재생 후 끝난다.
			const bool bHitPlaying = HitComp.IsValid() && HitComp->IsActive();
			if (bHitPlaying && Now - StartTime < 10.0) return false;

			Test->TestTrue(TEXT("[Hit] Hit effect was spawned"), bHitSpawned);
			Test->TestFalse(TEXT("[Hit] Hit effect finishes after one play"), bHitPlaying);
			Test->AddInfo(FString::Printf(TEXT("[Hit] Hit effect finished after %.1fs"), Now - StartTime));

			for (TWeakObjectPtr<ABaseTower>& Tower : Towers)
			{
				if (Tower.IsValid()) Tower->Destroy();
			}
			return true;
		}

	private:
		void SpawnAttackTowers()
		{
			IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
			TArray<FAssetData> Assets;
			Registry.GetAssetsByPath(FName(TEXT("/Game/01_BluePrints/00_TowerBP/01_Attack")), Assets, true);

			int32 Index = 0;
			for (const FAssetData& Asset : Assets)
			{
				const FString Name = Asset.AssetName.ToString();
				if (!Name.StartsWith(TEXT("BP_")) || Name.Contains(TEXT("Projectile"))) continue;
				UClass* TowerClass = LoadClass<ABaseTower>(nullptr, *(Asset.GetObjectPathString() + TEXT("_C")));
				if (!TowerClass) continue;

				const FVector Location(30000.f + (Index % 10) * 1500.f, -30000.f - (Index / 10) * 1500.f, -20000.f);
				++Index;
				if (ABaseTower* Tower = World->SpawnActor<ABaseTower>(TowerClass, Location, FRotator::ZeroRotator))
				{
					Towers.Add(Tower);
					TowerNames.Add(Name);
				}
			}
			Test->TestTrue(TEXT("[Hit] Spawned attack towers"), Towers.Num() > 0);
		}

		void CheckHitColors()
		{
			const UNiagaraSystem* Hit1 = LoadObject<UNiagaraSystem>(nullptr, HitEffectPath);
			Test->TestNotNull(TEXT("[Hit] NS_Free_Magic_Hit1 exists"), Hit1);

			int32 Checked = 0;
			for (int32 i = 0; i < Towers.Num(); ++i)
			{
				ABaseTower* Tower = Towers[i].Get();
				UAttackTypeComponent* Attack = Tower ? Tower->GetComponentByClass<UAttackTypeComponent>() : nullptr;
				UTowerStatComponent* Stat = Tower ? Tower->GetComponentByClass<UTowerStatComponent>() : nullptr;
				if (!Attack || !Stat) continue;
				++Checked;

				Test->TestTrue(FString::Printf(TEXT("[Hit] %s uses NS_Free_Magic_Hit1 as hit effect"), *TowerNames[i]), Attack->HitEffect == Hit1);

				FLinearColor Expected = Attack->NoElementHitColor;
				for (const TPair<FGameplayTag, FLinearColor>& Entry : Attack->ElementHitColors)
				{
					if (Stat->Elements.HasTag(Entry.Key)) { Expected = Entry.Value; break; }
				}
				Test->TestTrue(FString::Printf(TEXT("[Hit] %s hit color matches its element (%s)"), *TowerNames[i], *Stat->Elements.ToStringSimple()),
					Attack->GetHitColor().Equals(Expected));

				if (TowerNames[i] == TEXT("BP_AC02_Lv1"))
				{
					Test->TestTrue(TEXT("[Hit] BP_AC02_Lv1 is a fire tower"), Stat->Elements.HasTag(TDTags::Element_Fire));
					FireColor = Attack->GetHitColor();
					FireAttack = Attack;
				}
				if (TowerNames[i] == TEXT("BP_AC13_Lv1"))
				{
					Test->TestTrue(TEXT("[Hit] BP_AC13_Lv1 has no element and uses the no-element color"), Attack->GetHitColor().Equals(Attack->NoElementHitColor));
				}
			}
			Test->AddInfo(FString::Printf(TEXT("[Hit] Checked hit colors of %d attack towers"), Checked));
			Test->TestTrue(TEXT("[Hit] Found BP_AC02_Lv1"), FireAttack.IsValid());

			// 속성마다 다른 색이어야 한다.
			if (FireAttack.IsValid())
			{
				TSet<FString> Colors;
				for (const TPair<FGameplayTag, FLinearColor>& Entry : FireAttack->ElementHitColors) Colors.Add(Entry.Value.ToString());
				Colors.Add(FireAttack->NoElementHitColor.ToString());
				Test->TestEqual(TEXT("[Hit] Four elements plus no-element use five distinct colors"), Colors.Num(), 5);
			}
		}

		void SpawnEffects()
		{
			UNiagaraSystem* Hit1 = LoadObject<UNiagaraSystem>(nullptr, HitEffectPath);
			HitComp = UAttackTypeComponent::SpawnHitEffect(World.Get(), Hit1, FVector(0.f, 0.f, -30000.f), FireColor);
			bHitSpawned = HitComp.IsValid();
			if (HitComp.IsValid())
			{
				const FNiagaraVariable SparksVar(FNiagaraTypeDefinition::GetColorDef(), TEXT("User.Color_Sparks1"));
				const FLinearColor Sparks = HitComp->GetOverrideParameters().GetParameterValue<FLinearColor>(SparksVar);
				Test->TestTrue(TEXT("[Hit] Hit effect sparks use the fire color at the original brightness"),
					Sparks.Equals(FLinearColor(FireColor.R * 100.f, FireColor.G * 100.f, FireColor.B * 100.f, FireColor.A)));
			}

			const FNiagaraVariable ScaleAllVar(FNiagaraTypeDefinition::GetFloatDef(), TEXT("User.Scale_All"));
			int32 AreaCount = 0;
			for (int32 i = 0; i < Towers.Num(); ++i)
			{
				ABaseTower* Tower = Towers[i].Get();
				UAreaAttackComponent* Area = Tower ? Tower->GetComponentByClass<UAreaAttackComponent>() : nullptr;
				if (!Area || !Area->AreaEffect) continue;
				++AreaCount;

				const FVector Center = Tower->GetActorLocation() + FVector(0.f, 0.f, 500.f);
				UNiagaraComponent* Scaled = Area->SpawnAreaEffect(Center);
				Test->TestNotNull(FString::Printf(TEXT("[Area] %s spawns its area effect"), *TowerNames[i]), Scaled);
				if (!Scaled) continue;

				const float DefaultScale = Area->AreaEffect->GetExposedParameters().GetParameterValue<float>(ScaleAllVar);
				const float AppliedScale = Scaled->GetOverrideParameters().GetParameterValue<float>(ScaleAllVar);
				const float ExpectedScale = DefaultScale * Area->ExplosionRadius / Area->AreaEffectBaseRadius;
				Test->TestTrue(FString::Printf(TEXT("[Area] %s Scale_All %.2f follows explosion radius %.0f (expected %.2f)"),
					*TowerNames[i], AppliedScale, Area->ExplosionRadius, ExpectedScale), FMath::IsNearlyEqual(AppliedScale, ExpectedScale, 0.01f));
				Test->TestTrue(FString::Printf(TEXT("[Area] %s area effect is wider than its default size"), *TowerNames[i]),
					Area->ExplosionRadius <= Area->AreaEffectBaseRadius || AppliedScale > DefaultScale);

				// 크기 비교용: 같은 이펙트를 기본 크기로도 재생.
				UNiagaraComponent* Unscaled = UNiagaraFunctionLibrary::SpawnSystemAtLocation(World.Get(), Area->AreaEffect, Center + FVector(0.f, 0.f, 5000.f));
				AreaEffects.Add({ TowerNames[i], Area->ExplosionRadius, Scaled, Unscaled });
			}
			Test->TestTrue(TEXT("[Area] Found area towers with an area effect"), AreaCount > 0);
		}

		void LogAreaEffectSizes()
		{
			for (const FAreaEntry& Entry : AreaEffects)
			{
				auto Extent = [](const TWeakObjectPtr<UNiagaraComponent>& Comp)
				{
					if (!Comp.IsValid()) return 0.f;
					const FVector Box = Comp->CalcBounds(Comp->GetComponentTransform()).BoxExtent;
					return static_cast<float>(FMath::Max(Box.X, Box.Y));
				};
				Test->AddInfo(FString::Printf(TEXT("[Area] %s radius %.0f: default extent %.0f, scaled extent %.0f"),
					*Entry.Name, Entry.Radius, Extent(Entry.Unscaled), Extent(Entry.Scaled)));
			}
		}

		struct FAreaEntry
		{
			FString Name;
			float Radius;
			TWeakObjectPtr<UNiagaraComponent> Scaled;
			TWeakObjectPtr<UNiagaraComponent> Unscaled;
		};

		FAutomationTestBase* Test;
		TWeakObjectPtr<UWorld> World;
		TArray<TWeakObjectPtr<ABaseTower>> Towers;
		TArray<FString> TowerNames;
		TArray<FAreaEntry> AreaEffects;
		TWeakObjectPtr<UAttackTypeComponent> FireAttack;
		FLinearColor FireColor = FLinearColor::White;
		TWeakObjectPtr<UNiagaraComponent> HitComp;
		bool bHitSpawned = false;
		double StartTime = 0.;
		int32 Step = 0;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHitEffectTest,
	"TowerDefense.Fixes.HitEffect",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHitEffectTest::RunTest(const FString& Parameters)
{
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.f));
	ADD_LATENT_AUTOMATION_COMMAND(FHitEffectCheck(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif
