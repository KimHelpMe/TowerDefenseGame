#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BossHPBarWidget.generated.h"


UCLASS()
class TOWERDEFENSE_API UBossHPBarWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:

	// HP Bar Update.
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateHPBar(float HPPercent);

	// Boss Name.
	UFUNCTION(BlueprintImplementableEvent)
	void SetBossName(const FString& Name);
};
