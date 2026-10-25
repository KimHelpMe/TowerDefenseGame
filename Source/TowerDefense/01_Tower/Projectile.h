#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Projectile.generated.h"

class UTowerStatComponent;
class UNiagaraSystem;

UCLASS()
class TOWERDEFENSE_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	

	AProjectile();


	UPROPERTY(EditAnywhere)
	TObjectPtr<UProjectileMovementComponent> projectileMoveComp;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USphereComponent> sphereComponent;


	UFUNCTION()
	void OnHit(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UPROPERTY()
	TObjectPtr<UTowerStatComponent> TowerStatComponent;

	// 몬스터 피격 시 재생할 타격 이펙트와 색. 발사한 공격 컴포넌트가 지정한다.
	UPROPERTY()
	TObjectPtr<UNiagaraSystem> HitEffect;

	FLinearColor HitColor = FLinearColor::White;

	UFUNCTION()
	void OnProjectileStopped(const FHitResult& ImpactResult);

protected:

	virtual void BeginPlay() override;

public:	

	virtual void Tick(float DeltaTime) override;

};
