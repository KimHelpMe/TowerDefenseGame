#include "Projectile.h"
#include "Enemy.h"
#include "TowerStatComponent.h"
#include "TowerDefenseGameMode.h"
#include "Kismet/GameplayStatics.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	// SphereComponent 생성
	sphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	RootComponent = sphereComponent;
	sphereComponent->SetGenerateOverlapEvents(true);
	sphereComponent->SetCollisionObjectType(ECC_WorldDynamic);
	sphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	sphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	sphereComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);  // Overlap → Block으로 변경

	// ProjectileMoveComponent 생성. Projectile은 RootComponent에 붙일 필요 없음.
	projectileMoveComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComponent"));
	projectileMoveComp->InitialSpeed = 5000.f;
	projectileMoveComp->MaxSpeed = 5000.f;
	projectileMoveComp->bRotationFollowsVelocity = true;

	projectileMoveComp->bShouldBounce = false;
	projectileMoveComp->ProjectileGravityScale = 0.f; // 중력 제거


}


void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	sphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::OnHit);
	projectileMoveComp->OnProjectileStop.AddDynamic(this, &AProjectile::OnProjectileStopped);
}


void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProjectile::OnHit(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (Cast<AProjectile>(OtherActor)) return;

	UE_LOG(LogTemp, Warning, TEXT("OnHit called, OtherActor: %s"), *OtherActor->GetName());

	AEnemy* enemy = Cast<AEnemy>(OtherActor);

	ATowerDefenseGameMode* GameMode = Cast<ATowerDefenseGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	float DamageBonus = 0.f;

	
	if (GameMode && TowerStatComponent)
	{
		// 물리/마법 공격 타입에 따라 보너스 적용
		if (TowerStatComponent->AttackTypes.HasTag(
			UStatComponent::GetTag("AttackTypes.Physical")))
		{
			DamageBonus = GameMode->GetUpgradeBonus(
				UStatComponent::GetTag("Upgrade.AttackDamage.Physical"));
		}
		else if (TowerStatComponent->AttackTypes.HasTag(
			UStatComponent::GetTag("AttackTypes.Magic")))
		{
			DamageBonus = GameMode->GetUpgradeBonus(
				UStatComponent::GetTag("Upgrade.AttackDamage.Magic"));
		}
	}

	if (enemy && IsValid(enemy) && TowerStatComponent)
	{
		float FinalDamage = TowerStatComponent->GetStat(UStatComponent::GetTag("Stat.AttackDamage")) * (1.f + DamageBonus);
		enemy->ReceiveDamage(FinalDamage, TowerStatComponent);
	}
	Destroy();
}


void AProjectile::OnProjectileStopped(const FHitResult& ImpactResult)
{
	UE_LOG(LogTemp, Warning, TEXT("OnProjectileStopped called"));
	Destroy();
}