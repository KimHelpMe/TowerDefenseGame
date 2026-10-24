#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Enemy.generated.h"

class AEnemySplinePath;
class ABase;
class ATowerDefenseGameMode;
class UEnemyStatComponent;
class UStatComponent;
class UShieldAbilityComponent;
class USplitAbilityComponent;

UCLASS()
class TOWERDEFENSE_API AEnemy : public AActor
{
	GENERATED_BODY()
	
public:	

	AEnemy();
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<ABase> Base;

	// AEnemySplinePath를 참조하기 위한 변수 선언.
	UPROPERTY(EditAnywhere)
	TObjectPtr<AEnemySplinePath> splinePath;

	// Spline 안에서 이동한 적의 현재 위치.
	UPROPERTY(VisibleAnywhere)
	float CurrentSplinePoint;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USkeletalMeshComponent> EnemyMeshComponent;

	// 재화 보상
	UPROPERTY(EditAnywhere)
	float GoldReward;

	UPROPERTY(EditAnywhere)
	float ExpReward;


	UPROPERTY(EditAnywhere)
	TObjectPtr<UEnemyStatComponent> EnemyStatComponent;

	// 실드량
	UPROPERTY(EditAnywhere)
	TObjectPtr<UShieldAbilityComponent> ShieldComponent;

	// 분열
	UPROPERTY(EditAnywhere)
	TObjectPtr<USplitAbilityComponent> SplitComponent;


protected:

	virtual void BeginPlay() override;

public:	

	virtual void Tick(float DeltaTime) override;

private:
	FTimerHandle EnemyAttackHandle;

	FTimerHandle EnemyDeathHandle;

public:
	virtual void ReceiveDamage(float AmountDamage, UStatComponent* AttackerStat);

	void AttackBase();

	// 적 사망.
	UFUNCTION()
	void OnEnemyDeath();

	// Spline 도착 후 이동 중지를 위한 플래그 변수
	bool bReachedBase;

	// 사망.
	UPROPERTY(BlueprintReadOnly)
	bool bIsDead;

	UFUNCTION()
	void DestroyEnemy();
};
