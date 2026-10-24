#include "Enemy.h"
#include "TowerDefenseTags.h"
#include "TowerDefenseLog.h"
#include "Base.h"
#include "TowerDefenseGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "EnemyStatComponent.h"
#include "StatComponent.h"
#include "EnemySplinePath.h"
#include "ShieldAbilityComponent.h"
#include "SplitAbilityComponent.h"
#include "FBuffEffect.h"
#include "DefenseTypeDataRow.h"
#include "ElementalDataRow.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	EnemyMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("EnemyMeshComponent"));
	EnemyMeshComponent->SetupAttachment(RootComponent);
	EnemyMeshComponent->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	EnemyMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	EnemyMeshComponent->SetCollisionObjectType(ECC_Pawn);
	EnemyMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	EnemyMeshComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	EnemyMeshComponent->SetGenerateOverlapEvents(true);
	bReachedBase = false;

	//재화 획득
	GoldReward = 10.f;
	ExpReward = 1.f;

}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	// 본진 찾기.
	Base = Cast<ABase>(UGameplayStatics::GetActorOfClass(GetWorld(), ABase::StaticClass()));
	EnemyStatComponent = GetComponentByClass<UEnemyStatComponent>();
	ShieldComponent = GetComponentByClass<UShieldAbilityComponent>();
	SplitComponent = GetComponentByClass<USplitAbilityComponent>();

	if (EnemyStatComponent)
	{
		EnemyStatComponent->OnDeath.AddDynamic(this, &AEnemy::OnEnemyDeath);
	}
}


void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// splinePath 유효한지 확인.
	if (!splinePath) return;
	if (bReachedBase) return;
	if (!EnemyStatComponent) return;

	// 현재 이동한 적의 거리, 매 프레임 마다 Speed 만큼 증가.
	// DeltaTime 은 프레임 마다 시간이 다르기 때문에 일정 속도를 유지하기 위함.
	CurrentSplinePoint += EnemyStatComponent->GetStat(TDTags::Stat_MoveSpeed) * DeltaTime;

	// spline 위의 해당 거리 좌표를 가지고 옴.
	FVector targetLocation = splinePath->splineComponent->
		GetLocationAtDistanceAlongSpline(CurrentSplinePoint, ESplineCoordinateSpace::World);

	FVector targetDirection = splinePath->splineComponent->
		GetDirectionAtDistanceAlongSpline(CurrentSplinePoint, ESplineCoordinateSpace::World);

	// 적의 위치를 그 좌표로 이동.
	SetActorLocation(targetLocation);
	SetActorRotation(targetDirection.Rotation());

	// spline 전체 길이.
	float splineLength = splinePath->splineComponent->GetSplineLength();

	// 적이 spline 전체 길이에 도달 : 본진에 도달.
	if (CurrentSplinePoint >= splineLength)
	{
		bReachedBase = true;
		GetWorldTimerManager().SetTimer(
			EnemyAttackHandle,
			this,
			&AEnemy::AttackBase,
			EnemyStatComponent->GetStat(TDTags::Stat_AttackSpeed),
			true);
	}

}




void AEnemy::ReceiveDamage(float AmountDamage, UStatComponent* AttackerStat)
{
	UE_LOG(LogTowerDefense, Verbose, TEXT("ReceiveDamage called, Amount: %f"), AmountDamage);
	if (bIsDead) return;

	float FinalDamage = AmountDamage;

	ATowerDefenseGameMode* GameMode = Cast<ATowerDefenseGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	if (AttackerStat && EnemyStatComponent)
	{
		// ElementalMultiplier.
		float ElementalMultiplier = 1.0f;
		
		if (GameMode && GameMode->ElementalDataTable)
		{
			for (FGameplayTag AttackTag : AttackerStat->Elements)
			{
				// Attack Tag -> Row Key.
				FName RowKey = FName(*AttackTag.GetTagName().ToString());

				// ElementaldataTable에서 Row 찾기.
				FElementalDataRow* Row = GameMode->ElementalDataTable->FindRow<FElementalDataRow>(RowKey, TEXT(""));
				if (!Row) continue;

				// EnemyStatComponent -> Elements Tag로 배율 찾기.
				for (FGameplayTag DefendTag : EnemyStatComponent->Elements)
				{
					float* Multiplier = Row->Multipliers.Find(DefendTag);
					if (Multiplier)
					{
						ElementalMultiplier *= *Multiplier;
					}
				}
			}
		}
		FinalDamage = AmountDamage * ElementalMultiplier;

		// Defense Type Multiplier.

		float DefenseMultiplier = 1.0f;

		if (GameMode && GameMode->DefenseTypeDataTable)
		{
			for (FGameplayTag DefenseTag : EnemyStatComponent->DefenseTypes)
			{
				FName RowKey = FName(*DefenseTag.GetTagName().ToString());
				FDefenseTypeDataRow* Row = GameMode->DefenseTypeDataTable->FindRow<FDefenseTypeDataRow>(RowKey, TEXT(""));
				if (!Row) continue;

				// AttackerStat->AttackTypes 체크해서 배율 선택
				if (AttackerStat->AttackTypes.HasTag(TDTags::AttackTypes_Physical))
					DefenseMultiplier *= Row->PhysicalMultiplier;
				else if (AttackerStat->AttackTypes.HasTag(TDTags::AttackTypes_Magic))
					DefenseMultiplier *= Row->MagicMultiplier;
				else if (AttackerStat->AttackTypes.HasTag(TDTags::AttackTypes_Chaos))
					DefenseMultiplier *= Row->ChaosMultiplier;
			}
		}
		FinalDamage *= DefenseMultiplier;

		// 방어력 관통 적용
		float DefPenetration = GameMode ? GameMode->GetUpgradeBonus(
			TDTags::Upgrade_DefensePenetration) : 0.f;
		float EffectiveDefense = FMath::Max(0.f, EnemyStatComponent->GetStat(TDTags::Stat_Defense) * (1.f - DefPenetration));
		FinalDamage = FinalDamage * (100.f / (100.f + EffectiveDefense));

		// 속성 내성 관통 적용
		for (auto& Elem : EnemyStatComponent->ElementResistRate)
		{
			if (AttackerStat->Elements.HasTag(Elem.Key))
			{
				FString TagName = Elem.Key.GetTagName().ToString();
				FString LastPart = TagName.Contains(".") ? TagName.RightChop(TagName.Find(".") + 1) : TagName;
				FGameplayTag PenTag = UStatComponent::GetTag(("Upgrade.ElementPenetration." + LastPart));
				float Penetration = GameMode ? GameMode->GetUpgradeBonus(PenTag) : 0.f;
				FinalDamage *= (1.f - FMath::Max(0.f, Elem.Value - Penetration));
			}
		}
		UE_LOG(LogTowerDefense, Verbose, TEXT("FinalDamage: %f"), FinalDamage);
	}

	// 속성 저항이 1을 넘거나 배율 데이터가 음수면 피해가 음수가 될 수 있다.
	// 음수 피해가 실드를 늘리지 않도록 0 이상으로 제한.
	if (!FMath::IsFinite(FinalDamage) || FinalDamage <= 0.f) return;

	// 실드.
	if (ShieldComponent)
	{
		FinalDamage = ShieldComponent->AbsorbDamage(FinalDamage);
	}

	// HP 감소.
	if (EnemyStatComponent)
	{
		EnemyStatComponent->ApplyDamage(FinalDamage);
	}
}


void AEnemy::AttackBase()
{
	if (Base && IsValid(Base))
	{
		Base->ReceiveDamage(EnemyStatComponent->GetStat(TDTags::Stat_AttackDamage), EnemyStatComponent);
	}
}


void AEnemy::OnEnemyDeath()
{
	// 사망 후 제거 대기(1초) 중 다시 호출되면 보상·분열이 중복되므로 무시.
	if (bIsDead) return;
	bIsDead = true;
	bReachedBase = true;
	GetWorldTimerManager().ClearTimer(EnemyAttackHandle);

	ATowerDefenseGameMode* GameMode = Cast<ATowerDefenseGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (SplitComponent)
	{
		SplitComponent->OnEnemyDeathAbility();
	}
	if (GameMode)
	{
		GameMode->Gold += GoldReward;
		GameMode->AddExp(ExpReward);
	}

	GetWorldTimerManager().SetTimer(EnemyDeathHandle, this, &AEnemy::DestroyEnemy, 1.0f, false);
}

void AEnemy::DestroyEnemy()
{
	Destroy();
}
