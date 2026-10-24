#include "BaseTower.h"
#include "TowerSlot.h"
#include "TowerStatComponent.h"
#include "TowerInfoWidget.h"
#include "TowerRangeDecalComponent.h"
#include "SynergyManager.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "EpicTowerManager.h"

ABaseTower::ABaseTower()
{

	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;


	RangeDecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("RangeDecalComponent"));
	RangeDecalComponent->SetupAttachment(RootComponent);
	RangeDecalComponent->SetVisibility(false);
	TowerRangeDecal = CreateDefaultSubobject<UTowerRangeDecalComponent>(TEXT("TowerRangeDecal"));


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

void ABaseTower::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 어떤 경로로 파괴되든 시너지 목록에서 빠지도록 해제.
	UWorld* World = GetWorld();
	if (World && !World->bIsTearingDown)
	{
		ASynergyManager* SynergyManager = Cast<ASynergyManager>(
			UGameplayStatics::GetActorOfClass(World, ASynergyManager::StaticClass()));
		if (IsValid(SynergyManager))
		{
			SynergyManager->UnregisterTower(this);
		}
	}

	Super::EndPlay(EndPlayReason);
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

	if (IsValid(CurrentTurretTarget))
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
        if (TowerRangeDecal) TowerRangeDecal->SetRangeVisible(false);
		TowerInfoWidget->CloseWidget();
		return;
	}

	UTowerStatComponent* TowerStat = GetComponentByClass<UTowerStatComponent>();
	if (!TowerStat) return;

	// 다른 타워의 정보창이 열려 있으면 닫아 정보창과 사거리 표시를 하나만 유지.
	TArray<UUserWidget*> OpenInfoWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, OpenInfoWidgets, UTowerInfoWidget::StaticClass(), true);
	for (UUserWidget* OpenWidget : OpenInfoWidgets)
	{
		if (UTowerInfoWidget* OtherInfo = Cast<UTowerInfoWidget>(OpenWidget)) OtherInfo->CloseWidget();
	}

	TSubclassOf<UTowerInfoWidget> InfoWidgetClass = TowerInfoWidgetClass;
	if (!InfoWidgetClass)
	{
		InfoWidgetClass = LoadClass<UTowerInfoWidget>(nullptr, TEXT("/Game/03_UI/WBP_TowerInfo.WBP_TowerInfo_C"));
	}
	if (!InfoWidgetClass) return;

	TowerInfoWidget = CreateWidget<UTowerInfoWidget>(GetWorld(), InfoWidgetClass);
	if (!TowerInfoWidget) return;

	TowerInfoWidget->InitWidget(TowerStat);
	TowerInfoWidget->AddToViewport();

    if (TowerRangeDecal) TowerRangeDecal->SetRangeVisible(true);
}
float ABaseTower::GetRangeRadius()
{
	return IsValid(TowerAttackRangeComponent) ? TowerAttackRangeComponent->GetScaledSphereRadius() : 0.f;
}
