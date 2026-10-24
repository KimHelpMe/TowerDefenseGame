
#pragma once

#include "CoreMinimal.h"
#include "StatComponent.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "EnemyStatComponent.generated.h"



UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TOWERDEFENSE_API UEnemyStatComponent : public UStatComponent
{
	GENERATED_BODY()
	

public:

	UEnemyStatComponent();

	UPROPERTY(EditAnywhere)
	TObjectPtr<UDataTable> EnemyDataTable;

	UPROPERTY(EditAnywhere)
	FName EnemyRowName;

	UFUNCTION()
	void InitFromDataTable();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString EnemyGrade;

	UPROPERTY(EditAnywhere)
	TMap<FGameplayTag, float> ElementResistRate;


protected:
	virtual void BeginPlay() override;
};
