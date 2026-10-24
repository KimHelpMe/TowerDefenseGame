#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BattleResultWidget.generated.h"
UCLASS()
class TOWERDEFENSE_API UBattleResultWidget : public UUserWidget
{
    GENERATED_BODY()
protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
private:
    UFUNCTION() void RetryBattle();
    UFUNCTION() void ReturnToMenu();
};
