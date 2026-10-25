#include "FantasyUITheme.h"
#include "UIEffectFormatting.h"
#include "SynergyManager.h"
#include "GachaManager.h"
#include "UObject/UnrealType.h"
#include "TowerStatComponent.h"
#include "BaseTower.h"
#include "Kismet/GameplayStatics.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/ScrollBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"

namespace
{
	// "잿불과 금박" 팔레트: 픽셀 아이콘의 칠흑 배경·금박 트림·속성 발광색에서 가져왔다(sRGB 값으로 적고 선형으로 변환).
	const FLinearColor PanelColor(FColor(23, 19, 32, 240));
	const FLinearColor InsetColor(FColor(29, 24, 40, 245));
	const FLinearColor GoldColor(FColor(217, 165, 74));
	const FLinearColor GoldFrameColor(FColor(184, 138, 62));
	const FLinearColor CyanColor(FColor(63, 162, 255));
	const FLinearColor PrimaryTextColor(FColor(239, 225, 189));
	const FLinearColor MutedTextColor(FColor(155, 143, 120));
	const FLinearColor HealthColor(FColor(111, 220, 122));
	const FLinearColor SegmentFrameColor(FColor(58, 48, 72));
	const FLinearColor SegmentEmptyColor(FColor(42, 35, 56));

	// 테마는 0.5초마다 다시 적용되므로, 값이 같을 때 Set을 호출하면 매번 Slate 무효화(재그리기)가 일어난다.
	// 아래 함수들은 실제로 값이 바뀔 때만 위젯에 반영한다.
	bool SameColor(const FLinearColor& A, const FLinearColor& B)
	{
		return A.Equals(B, 1.e-4f);
	}

	void SetCursorIfChanged(UWidget* Widget, EMouseCursor::Type Cursor)
	{
		if (!Widget->bOverride_Cursor || Widget->GetCursor() != Cursor) Widget->SetCursor(Cursor);
	}

	void SetBrushColorIfChanged(UBorder* Border, const FLinearColor& Color)
	{
		if (Border && !SameColor(Border->GetBrushColor(), Color)) Border->SetBrushColor(Color);
	}

	void SetButtonColorIfChanged(UButton* Button, const FLinearColor& Color)
	{
		if (Button && !SameColor(Button->GetBackgroundColor(), Color)) Button->SetBackgroundColor(Color);
	}

	void SetFillColorIfChanged(UProgressBar* ProgressBar, const FLinearColor& Color)
	{
		if (ProgressBar && !SameColor(ProgressBar->GetFillColorAndOpacity(), Color)) ProgressBar->SetFillColorAndOpacity(Color);
	}

	void SetTextColorIfChanged(UTextBlock* Text, const FLinearColor& Color)
	{
		if (!Text) return;
		const FSlateColor Current = Text->GetColorAndOpacity();
		if (!Current.IsColorSpecified() || !SameColor(Current.GetSpecifiedColor(), Color)) Text->SetColorAndOpacity(FSlateColor(Color));
	}

	void SetTextIfChanged(UTextBlock* Text, const FString& Value)
	{
		if (Text && !Text->GetText().ToString().Equals(Value, ESearchCase::CaseSensitive)) Text->SetText(FText::FromString(Value));
	}

	void SetVisibilityIfChanged(UWidget* Widget, ESlateVisibility Visibility)
	{
		if (Widget && Widget->GetVisibility() != Visibility) Widget->SetVisibility(Visibility);
	}

	void SetToolTipTextIfChanged(UWidget* Widget, const FString& Value)
	{
		if (Widget && !Widget->GetToolTipText().ToString().Equals(Value, ESearchCase::CaseSensitive)) Widget->SetToolTipText(FText::FromString(Value));
	}

	void ApplyWidgetStyle(UWidget* Widget)
	{
		if (!IsValid(Widget))
		{
			return;
		}

		SetCursorIfChanged(Widget, EMouseCursor::None);
		const FString Name = Widget->GetName();
		const FString LowerName = Name.ToLower();

        // Tower detail cards own their palette and spacing.
        if (Name.StartsWith(TEXT("TD_"))) return;

		if (UBorder* Border = Cast<UBorder>(Widget))
		{
            if (Border->Background.GetResourceObject()) { SetBrushColorIfChanged(Border, FLinearColor::White); return; }
			if (LowerName.Contains(TEXT("goldframe")) || LowerName.Contains(TEXT("divider")))
			{
				// 안쪽 패널이 이미 픽셀 금박 테두리를 그리면 바깥 금테는 숨겨 이중 테두리를 막는다.
				// 가챠 카드 금테는 등급색·등장 연출에 쓰이므로 그대로 둔다.
				const UBorder* Inner = Cast<UBorder>(Border->GetContent());
				const bool bInnerFramed = Inner && Inner->Background.GetResourceObject() && !LowerName.Contains(TEXT("card"));
				SetBrushColorIfChanged(Border, bInnerFramed ? FLinearColor::Transparent : GoldFrameColor);
			}
			else if (LowerName.Contains(TEXT("badge")))
			{
				SetBrushColorIfChanged(Border, FLinearColor(FColor(58, 40, 16, 250)));
			}
			else
			{
				SetBrushColorIfChanged(Border, LowerName.Contains(TEXT("card")) ? InsetColor : PanelColor);
			}
		}
		else if (UButton* Button = Cast<UButton>(Widget))
		{
			// 픽셀 프레임 텍스처를 쓰는 버튼은 색을 곱하지 않고 텍스처 그대로 보여준다.
			if (Button->GetStyle().Normal.GetResourceObject()) { SetButtonColorIfChanged(Button, FLinearColor::White); return; }
			if (LowerName.Contains(TEXT("place")) || LowerName.Contains(TEXT("upgrade")) ||
				LowerName.Contains(TEXT("reload")) || LowerName.Contains(TEXT("heal")))
			{
				SetButtonColorIfChanged(Button, FLinearColor(FColor(138, 100, 36)));
			}
			else if (LowerName.Contains(TEXT("purge")) || LowerName.Contains(TEXT("skill")) ||
				LowerName.Contains(TEXT("autofire")))
			{
				SetButtonColorIfChanged(Button, FLinearColor(FColor(42, 35, 56)));
			}
			else
			{
				SetButtonColorIfChanged(Button, InsetColor);
			}
		}
		else if (UProgressBar* ProgressBar = Cast<UProgressBar>(Widget))
		{
			if (LowerName.Contains(TEXT("hp")))
			{
				SetFillColorIfChanged(ProgressBar, HealthColor);
			}
			else if (LowerName.Contains(TEXT("mana")) || LowerName.Contains(TEXT("exp")))
			{
				SetFillColorIfChanged(ProgressBar, CyanColor);
			}
			else
			{
				SetFillColorIfChanged(ProgressBar, GoldColor);
			}
		}
		else if (UTextBlock* Text = Cast<UTextBlock>(Widget))
		{
			FLinearColor TextColor = PrimaryTextColor;
			const UButton* ParentButton = Cast<UButton>(Text->GetParent());
			const UObject* ButtonTexture = ParentButton ? ParentButton->GetStyle().Normal.GetResourceObject() : nullptr;
			if (ButtonTexture && ButtonTexture->GetName().Contains(TEXT("ButtonGold")))
			{
				// 금 버튼 위 글자는 밝은 양피지색이 묻히므로 짙은 먹색으로.
				TextColor = FLinearColor(FColor(42, 26, 6));
			}
			else if (LowerName.StartsWith(TEXT("lbl_")) || LowerName.Contains(TEXT("effect")) ||
				LowerName.Contains(TEXT("meta")) || LowerName.Contains(TEXT("cooldown")) ||
				LowerName.Contains(TEXT("attacktype")) || LowerName.Contains(TEXT("info")))
			{
				TextColor = MutedTextColor;
			}
			else if (LowerName.Contains(TEXT("name")) || LowerName.Contains(TEXT("grade")) ||
				LowerName.Contains(TEXT("round")) || LowerName.Contains(TEXT("level")) ||
				LowerName.Contains(TEXT("gold")))
			{
				TextColor = GoldColor;
			}
			else if (LowerName.Contains(TEXT("count")) || LowerName.Contains(TEXT("token")) ||
				LowerName.Contains(TEXT("hp")) || LowerName.Contains(TEXT("mana")) ||
				LowerName.Contains(TEXT("exp")))
			{
				// 수치는 아이콘처럼 밝게: 본문보다 한 단계 밝은 양피지색.
				TextColor = FLinearColor(FColor(255, 243, 214));
			}
			SetTextColorIfChanged(Text, TextColor);
		}
	}

	// 테마 한 번 적용(패스) 동안 매니저 검색·카드 정보 생성을 한 번만 하도록 공유하는 상태.
	// (예전에는 시너지 행·가챠 카드 위젯마다 레벨 전체 액터를 GetActorOfClass로 훑었다.)
	struct FThemePass
	{
		TSet<UUserWidget*> Visited;

		ASynergyManager* GetSynergyManager(const UObject* WorldContext)
		{
			if (!bSynergyLooked)
			{
				bSynergyLooked = true;
				SynergyManager = Cast<ASynergyManager>(UGameplayStatics::GetActorOfClass(WorldContext, ASynergyManager::StaticClass()));
			}
			return SynergyManager;
		}

		AGachaManager* GetGachaManager(const UObject* WorldContext)
		{
			if (!bGachaLooked)
			{
				bGachaLooked = true;
				GachaManager = Cast<AGachaManager>(UGameplayStatics::GetActorOfClass(WorldContext, AGachaManager::StaticClass()));
			}
			return GachaManager;
		}

		const TArray<FGachaCardInfo>& GetCardInfos(AGachaManager* Manager)
		{
			if (!bCardInfosBuilt)
			{
				bCardInfosBuilt = true;
				CardInfos = Manager->BuildCardInfos(Manager->CurrentRollResult);
			}
			return CardInfos;
		}

	private:
		ASynergyManager* SynergyManager = nullptr;
		AGachaManager* GachaManager = nullptr;
		TArray<FGachaCardInfo> CardInfos;
		bool bSynergyLooked = false;
		bool bGachaLooked = false;
		bool bCardInfosBuilt = false;
	};

    void ApplyGachaCard(UUserWidget* Widget, FThemePass& Pass)
    {
        AGachaManager* Manager = Pass.GetGachaManager(Widget);
        FIntProperty* IndexProperty = FindFProperty<FIntProperty>(Widget->GetClass(), TEXT("CardIndex"));
        if (!Manager || !IndexProperty) return;
        const TArray<FGachaCardInfo>& Infos = Pass.GetCardInfos(Manager);
        const int32 Index = IndexProperty->GetPropertyValue_InContainer(Widget);
        if (!Infos.IsValidIndex(Index)) return;
        const FGachaCardInfo& Info = Infos[Index];
        // 등급색: 커먼 회철, 레어 청, 유니크 보라, 에픽 금. 일반 테마 패스 뒤에 카드 테두리·등급 글자에 덮어쓴다.
        // 업그레이드 카드는 등급과 상관없이 바람색으로 타워 카드와 구분한다.
        FLinearColor GradeColor(FColor(163, 171, 181));
        if (Info.bIsUpgrade) GradeColor = FLinearColor(FColor(95, 230, 198));
        else if (Info.Grade == TEXT("Rare")) GradeColor = FLinearColor(FColor(79, 176, 255));
        else if (Info.Grade == TEXT("Unique")) GradeColor = FLinearColor(FColor(189, 116, 255));
        else if (Info.Grade == TEXT("Epic")) GradeColor = FLinearColor(FColor(255, 188, 63));
        if (UTextBlock* Grade = Cast<UTextBlock>(Widget->GetWidgetFromName(TEXT("TEXT_Grade"))))
        {
            SetTextIfChanged(Grade, Info.Grade);
            SetTextColorIfChanged(Grade, GradeColor);
        }
        if (UBorder* Frame = Cast<UBorder>(Widget->GetWidgetFromName(TEXT("CardBorderGoldFrame")))) SetBrushColorIfChanged(Frame, GradeColor);
        if (UBorder* Badge = Cast<UBorder>(Widget->GetWidgetFromName(TEXT("UPBadge")))) SetBrushColorIfChanged(Badge, GradeColor * FLinearColor(.22f,.22f,.22f,1.f));
        if (UTextBlock* Type = Cast<UTextBlock>(Widget->GetWidgetFromName(TEXT("TEXT_AttackType")))) SetTextIfChanged(Type, Info.AttackType);
        UTextBlock* Previous = Cast<UTextBlock>(Widget->GetWidgetFromName(TEXT("TEXT_Info")));
        UVerticalBox* List = Cast<UVerticalBox>(Widget->GetWidgetFromName(TEXT("CardDetailsList")));
        if (!Previous || !List) return;
        if (Previous->GetText().ToString() == Info.Description && List->GetChildrenCount() > 0) return;
        Previous->SetText(FText::FromString(Info.Description));
        Previous->SetVisibility(ESlateVisibility::Collapsed);
        List->ClearChildren();
        auto Text = [Widget](const FString& Value, int32 Size, const FLinearColor& Color, const TCHAR* Prefix)
        {
            UTextBlock* Result = Widget->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), MakeUniqueObjectName(Widget->WidgetTree, UTextBlock::StaticClass(), Prefix));
            Result->SetText(FText::FromString(Value));
            FSlateFontInfo Font = Result->GetFont(); Font.Size = Size; Result->SetFont(Font);
            Result->SetColorAndOpacity(Color); Result->SetAutoWrapText(true);
            return Result;
        };
        TArray<FString> Sections; Info.Description.TrimStartAndEnd().ParseIntoArray(Sections, TEXT("\n\n"), true);
        for (const FString& Section : Sections)
        {
            TArray<FString> Lines; Section.ParseIntoArrayLines(Lines, true);
            if (Lines.IsEmpty()) continue;
            UBorder* Panel = Widget->WidgetTree->ConstructWidget<UBorder>();
            Panel->SetBrushColor(PanelColor); Panel->SetPadding(FMargin(12.f,10.f));
            List->AddChildToVerticalBox(Panel)->SetPadding(FMargin(0,0,0,10));
            UVerticalBox* Content = Widget->WidgetTree->ConstructWidget<UVerticalBox>(); Panel->SetContent(Content);
            Content->AddChildToVerticalBox(Text(Lines[0],14,GoldColor,TEXT("SectionName")))->SetPadding(FMargin(0,0,0,8));
            for (int32 LineIndex=1; LineIndex<Lines.Num(); ++LineIndex)
            {
                FString Label,Value;
                if (Lines[LineIndex].Split(TEXT(":"),&Label,&Value))
                {
                    UHorizontalBox* Row = Widget->WidgetTree->ConstructWidget<UHorizontalBox>();
                    Content->AddChildToVerticalBox(Row)->SetPadding(FMargin(0,3));
                    auto* LabelSlot = Row->AddChildToHorizontalBox(Text(Label.TrimStartAndEnd(),14,MutedTextColor,TEXT("LBL_Detail")));
                    LabelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
                    UTextBlock* Number = Text(Value.TrimStartAndEnd(),14,PrimaryTextColor,TEXT("DetailValue"));
                    Number->SetJustification(ETextJustify::Right);
                    Number->SetAutoWrapText(false);
                    Row->AddChildToHorizontalBox(Number)->SetPadding(FMargin(12,0,0,0));
                }
                else Content->AddChildToVerticalBox(Text(Lines[LineIndex],14,PrimaryTextColor,TEXT("DetailBody")))->SetPadding(FMargin(0,3));
            }
        }
        if (UScrollBox* Scroll = Cast<UScrollBox>(Widget->GetWidgetFromName(TEXT("CardDetailsScroll")))) Scroll->ScrollToStart();
    }

	void ApplyRecursive(UUserWidget* Widget, FThemePass& Pass)
	{
		if (!IsValid(Widget) || Pass.Visited.Contains(Widget) || !Widget->WidgetTree)
		{
			return;
		}
		Pass.Visited.Add(Widget);
        if (Widget->GetClass()->GetName().Contains(TEXT("SynergyBar")))
        {
            ASynergyManager* Manager = Pass.GetSynergyManager(Widget);
            UScrollBox* List = Cast<UScrollBox>(Widget->GetWidgetFromName(TEXT("SynergyList")));
            if (Manager && Manager->SynergyDataTable && List)
            {
                TArray<FSynergyDataRow*> Rows; Manager->SynergyDataTable->GetAllRows(TEXT("Synergy list"), Rows);
                UClass* RowClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/03_UI/01_SynergyUI/WBP_SynergyRow.WBP_SynergyRow_C"));
                for (const FSynergyDataRow* Row : Rows)
                {
                    bool bFound = false;
                    for (int32 Index = 0; Index < List->GetChildrenCount(); ++Index)
                    {
                        UUserWidget* Child = Cast<UUserWidget>(List->GetChildAt(Index));
                        UTextBlock* Label = Child ? Cast<UTextBlock>(Child->GetWidgetFromName(TEXT("SynergyName"))) : nullptr;
                        if (Label && Label->GetText().ToString() == Row->SynergyDisplayName) { bFound = true; break; }
                    }
                    if (bFound || !RowClass || Manager->SynergyCountMap.FindRef(Row->SynergyTag) <= 0) continue;
                    UUserWidget* Child = CreateWidget<UUserWidget>(Widget->GetWorld(), RowClass);
                    if (!Child) continue;
                    List->AddChild(Child);
                    if (UTextBlock* Label = Cast<UTextBlock>(Child->GetWidgetFromName(TEXT("SynergyName")))) Label->SetText(FText::FromString(Row->SynergyDisplayName));
                    if (UImage* Icon = Cast<UImage>(Child->GetWidgetFromName(TEXT("SynergyIcon")))) Icon->SetBrushFromTexture(Row->SynergyIcon);

                }

                // Keep ties in data-table order, independent of Blueprint rebuild order.
                TMap<FString, int32> CountsByName;
                TMap<FString, int32> OrderByName;
                for (int32 Index = 0; Index < Rows.Num(); ++Index)
                {
                    CountsByName.Add(Rows[Index]->SynergyDisplayName, Manager->SynergyCountMap.FindRef(Rows[Index]->SynergyTag));
                    OrderByName.Add(Rows[Index]->SynergyDisplayName, Index);
                }
                struct FSortedSynergyRow
                {
                    UWidget* Child;
                    UPanelSlot* SlotTemplate;
                    int32 Count;
                    int32 TableOrder;
                };
                TArray<FSortedSynergyRow> SortedRows;
                for (int32 Index = 0; Index < List->GetChildrenCount(); ++Index)
                {
                    UWidget* Child = List->GetChildAt(Index);
                    UUserWidget* RowWidget = Cast<UUserWidget>(Child);
                    UTextBlock* Label = RowWidget ? Cast<UTextBlock>(RowWidget->GetWidgetFromName(TEXT("SynergyName"))) : nullptr;
                    const FString DisplayName = Label ? Label->GetText().ToString() : FString();
                    const int32* TableOrder = OrderByName.Find(DisplayName);
                    SortedRows.Add({Child, Child->Slot, CountsByName.FindRef(DisplayName), TableOrder ? *TableOrder : MAX_int32});
                }
                SortedRows.StableSort([](const FSortedSynergyRow& A, const FSortedSynergyRow& B)
                {
                    return A.Count != B.Count ? A.Count > B.Count : A.TableOrder < B.TableOrder;
                });
                bool bOrderChanged = false;
                for (int32 Index = 0; Index < SortedRows.Num(); ++Index)
                    bOrderChanged |= List->GetChildAt(Index) != SortedRows[Index].Child;
                if (bOrderChanged)
                {
                    const float ScrollOffset = List->GetScrollOffset();
                    // InsertChildAt/ShiftChild do not update live Slate order; reattach only when needed.
                    List->ClearChildren();
                    for (const FSortedSynergyRow& Row : SortedRows) List->AddChild(Row.Child, Row.SlotTemplate);
                    List->SetScrollOffset(ScrollOffset);
                }
            }
        }

        if (Widget->GetClass()->GetName().Contains(TEXT("SynergyRow")))
        {
            UTextBlock* NameText = Cast<UTextBlock>(Widget->GetWidgetFromName(TEXT("SynergyName")));
            ASynergyManager* Manager = Pass.GetSynergyManager(Widget);
            if (NameText && Manager && Manager->SynergyDataTable)
            {
                TArray<FSynergyDataRow*> Rows; Manager->SynergyDataTable->GetAllRows(TEXT("Synergy UI"), Rows);
                for (const FSynergyDataRow* Row : Rows)
                {
                    if (Row->SynergyDisplayName != NameText->GetText().ToString()) continue;
                    const int32 Count = Manager->SynergyCountMap.FindRef(Row->SynergyTag);
                    SetVisibilityIfChanged(Widget, Count > 0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
                    if (Count <= 0) break;
                    int32 MaxStack = 0;
                    for (const FSynergyStep& Step : Row->Steps) MaxStack = FMath::Max(MaxStack, Step.RequireCount);
                    const int32 DisplayStack = MaxStack > 0 ? FMath::Clamp(Count, 0, MaxStack) : Count;
                    if (UTextBlock* CountText = Cast<UTextBlock>(Widget->GetWidgetFromName(TEXT("CountText"))))
                    {
                        SetTextIfChanged(CountText, MaxStack > 0
                            ? FString::Printf(TEXT("%d/%d"), DisplayStack, MaxStack)
                            : FString::FromInt(Count));
                    }

                    if (UHorizontalBox* SegmentBox = Cast<UHorizontalBox>(Widget->GetWidgetFromName(TEXT("SegmentBox"))))
                    {
                        bool bRebuildSegments = SegmentBox->GetChildrenCount() != Row->Steps.Num();
                        for (int32 Index = 0; !bRebuildSegments && Index < SegmentBox->GetChildrenCount(); ++Index)
                        {
                            USizeBox* Cell = Cast<USizeBox>(SegmentBox->GetChildAt(Index));
                            UBorder* Frame = Cell ? Cast<UBorder>(Cell->GetContent()) : nullptr;
                            UBorder* Fill = Frame ? Cast<UBorder>(Frame->GetContent()) : nullptr;
                            bRebuildSegments = Fill == nullptr;
                        }

                        if (bRebuildSegments)
                        {
                            SegmentBox->ClearChildren();
                            for (int32 Index = 0; Index < Row->Steps.Num(); ++Index)
                            {
                                USizeBox* Cell = Widget->WidgetTree->ConstructWidget<USizeBox>(
                                    USizeBox::StaticClass(),
                                    MakeUniqueObjectName(Widget->WidgetTree, USizeBox::StaticClass(), *FString::Printf(TEXT("SynergySegment_%d"), Index)));
                                Cell->SetWidthOverride(22.f);
                                Cell->SetHeightOverride(12.f);

                                UBorder* Frame = Widget->WidgetTree->ConstructWidget<UBorder>(
                                    UBorder::StaticClass(),
                                    MakeUniqueObjectName(Widget->WidgetTree, UBorder::StaticClass(), *FString::Printf(TEXT("SynergySegmentFrame_%d"), Index)));
                                Frame->SetBrushColor(SegmentFrameColor);
                                Frame->SetPadding(FMargin(1.f));

                                UBorder* Fill = Widget->WidgetTree->ConstructWidget<UBorder>(
                                    UBorder::StaticClass(),
                                    MakeUniqueObjectName(Widget->WidgetTree, UBorder::StaticClass(), *FString::Printf(TEXT("SynergySegmentFill_%d"), Index)));
                                Fill->SetBrushColor(SegmentEmptyColor);
                                Frame->SetContent(Fill);
                                Cell->SetContent(Frame);
                                SegmentBox->AddChildToHorizontalBox(Cell)->SetPadding(FMargin(2.f, 0.f));
                            }
                        }

                        const FLinearColor AccentColor = Row->BadgeColor.A > 0.f ? Row->BadgeColor : GoldColor;
                        for (int32 Index = 0; Index < Row->Steps.Num(); ++Index)
                        {
                            USizeBox* Cell = Cast<USizeBox>(SegmentBox->GetChildAt(Index));
                            UBorder* Frame = Cell ? Cast<UBorder>(Cell->GetContent()) : nullptr;
                            UBorder* Fill = Frame ? Cast<UBorder>(Frame->GetContent()) : nullptr;
                            if (!Fill) continue;

                            const FSynergyStep& Step = Row->Steps[Index];
                            SetBrushColorIfChanged(Fill, Count >= Step.RequireCount
                                ? AccentColor
                                : SegmentEmptyColor);
                            SetToolTipTextIfChanged(Cell, FString::Printf(TEXT("효과 단계 · %d스택"), Step.RequireCount));
                        }
                    }

                    FString Detail = FString::Printf(TEXT("%s · 현재 %d/%d스택\n\n단계별 효과\n"), *Row->SynergyDisplayName, DisplayStack, MaxStack);
                    FString ActiveEffect;
                    bool bHasActiveStep = false;
                    int32 NextStepCount = INDEX_NONE;
                    for (const FSynergyStep& Step : Row->Steps)
                    {
                        FString Effects = Step.EffectDesc;
                        for (const FBuffEffect& Effect : Step.Effects)
                            Effects += FString::Printf(TEXT("\n%s %+.2f%s"), *TowerUI::TagName(Effect.StatTag), Effect.ValueType == EBuffValueType::Percent ? Effect.Value*100.f : Effect.Value, Effect.ValueType == EBuffValueType::Percent ? TEXT("%") : TEXT(""));
                        Detail += FString::Printf(TEXT("%s %d스택: %s\n"), Count >= Step.RequireCount ? TEXT("[활성]") : TEXT("[미달]"), Step.RequireCount, *Effects);
                        if (Count >= Step.RequireCount)
                        {
                            bHasActiveStep = true;
                            ActiveEffect = Effects.TrimStartAndEnd();
                        }
                        else if (NextStepCount == INDEX_NONE)
                        {
                            NextStepCount = Step.RequireCount;
                        }
                    }
                    Detail += TEXT("\n스택에 기여한 타워\n");
                    for (ABaseTower* Tower : Manager->FieldTowers)
                    {
                        UTowerStatComponent* Stat = IsValid(Tower) ? Tower->GetComponentByClass<UTowerStatComponent>() : nullptr;
                        if (Stat && Stat->SynergyTags.HasTag(Row->SynergyTag)) Detail += FString::Printf(TEXT("%s · Lv.%d\n"), *Stat->TowerName, Stat->CurrentLevel);
                    }

                    UUserWidget* Tip = Cast<UUserWidget>(Widget->GetToolTip());
                    if (!Tip)
                    {
                        UClass* TipClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/03_UI/01_SynergyUI/WBP_SynergyTooltip.WBP_SynergyTooltip_C"));
                        if (TipClass) { Tip = CreateWidget<UUserWidget>(Widget->GetWorld(), TipClass); Widget->SetToolTip(Tip); }
                    }
                    if (Tip) if (UTextBlock* Text = Cast<UTextBlock>(Tip->GetWidgetFromName(TEXT("TooltipBody")))) SetTextIfChanged(Text, Detail);
                    FString EffectSummary = ActiveEffect;
                    if (EffectSummary.IsEmpty())
                    {
                        if (bHasActiveStep) EffectSummary = TEXT("효과 단계 활성");
                        else if (NextStepCount != INDEX_NONE) EffectSummary = FString::Printf(TEXT("다음 단계 · %d스택"), NextStepCount);
                        else EffectSummary = TEXT("단계별 효과 없음");
                    }
                    if (UTextBlock* EffectText = Cast<UTextBlock>(Widget->GetWidgetFromName(TEXT("EffectText")))) SetTextIfChanged(EffectText, EffectSummary);
                    break;
                }
            }
        }


		TArray<UWidget*> Children;
		Widget->WidgetTree->ForEachWidget([&Children](UWidget* Child)
		{
			if (IsValid(Child))
			{
				Children.Add(Child);
			}
		});

		for (UWidget* Child : Children)
		{
			ApplyWidgetStyle(Child);
			if (UUserWidget* NestedWidget = Cast<UUserWidget>(Child))
			{
				ApplyRecursive(NestedWidget, Pass);
			}
		}
        if (Widget->GetClass()->GetName().Contains(TEXT("GachaCard"))) ApplyGachaCard(Widget, Pass);
	}
}

void FFantasyUITheme::ApplyToWidgetTree(UUserWidget* RootWidget)
{
	FThemePass Pass;
	ApplyRecursive(RootWidget, Pass);
}

void FFantasyUITheme::ApplyToViewport(UWorld* World)
{
	if (!IsValid(World))
	{
		return;
	}

	TArray<UUserWidget*> TopLevelWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
		World,
		TopLevelWidgets,
		UUserWidget::StaticClass(),
		true);

	FThemePass Pass;
	for (UUserWidget* Widget : TopLevelWidgets)
	{
		ApplyRecursive(Widget, Pass);
	}
}
