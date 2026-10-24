// 적 분열 기능
#pragma once

#include "CoreMinimal.h"
#include "EnemyAbilityComponent.h"
#include "SplitAbilityComponent.generated.h"

class AEnemy;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TOWERDEFENSE_API USplitAbilityComponent : public UEnemyAbilityComponent
{
	GENERATED_BODY()
	

public:

	USplitAbilityComponent();

	// 분열 시 생설될 적 수
	UPROPERTY(EditAnywhere)
	int32 SplitCount;
	// Distance (cm) between split children along the spline.
	UPROPERTY(EditAnywhere)
	float SplitSpacing = 50.f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AEnemy> SplitEnemyClass;


public:

	virtual void OnEnemyDeathAbility() override;
};
