#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EpicNotifyWidget.generated.h"


UCLASS()
class TOWERDEFENSE_API UEpicNotifyWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:

	UFUNCTION(BlueprintImplementableEvent)
	void SetNotifyText(const FString& Text);
};
