#include "BattleResultWidget.h"
#include "TowerDefenseGameMode.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
void UBattleResultWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetIsFocusable(true);
    if (UButton* Button=Cast<UButton>(GetWidgetFromName(TEXT("RetryButton")))) Button->OnClicked.AddUniqueDynamic(this,&UBattleResultWidget::RetryBattle);
    if (UButton* Button=Cast<UButton>(GetWidgetFromName(TEXT("MenuButton")))) Button->OnClicked.AddUniqueDynamic(this,&UBattleResultWidget::ReturnToMenu);
    if (ATowerDefenseGameMode* Mode=Cast<ATowerDefenseGameMode>(UGameplayStatics::GetGameMode(this)))
    {
        const int32 Seconds=FMath::FloorToInt(GetWorld()->GetTimeSeconds());
        const FString Report=FString::Printf(TEXT("도달 라운드  %d / 30\n배치 타워  %d\n전투 시간  %02d:%02d"),Mode->RoundManager ? Mode->RoundManager->CurrentRound : 0,Mode->CurrentTowerSlot,Seconds/60,Seconds%60);
        if (UTextBlock* Text=Cast<UTextBlock>(GetWidgetFromName(TEXT("ResultStats")))) Text->SetText(FText::FromString(Report));
    }
    UGameplayStatics::SetGamePaused(this,true);
    if (APlayerController* PC=GetWorld()->GetFirstPlayerController()) { FInputModeUIOnly Input; Input.SetWidgetToFocus(TakeWidget()); Input.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); PC->SetInputMode(Input); }
}
void UBattleResultWidget::RetryBattle()
{
    UGameplayStatics::SetGamePaused(this,false);
    UGameplayStatics::OpenLevel(this,FName(*UGameplayStatics::GetCurrentLevelName(this)),true,TEXT("StartBattle=1"));
}
void UBattleResultWidget::ReturnToMenu()
{
    UGameplayStatics::SetGamePaused(this,false);
    UGameplayStatics::OpenLevel(this,FName(*UGameplayStatics::GetCurrentLevelName(this)));
}

void UBattleResultWidget::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
    Super::NativeTick(Geometry, DeltaTime);
    float X=0.f,Y=0.f;
    if (!UWidgetLayoutLibrary::GetMousePositionScaledByDPI(GetWorld()->GetFirstPlayerController(),X,Y)) return;
    TArray<UUserWidget*> Widgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this,Widgets,UUserWidget::StaticClass(),true);
    for (UUserWidget* Widget:Widgets)
        if (Widget && Widget->GetClass()->GetName().Contains(TEXT("WBP_Cursor"))) Widget->SetPositionInViewport(FVector2D(X,Y),false);
}