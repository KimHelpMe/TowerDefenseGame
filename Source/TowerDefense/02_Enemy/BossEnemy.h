#pragma once

#include "CoreMinimal.h"
#include "Enemy.h"
#include "Engine/Texture2D.h"
#include "BossEnemy.generated.h"

class UUserWidget;
class UBossAbilityComponent;
class UStatComponent;
class UBossNotifyWidget;
class UBossHPBarWidget;


UCLASS()
class TOWERDEFENSE_API ABossEnemy : public AEnemy
{
	GENERATED_BODY()
	
public:

	ABossEnemy();

	UPROPERTY(EditAnywhere)
	UBossAbilityComponent* BossAbilityComponent = nullptr;

    UPROPERTY(EditAnywhere)
	UTexture2D* BossPortrait = nullptr;

	UPROPERTY(EditAnywhere)
	FString BossName;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UBossHPBarWidget> BossHPBarWidgetClass;

	UPROPERTY(EditAnywhere)
	UBossHPBarWidget* BossHPBarWidget = nullptr;

	// Widget.
	UPROPERTY(EditAnywhere)
	TSubclassOf<UBossNotifyWidget> BossNotifyWidgetClass;

	UFUNCTION()
	void OnBossDeath();

public:

	virtual void BeginPlay() override;


	virtual void ReceiveDamage(float AmountDamage, UStatComponent* AttackerStat) override;
};
