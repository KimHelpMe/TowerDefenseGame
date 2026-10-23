#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "MainMenuWidget.generated.h"

class APlayerController;
class UButton;
class UTextBlock;

UCLASS()
class TOWERDEFENSE_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;

private:
	void BuildMenu();
	APlayerController* GetMenuPlayerController() const;
	UTextBlock* CreateMenuText(const FText& Text, int32 FontSize, const FLinearColor& Color, ETextJustify::Type Justification);

	UFUNCTION()
	void HandleStartClicked();

	UFUNCTION()
	void HandleQuitClicked();

	UPROPERTY(Transient)
	TObjectPtr<UButton> StartButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> QuitButton;
};
