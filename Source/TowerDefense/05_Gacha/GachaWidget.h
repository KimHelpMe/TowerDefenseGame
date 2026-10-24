#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GachaCardInfo.h"
#include "GachaWidget.generated.h"

class UBorder;
class UCanvasPanel;

struct FGachaRarityRevealState
{
	TWeakObjectPtr<UUserWidget> Card;
	TWeakObjectPtr<UBorder> Frame;
	TWeakObjectPtr<UCanvasPanel> SparkCanvas;
	TArray<TWeakObjectPtr<UBorder>> Sparks;
	FLinearColor AccentColor = FLinearColor::White;
	FLinearColor OriginalFrameColor = FLinearColor::White;
	double StartTime = 0.0;
	float StartDelay = 0.0f;
	float Duration = 1.35f;
	bool bEpic = false;
};

UCLASS()
class TOWERDEFENSE_API UGachaWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;

public:

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateReloadUI(int32 ReloadCount, bool bCanAbandon);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateGachaUI(const TArray<FGachaCardInfo>& CardInfos);

	void PlayRarityReveal(const TArray<FGachaCardInfo>& CardInfos);

private:
	void ResetRarityReveals();

	TArray<FGachaRarityRevealState> ActiveRarityReveals;
};
