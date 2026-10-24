#include "BaseTower.h"
#include "TowerSlot.h"
#include "TowerStatComponent.h"
#include "TowerInfoWidget.h"
#include "SynergyManager.h"
#include "Kismet/GameplayStatics.h"
#include "EpicTowerManager.h"
#include "UObject/ConstructorHelpers.h"

ABaseTower::ABaseTower()
{

	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;


	RangeDecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("RangeDecalComponent"));
	RangeDecalComponent->SetupAttachment(RootComponent);
	RangeDecalComponent->SetVisibility(false);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> RangeMaterial(TEXT("/Game/05_Materials/M_RangeDecal.M_RangeDecal"));
	if (RangeMaterial.Succeeded())
	{
		RangeDecalComponent->SetDecalMaterial(RangeMaterial.Object);
	}


	TowerMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TowerMeshComponent"));
	TowerMeshComponent->SetupAttachment(RootComponent);

	MuzzleComponent = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleComponent"));
	MuzzleComponent->SetupAttachment(RootComponent);

	TurretStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TurretStaticMeshComponent"));
	TurretStaticMeshComponent->SetupAttachment(RootComponent);

	TurretSkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TurretSkeletalMeshComponent"));
	TurretSkeletalMeshComponent->SetupAttachment(RootComponent);
	TurretRotationSpeed = 5.f;

	TowerAttackRangeComponent = CreateDefaultSubobject<USphereComponent>(TEXT("TowerAttackRangeComponent"));
	TowerAttackRangeComponent->SetupAttachment(RootComponent);
	TowerAttackRangeComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TowerAttackRangeComponent->SetCollisionObjectType(ECC_WorldDynamic);
	TowerAttackRangeComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	TowerAttackRangeComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TowerAttackRangeComponent->SetGenerateOverlapEvents(true);
	TowerMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TowerMeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);


	TowerSound = CreateDefaultSubobject<UAudioComponent>(TEXT("TowerSound"));
	TowerSound->SetupAttachment(RootComponent);
	TowerEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TowerEffect"));
	TowerEffect->SetupAttachment(RootComponent);

	TowerStatComponent = CreateDefaultSubobject<UTowerStatComponent>(TEXT("TowerStatComponent"));

	Element = TEXT("None");
	Grade = TEXT("None");
	TowerType = TEXT("None");
	TowerName = TEXT("None");
	TowerInfo = TEXT("None");
}

#if WITH_EDITOR
void ABaseTower::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	UTowerStatComponent* TowerStat = GetComponentByClass<UTowerStatComponent>();
	if (TowerStat && TowerAttackRangeComponent)
	{
		TowerAttackRangeComponent->SetSphereRadius(TowerStat->GetStat(FGameplayTag::RequestGameplayTag("Stat.AttackRange")));
	}
}
#endif



void ABaseTower::BeginPlay()
{
	Super::BeginPlay();
    for (UPrimitiveComponent* Component : { static_cast<UPrimitiveComponent*>(TowerMeshComponent), static_cast<UPrimitiveComponent*>(TurretStaticMeshComponent), static_cast<UPrimitiveComponent*>(TurretSkeletalMeshComponent) })
        if (Component && !Component->OnClicked.IsBound()) Component->OnClicked.AddUniqueDynamic(this, &ABaseTower::OnTowerClicked);


	ASynergyManager* SynergyManager = Cast<ASynergyManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ASynergyManager::StaticClass()));
	if (SynergyManager)
	{
		SynergyManager->RegisterTower(this);
	}
	if (TurretSkeletalMeshComponent && TurretSkeletalMeshComponent->GetSkeletalMeshAsset())
	{
		MuzzleComponent->AttachToComponent(
			TurretSkeletalMeshComponent,
			FAttachmentTransformRules::KeepWorldTransform);
	}
	else if (TurretStaticMeshComponent && TurretStaticMeshComponent->GetStaticMesh())
	{
		MuzzleComponent->AttachToComponent(
			TurretStaticMeshComponent,
			FAttachmentTransformRules::KeepWorldTransform);
	}
}


void ABaseTower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateTurretRotation(DeltaTime);
}


// 메쉬 회전.
void ABaseTower::UpdateTurretRotation(float DeltaTime)
{
	USceneComponent* TurretToRotate = nullptr;

	if (TurretSkeletalMeshComponent && TurretSkeletalMeshComponent->GetSkeletalMeshAsset())
	{
		TurretToRotate = TurretSkeletalMeshComponent;
	}
	else if (TurretStaticMeshComponent && TurretStaticMeshComponent->GetStaticMesh())
	{
		TurretToRotate = TurretStaticMeshComponent;
	}

	if (TurretToRotate == nullptr) return;

	if (CurrentTurretTarget)
	{
		FVector TargetLocation = CurrentTurretTarget->GetActorLocation();
		FVector MyLocation = GetActorLocation();
		FVector Direction = (TargetLocation - MyLocation).GetSafeNormal();
		FRotator TargetRotator = Direction.Rotation() + TurretRotationOffset;

		FRotator CurrentRotation = TurretToRotate->GetComponentRotation();
		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotator, DeltaTime, TurretRotationSpeed);
		TurretToRotate->SetWorldRotation(NewRotation);
	}
	else
	{
		FRotator DefaultRotation = GetActorRotation();
		FRotator CurrentRotation = TurretToRotate->GetComponentRotation();
		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, DefaultRotation, DeltaTime, TurretRotationSpeed);
		TurretToRotate->SetWorldRotation(NewRotation);
	}

}

void ABaseTower::SetTurretTarget(AActor* NewTarget)
{
	CurrentTurretTarget = NewTarget;
}






void ABaseTower::Attack_Implementation()
{
	bIsAttacking = true;
	GetWorldTimerManager().SetTimer(AttackHandle, this, &ABaseTower::EndAttack, 1.0f, false);
}

void ABaseTower::EndAttack()
{
	bIsAttacking = false;
}


// Gacha

FString ABaseTower::GetItemName()
{
	UTowerStatComponent* TowerStat = GetComponentByClass<UTowerStatComponent>();
	if (TowerStat && !TowerStat->TowerName.IsEmpty())
	{
		return TowerStat->TowerName;
	}
	return FString(TowerName);
}

FString ABaseTower::GetItemGrade()
{
	return FString(Grade);
}

UTexture2D* ABaseTower::GetItemIcon()
{
	return nullptr;
}

void ABaseTower::OnSelected(ATowerSlot* Slot)
{
	if (!Slot) return;
	SetActorLocation(Slot->GetActorLocation());
}



// Tower Info.
void ABaseTower::OnTowerClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
	if (TowerInfoWidget && TowerInfoWidget->IsInViewport())
	{
		if (RangeDecalComponent)
		{
			RangeDecalComponent->SetVisibility(false);
		}
		TowerInfoWidget->CloseWidget();
		return;
	}

	UTowerStatComponent* TowerStat = GetComponentByClass<UTowerStatComponent>();
	if (!TowerStat || !TowerInfoWidgetClass) return;

	TowerInfoWidget = CreateWidget<UTowerInfoWidget>(GetWorld(), TowerInfoWidgetClass);
	if (!TowerInfoWidget) return;

	TowerInfoWidget->InitWidget(TowerStat);
	TowerInfoWidget->AddToViewport();

	const float Range = GetRangeRadius();
	if (RangeDecalComponent && Range > 0.f)
	{
		RangeDecalComponent->DecalSize = FVector(100.f, Range, Range);
		RangeDecalComponent->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
		RangeDecalComponent->SetVisibility(true);
	}
	else if (RangeDecalComponent)
	{
		RangeDecalComponent->SetVisibility(false);
	}
}
float ABaseTower::GetRangeRadius()
{
	return IsValid(TowerAttackRangeComponent) ? TowerAttackRangeComponent->GetScaledSphereRadius() : 0.f;
}
