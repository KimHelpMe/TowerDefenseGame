#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/DecalComponent.h"
#include "Gachable.h"
#include "TowerStatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"


#include "BaseTower.generated.h"

class ATowerSlot;
class UTowerInfoWidget;
class UTowerRangeDecalComponent;

UCLASS()
class TOWERDEFENSE_API ABaseTower : public AActor, public IGachable
{
	GENERATED_BODY()
	
public:	

	ABaseTower();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTowerStatComponent> TowerStatComponent;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> TowerMeshComponent;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> TurretStaticMeshComponent;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USkeletalMeshComponent> TurretSkeletalMeshComponent;

	UPROPERTY(EditAnywhere)
	FRotator TurretRotationOffset;

	// 보간 속도.
	UPROPERTY(EditAnywhere)
	float TurretRotationSpeed;

	// 바라볼 대상.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<AActor> CurrentTurretTarget;

	UFUNCTION()
	void UpdateTurretRotation(float DeltaTime);

	UFUNCTION()
	void SetTurretTarget(AActor* NewTarget);



	UPROPERTY(EditAnywhere)
	TObjectPtr<USceneComponent> MuzzleComponent;

	// Level Up 외형 BP.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ABaseTower> NextLevelClass;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAudioComponent> TowerSound;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraComponent> TowerEffect;

	// Slot.
	UPROPERTY()
	TObjectPtr<ATowerSlot> OwnerSlot;


	// FString 임시로 사용.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Element;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Grade;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TowerType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TowerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TowerInfo;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USphereComponent> TowerAttackRangeComponent;


	// Decal.

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UDecalComponent> RangeDecalComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Tower Range")
    TObjectPtr<UTowerRangeDecalComponent> TowerRangeDecal;

	UFUNCTION()
	virtual float GetRangeRadius();

public:

	UPROPERTY()
	TObjectPtr<UTowerInfoWidget> TowerInfoWidget = nullptr;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UTowerInfoWidget> TowerInfoWidgetClass;

	UFUNCTION()
	void OnTowerClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	// Gacha Interface Function


	virtual FString GetItemName() override;
	virtual FString GetItemGrade() override;
	virtual UTexture2D* GetItemIcon() override;
	virtual void OnSelected(ATowerSlot* Slot) override;



protected:

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:

	virtual void Tick(float DeltaTime) override;

public:

	// virtual 제거
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Attack();

	// _Implementation 선언 추가
	virtual void Attack_Implementation();

	UPROPERTY(BlueprintReadOnly)
	bool bIsAttacking;

	UFUNCTION()
	void EndAttack();
	

private:
		FTimerHandle AttackHandle;
};
