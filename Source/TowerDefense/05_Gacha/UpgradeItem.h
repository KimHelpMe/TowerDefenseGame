#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FBuffEffect.h"
#include "Gachable.h"
#include "UpgradeItem.generated.h"

class ATowerSlot;

UCLASS()
class TOWERDEFENSE_API AUpgradeItem : public AActor, public IGachable
{
	GENERATED_BODY()
	
public:	

	AUpgradeItem();

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString UpgradeName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString UpgradeInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Grade;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> UpgradeIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FBuffEffect> UpgradeEffects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxCount;


public:
	// Gacha

	virtual FString GetItemName() override;
	virtual FString GetItemGrade() override;
	virtual UTexture2D* GetItemIcon() override;
	virtual void OnSelected(ATowerSlot* Slot) override;


public:

	UFUNCTION()
	void ApplyUpgradeEffects();


protected:

	virtual void BeginPlay() override;

public:	

	virtual void Tick(float DeltaTime) override;




};
