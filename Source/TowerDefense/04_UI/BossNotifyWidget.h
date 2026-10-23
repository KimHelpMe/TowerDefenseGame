#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BossNotifyWidget.generated.h"

UCLASS()
class TOWERDEFENSE_API UBossNotifyWidget : public UUserWidget
{
	GENERATED_BODY()
	

public:

	// 보스 네임 표시.
	UFUNCTION(BlueprintImplementableEvent)
	void SetNotifyText(const FString& Text);

	// 이미지 표시.
	UFUNCTION(BlueprintImplementableEvent)
	void SetBossPortrait(UTexture2D* Portrait);
};
