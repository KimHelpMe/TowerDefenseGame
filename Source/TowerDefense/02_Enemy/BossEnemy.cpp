

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
	if (IsValid(BossAbilityComponent) && BossAbilityComponent->CurrentPhase < 1)
	{
		BossAbilityComponent->CurrentPhase = 1;
	}
	
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

	// HP바를 WBP 기본값이 아닌 실제 체력 비율로 시작.
	if (BossHPBarWidget)
	{
		const UEnemyStatComponent* BossStat = GetComponentByClass<UEnemyStatComponent>();
		const float MaxHP = BossStat ? BossStat->GetStat(UStatComponent::GetTag("Stat.MaxHP")) : 0.f;
		const float HP = BossStat ? BossStat->GetStat(UStatComponent::GetTag("Stat.HP")) : 0.f;
		const float HPPercent = FMath::IsFinite(MaxHP) && MaxHP > KINDA_SMALL_NUMBER
			? FMath::Clamp(HP / MaxHP * 100.f, 0.f, 100.f)
			: 100.f;
		BossHPBarWidget->UpdateHPBar(HPPercent);
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
	if (!IsValid(this) || bIsDead) return;

	UEnemyStatComponent* BossStat = GetComponentByClass<UEnemyStatComponent>();
	if (!IsValid(BossStat) || !IsValid(BossAbilityComponent)) return;

	const float MaxHP = BossStat->GetStat(UStatComponent::GetTag("Stat.MaxHP"));
	if (!FMath::IsFinite(MaxHP) || MaxHP <= KINDA_SMALL_NUMBER) return;

	const float HP = BossStat->GetStat(UStatComponent::GetTag("Stat.HP"));
	const float HPPercent = FMath::Clamp((HP / MaxHP) * 100.f, 0.f, 100.f);

	// 한 번에 50% 초과 → 10% 이하로 떨어져도 3페이즈까지 순서대로 진입.
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

void ABossEnemy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 사망 외의 경로(레벨 전환, 외부 Destroy 등)로 사라져도 HP바가 화면에 남지 않도록 정리.
	if (IsValid(BossHPBarWidget))
	{
		BossHPBarWidget->RemoveFromParent();
	}

	Super::EndPlay(EndPlayReason);
}
