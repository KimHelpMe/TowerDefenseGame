#include "Projectile.h"
#include "TowerDefenseTags.h"
#include "TowerDefenseLog.h"
#include "Enemy.h"
#include "TowerStatComponent.h"
#include "TowerDefenseGameMode.h"
#include "Kismet/GameplayStatics.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = false; // Tick does nothing; BP Event Tick re-enables it on compile.

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

	// 빗나간 탄이 월드에 계속 남지 않도록 수명 제한.
	InitialLifeSpan = 3.f;

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
	if (!IsValid(OtherActor) || Cast<AProjectile>(OtherActor)) return;

	UE_LOG(LogTowerDefense, Verbose, TEXT("OnHit called, OtherActor: %s"), *OtherActor->GetName());

	AEnemy* enemy = Cast<AEnemy>(OtherActor);

	ATowerDefenseGameMode* GameMode = Cast<ATowerDefenseGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	// TowerStatComponent는 UPROPERTY라 GC 후 null이 되므로 null 체크로 충분함.
	// 발사한 타워가 레벨업/합성으로 교체되어도 날아가던 탄의 피해는 유지하기 위해 IsValid는 쓰지 않음.
	if (IsValid(enemy) && TowerStatComponent)
	{
		// 물리/마법 공격 타입에 따른 업그레이드 보너스 적용.
		const float DamageBonus = GameMode ? GameMode->GetAttackDamageBonus(TowerStatComponent) : 0.f;
		float FinalDamage = TowerStatComponent->GetStat(TDTags::Stat_AttackDamage) * (1.f + DamageBonus);
		enemy->ReceiveDamage(FinalDamage, TowerStatComponent);
	}
	Destroy();
}


void AProjectile::OnProjectileStopped(const FHitResult& ImpactResult)
{
	UE_LOG(LogTowerDefense, Verbose, TEXT("OnProjectileStopped called"));
	Destroy();
}