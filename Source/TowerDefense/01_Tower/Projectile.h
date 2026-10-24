#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Projectile.generated.h"

class UTowerStatComponent;

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

	UFUNCTION()
	void OnProjectileStopped(const FHitResult& ImpactResult);

protected:

	virtual void BeginPlay() override;

public:	

	virtual void Tick(float DeltaTime) override;

};
