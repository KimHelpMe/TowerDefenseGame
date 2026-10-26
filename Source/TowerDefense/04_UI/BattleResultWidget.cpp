#include "BattleResultWidget.h"
#include "TowerDefenseGameMode.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/GameplayStatics.h"

namespace
{
    struct FResultStat { const TCHAR* Label; FString Value; };

    // 한 줄 통계 텍스트(ResultStats) 자리에 라벨/값 카드 3칸을 만든다. ResultStats는 숨겨 둔 채 남긴다.
    void BuildStatCards(UTextBlock* Legacy,const TArray<FResultStat>& Stats)
    {
        UUserWidget* Owner=Legacy ? Legacy->GetTypedOuter<UUserWidget>() : nullptr;
        UPanelWidget* Parent=Legacy ? Legacy->GetParent() : nullptr;
        if (!Owner || !Owner->WidgetTree || !Parent || Owner->GetWidgetFromName(TEXT("TD_ResultStatRow"))) return;

        UHorizontalBox* Row=Owner->WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(),TEXT("TD_ResultStatRow"));
        for (int32 Index=0;Index<Stats.Num();++Index)
        {
            UBorder* Card=Owner->WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(),MakeUniqueObjectName(Owner->WidgetTree,UBorder::StaticClass(),TEXT("TD_ResultStatCard")));
            Card->SetBrushColor(FLinearColor(FColor(11,9,17,235)));
            Card->SetPadding(FMargin(8.f,12.f));
            UVerticalBox* Content=Owner->WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(),MakeUniqueObjectName(Owner->WidgetTree,UVerticalBox::StaticClass(),TEXT("TD_ResultStatContent")));
            Card->SetContent(Content);
            auto MakeText=[Owner,Legacy](const FString& Value,int32 Size,const FLinearColor& Color)
            {
                UTextBlock* Text=Owner->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),MakeUniqueObjectName(Owner->WidgetTree,UTextBlock::StaticClass(),TEXT("TD_ResultStatText")));
                FSlateFontInfo Font=Legacy->GetFont(); Font.Size=Size; Text->SetFont(Font);
                Text->SetText(FText::FromString(Value)); Text->SetColorAndOpacity(FSlateColor(Color));
                Text->SetJustification(ETextJustify::Center);
                return Text;
            };
            Content->AddChildToVerticalBox(MakeText(Stats[Index].Label,15,FLinearColor(FColor(155,143,120))))->SetHorizontalAlignment(HAlign_Center);
            UVerticalBoxSlot* ValueSlot=Content->AddChildToVerticalBox(MakeText(Stats[Index].Value,28,FLinearColor(FColor(239,225,189))));
            ValueSlot->SetHorizontalAlignment(HAlign_Center);
            ValueSlot->SetPadding(FMargin(0.f,6.f,0.f,0.f));
            UHorizontalBoxSlot* CardSlot=Row->AddChildToHorizontalBox(Card);
            CardSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
            CardSlot->SetPadding(FMargin(Index>0 ? 5.f : 0.f,0.f,Index<Stats.Num()-1 ? 5.f : 0.f,0.f));
        }
        // 이미 만들어진 위젯에서는 InsertChildAt이 실제 Slate 순서에 반영되지 않아 맨 아래로 붙는다.
        // 자식을 슬롯 설정째로 떼었다가 ResultStats 바로 뒤에 카드 줄을 끼워 다시 붙인다.
        TArray<TPair<UWidget*,UPanelSlot*>> Children;
        for (int32 Index=0;Index<Parent->GetChildrenCount();++Index)
        {
            UWidget* Child=Parent->GetChildAt(Index);
            Children.Add({Child,Child->Slot});
            if (Child==Legacy) Children.Add({Row,nullptr});
        }
        Parent->ClearChildren();
        for (const TPair<UWidget*,UPanelSlot*>& Child:Children)
        {
            if (Child.Value) Parent->AddChild(Child.Key,Child.Value);
            else Parent->AddChild(Child.Key);
        }
        Legacy->SetVisibility(ESlateVisibility::Collapsed);
    }
}
void UBattleResultWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetIsFocusable(true);
    if (UButton* Button=Cast<UButton>(GetWidgetFromName(TEXT("RetryButton")))) Button->OnClicked.AddUniqueDynamic(this,&UBattleResultWidget::RetryBattle);
    if (UButton* Button=Cast<UButton>(GetWidgetFromName(TEXT("MenuButton")))) Button->OnClicked.AddUniqueDynamic(this,&UBattleResultWidget::ReturnToMenu);
    if (ATowerDefenseGameMode* Mode=Cast<ATowerDefenseGameMode>(UGameplayStatics::GetGameMode(this)))
    {
        if (Mode->bIsVictory)
        {
            if (UTextBlock* Title=Cast<UTextBlock>(GetWidgetFromName(TEXT("ResultTitle")))) Title->SetText(FText::FromString(TEXT("승리")));
            if (UTextBlock* Subtitle=Cast<UTextBlock>(GetWidgetFromName(TEXT("ResultSubtitle")))) Subtitle->SetText(FText::FromString(TEXT("모든 라운드를 방어했습니다.")));
        }
        const int32 Seconds=FMath::FloorToInt(GetWorld()->GetTimeSeconds());
        const FString RoundValue=FString::Printf(TEXT("%d / %d"),Mode->RoundManager ? Mode->RoundManager->CurrentRound : 0,Mode->RoundManager ? Mode->RoundManager->MaxRound : 0);
        const FString TowerValue=FString::FromInt(Mode->CurrentTowerSlot);
        const FString TimeValue=FString::Printf(TEXT("%02d:%02d"),Seconds/60,Seconds%60);
        const FString Report=FString::Printf(TEXT("도달 라운드  %s\n배치 타워  %s\n전투 시간  %s"),*RoundValue,*TowerValue,*TimeValue);
        if (UTextBlock* Text=Cast<UTextBlock>(GetWidgetFromName(TEXT("ResultStats"))))
        {
            Text->SetText(FText::FromString(Report));
            BuildStatCards(Text,{{TEXT("도달 라운드"),RoundValue},{TEXT("배치 타워"),TowerValue},{TEXT("전투 시간"),TimeValue}});
        }
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
    // 커서 위젯은 한 번 찾아 캐시하고, 없을 때만 0.5초 간격으로 다시 찾는다(매 프레임 전체 위젯 검색 방지).
    if (!CursorWidget.IsValid() && FPlatformTime::Seconds()>=NextCursorSearchTime)
    {
        NextCursorSearchTime=FPlatformTime::Seconds()+0.5;
        TArray<UUserWidget*> Widgets;
        UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this,Widgets,UUserWidget::StaticClass(),true);
        for (UUserWidget* Widget:Widgets)
            if (Widget && Widget->GetClass()->GetName().Contains(TEXT("WBP_Cursor"))) { CursorWidget=Widget; break; }
    }
    if (UUserWidget* CursorUI=CursorWidget.Get()) CursorUI->SetPositionInViewport(FVector2D(X,Y),false);
}