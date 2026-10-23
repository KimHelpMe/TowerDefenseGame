

#include "BossEnemy.h"
#include "BossAbilityComponent.h"
#include "EnemyStatComponent.h"
#include "BossNotifyWidget.h"
#include "BossHPBarWidget.h"


ABossEnemy::ABossEnemy()
{
	BossName = TEXT("None");
}


void ABossEnemy::BeginPlay()
{
	Super::BeginPlay();

	BossAbilityComponent = GetComponentByClass<UBossAbilityComponent>();
	
	if (BossNotifyWidgetClass)
	{
		UBossNotifyWidget* BossWidget = CreateWidget<UBossNotifyWidget>(GetWorld(), BossNotifyWidgetClass);
		if (BossWidget)
		{
			BossWidget->AddToViewport();
			BossWidget->SetNotifyText(BossName);

			if (BossPortrait)
			{
				BossWidget->SetBossPortrait(BossPortrait);
			}

			FTimerHandle BossNotifyTimerHandle;

			GetWorldTimerManager().SetTimer(BossNotifyTimerHandle, [BossWidget]()
				{
					if (IsValid(BossWidget))
					{
						BossWidget->RemoveFromParent();
					}
				}, 3.f, false);
		}
	}

	// BossHPBar Widget.
	
	if (BossHPBarWidgetClass)
	{
		BossHPBarWidget = CreateWidget<UBossHPBarWidget>(GetWorld(), BossHPBarWidgetClass);

		if (BossHPBarWidget)
		{
			BossHPBarWidget->AddToViewport();
			BossHPBarWidget->SetBossName(BossName);
		}
	}

	UEnemyStatComponent* EnemyStatComp = GetComponentByClass<UEnemyStatComponent>();
	if (EnemyStatComp)
	{
		EnemyStatComp->OnDeath.AddDynamic(this, &ABossEnemy::OnBossDeath);
	}
}


void ABossEnemy::ReceiveDamage(float AmountDamage, UStatComponent* AttackerStat)
{
	Super::ReceiveDamage(AmountDamage, AttackerStat);
	if (!IsValid(this)) return;

	UEnemyStatComponent* BossStat = GetComponentByClass<UEnemyStatComponent>();
	if (!BossStat) return;
	if (!BossAbilityComponent) return;

	float HP = BossStat->GetStat(UStatComponent::GetTag("Stat.HP"));
	float MaxHP = BossStat->GetStat(UStatComponent::GetTag("Stat.MaxHP"));
	float HPPercent = (HP / MaxHP) * 100.f;

	if (HPPercent <= 50.f && BossAbilityComponent->CurrentPhase == 1)
		{
			BossAbilityComponent->OnPhaseChanged(2);
		}

	if (HPPercent <= 10.f && BossAbilityComponent->CurrentPhase == 2)
	{
		BossAbilityComponent->OnPhaseChanged(3);
	}

	if (BossHPBarWidget)
	{
		BossHPBarWidget->UpdateHPBar(HPPercent);
	}
}

void ABossEnemy::OnBossDeath()
{
	if (BossHPBarWidget)
	{
		BossHPBarWidget->RemoveFromParent();
	}
}