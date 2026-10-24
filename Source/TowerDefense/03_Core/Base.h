#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Base.generated.h"

class UStatComponent;

UCLASS()
class TOWERDEFENSE_API ABase : public AActor
{
	GENERATED_BODY()
	
public:	

	ABase();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseHp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseMaxHp;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> BaseMeshComponent;

	UFUNCTION()
	void ReceiveDamage(float AmountDamage, UStatComponent* AttackerStat);


protected:

	virtual void BeginPlay() override;
	bool bIsDefeated = false;

public:	

	virtual void Tick(float DeltaTime) override;

};
