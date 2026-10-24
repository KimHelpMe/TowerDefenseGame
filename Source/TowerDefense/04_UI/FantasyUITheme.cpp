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
	const FLinearColor PanelColor(0.025f, 0.038f, 0.072f, 0.94f);
	const FLinearColor InsetColor(0.045f, 0.062f, 0.102f, 0.96f);
	const FLinearColor GoldColor(0.86f, 0.70f, 0.40f, 1.f);
	const FLinearColor CyanColor(0.36f, 0.78f, 0.86f, 1.f);
	const FLinearColor PrimaryTextColor(0.84f, 0.88f, 0.95f, 1.f);
	const FLinearColor MutedTextColor(0.56f, 0.64f, 0.75f, 1.f);

	void ApplyWidgetStyle(UWidget* Widget)
	{
		if (!IsValid(Widget))
		{
			return;
		}

		Widget->SetCursor(EMouseCursor::None);
		const FString Name = Widget->GetName();
		const FString LowerName = Name.ToLower();

		if (UBorder* Border = Cast<UBorder>(Widget))
		{
            if (Border->Background.GetResourceObject()) { Border->SetBrushColor(FLinearColor::White); return; }
			if (LowerName.Contains(TEXT("goldframe")) || LowerName.Contains(TEXT("divider")))
			{
				Border->SetBrushColor(FLinearColor(0.48f, 0.36f, 0.19f, 0.9f));
			}
			else if (LowerName.Contains(TEXT("badge")))
			{
				Border->SetBrushColor(FLinearColor(0.36f, 0.25f, 0.09f, 0.98f));
			}
			else
			{
				Border->SetBrushColor(LowerName.Contains(TEXT("card")) ? InsetColor : PanelColor);
			}
		}
		else if (UButton* Button = Cast<UButton>(Widget))
		{
			if (LowerName.Contains(TEXT("place")) || LowerName.Contains(TEXT("upgrade")) ||
				LowerName.Contains(TEXT("reload")) || LowerName.Contains(TEXT("heal")))
			{
				Button->SetBackgroundColor(FLinearColor(0.34f, 0.24f, 0.105f, 1.f));
			}
			else if (LowerName.Contains(TEXT("purge")) || LowerName.Contains(TEXT("skill")) ||
				LowerName.Contains(TEXT("autofire")))
			{
				Button->SetBackgroundColor(FLinearColor(0.075f, 0.22f, 0.28f, 1.f));
			}
			else
			{
				Button->SetBackgroundColor(InsetColor);
			}
		}
		else if (UProgressBar* ProgressBar = Cast<UProgressBar>(Widget))
		{
			if (LowerName.Contains(TEXT("hp")))
			{
				ProgressBar->SetFillColorAndOpacity(FLinearColor(0.27f, 0.72f, 0.46f, 1.f));
			}
			else if (LowerName.Contains(TEXT("mana")))
			{
				ProgressBar->SetFillColorAndOpacity(CyanColor);
			}
			else
			{
				ProgressBar->SetFillColorAndOpacity(GoldColor);
			}
		}
		else if (UTextBlock* Text = Cast<UTextBlock>(Widget))
		{
			FLinearColor TextColor = PrimaryTextColor;
			if (LowerName.StartsWith(TEXT("lbl_")) || LowerName.Contains(TEXT("effect")) ||
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
				TextColor = CyanColor;
			}
			Text->SetColorAndOpacity(FSlateColor(TextColor));
		}
	}

    void ApplyGachaCard(UUserWidget* Widget)
    {
        AGachaManager* Manager = Cast<AGachaManager>(UGameplayStatics::GetActorOfClass(Widget, AGachaManager::StaticClass()));
        FIntProperty* IndexProperty = FindFProperty<FIntProperty>(Widget->GetClass(), TEXT("CardIndex"));
        if (!Manager || !IndexProperty) return;
        const TArray<FGachaCardInfo> Infos = Manager->BuildCardInfos(Manager->CurrentRollResult);
        const int32 Index = IndexProperty->GetPropertyValue_InContainer(Widget);
        if (!Infos.IsValidIndex(Index)) return;
        const FGachaCardInfo& Info = Infos[Index];
        // Preserve the original game's rarity palette after the general theme pass.
        FLinearColor GradeColor(.6f,.6f,.6f,1.f);
        if (Info.Grade == TEXT("Common")) GradeColor = FLinearColor(.1f,.7f,.2f,1.f);
        else if (Info.Grade == TEXT("Rare")) GradeColor = FLinearColor(.1f,.4f,.9f,1.f);
        else if (Info.Grade == TEXT("Unique")) GradeColor = FLinearColor(.8f,.1f,.1f,1.f);
        else if (Info.Grade == TEXT("Epic")) GradeColor = FLinearColor(.9f,.7f,.1f,1.f);
        if (UTextBlock* Grade = Cast<UTextBlock>(Widget->GetWidgetFromName(TEXT("TEXT_Grade"))))
        {
            Grade->SetText(FText::FromString(Info.Grade));
            Grade->SetColorAndOpacity(GradeColor);
        }
        if (UBorder* Frame = Cast<UBorder>(Widget->GetWidgetFromName(TEXT("CardBorderGoldFrame")))) Frame->SetBrushColor(GradeColor);
        if (UBorder* Badge = Cast<UBorder>(Widget->GetWidgetFromName(TEXT("UPBadge")))) Badge->SetBrushColor(GradeColor * FLinearColor(.22f,.22f,.22f,1.f));
        if (UTextBlock* Type = Cast<UTextBlock>(Widget->GetWidgetFromName(TEXT("TEXT_AttackType")))) Type->SetText(FText::FromString(Info.AttackType));
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

	void ApplyRecursive(UUserWidget* Widget, TSet<UUserWidget*>& Visited)
	{
		if (!IsValid(Widget) || Visited.Contains(Widget) || !Widget->WidgetTree)
		{
			return;
		}
		Visited.Add(Widget);
        if (Widget->GetClass()->GetName().Contains(TEXT("SynergyBar")))
        {
            ASynergyManager* Manager = Cast<ASynergyManager>(UGameplayStatics::GetActorOfClass(Widget, ASynergyManager::StaticClass()));
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
            ASynergyManager* Manager = Cast<ASynergyManager>(UGameplayStatics::GetActorOfClass(Widget, ASynergyManager::StaticClass()));
            if (NameText && Manager && Manager->SynergyDataTable)
            {
                TArray<FSynergyDataRow*> Rows; Manager->SynergyDataTable->GetAllRows(TEXT("Synergy UI"), Rows);
                for (const FSynergyDataRow* Row : Rows)
                {
                    if (Row->SynergyDisplayName != NameText->GetText().ToString()) continue;
                    const int32 Count = Manager->SynergyCountMap.FindRef(Row->SynergyTag);
                    Widget->SetVisibility(Count > 0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
                    if (Count <= 0) break;
                    int32 MaxStack = 0;
                    for (const FSynergyStep& Step : Row->Steps) MaxStack = FMath::Max(MaxStack, Step.RequireCount);
                    const int32 DisplayStack = MaxStack > 0 ? FMath::Clamp(Count, 0, MaxStack) : Count;
                    if (UTextBlock* CountText = Cast<UTextBlock>(Widget->GetWidgetFromName(TEXT("CountText"))))
                    {
                        CountText->SetText(MaxStack > 0
                            ? FText::FromString(FString::Printf(TEXT("%d/%d"), DisplayStack, MaxStack))
                            : FText::AsNumber(Count));
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
                                Frame->SetBrushColor(FLinearColor(0.24f, 0.29f, 0.36f, 1.f));
                                Frame->SetPadding(FMargin(1.f));

                                UBorder* Fill = Widget->WidgetTree->ConstructWidget<UBorder>(
                                    UBorder::StaticClass(),
                                    MakeUniqueObjectName(Widget->WidgetTree, UBorder::StaticClass(), *FString::Printf(TEXT("SynergySegmentFill_%d"), Index)));
                                Fill->SetBrushColor(FLinearColor(0.09f, 0.13f, 0.19f, 1.f));
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
                            Fill->SetBrushColor(Count >= Step.RequireCount
                                ? AccentColor
                                : FLinearColor(0.09f, 0.13f, 0.19f, 1.f));
                            Cell->SetToolTipText(FText::FromString(FString::Printf(TEXT("효과 단계 · %d스택"), Step.RequireCount)));
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
                    if (Tip) if (UTextBlock* Text = Cast<UTextBlock>(Tip->GetWidgetFromName(TEXT("TooltipBody")))) Text->SetText(FText::FromString(Detail));
                    FString EffectSummary = ActiveEffect;
                    if (EffectSummary.IsEmpty())
                    {
                        if (bHasActiveStep) EffectSummary = TEXT("효과 단계 활성");
                        else if (NextStepCount != INDEX_NONE) EffectSummary = FString::Printf(TEXT("다음 단계 · %d스택"), NextStepCount);
                        else EffectSummary = TEXT("단계별 효과 없음");
                    }
                    if (UTextBlock* EffectText = Cast<UTextBlock>(Widget->GetWidgetFromName(TEXT("EffectText")))) EffectText->SetText(FText::FromString(EffectSummary));
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
				ApplyRecursive(NestedWidget, Visited);
			}
		}
        if (Widget->GetClass()->GetName().Contains(TEXT("GachaCard"))) ApplyGachaCard(Widget);
	}
}

void FFantasyUITheme::ApplyToWidgetTree(UUserWidget* RootWidget)
{
	TSet<UUserWidget*> Visited;
	ApplyRecursive(RootWidget, Visited);
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

	TSet<UUserWidget*> Visited;
	for (UUserWidget* Widget : TopLevelWidgets)
	{
		ApplyRecursive(Widget, Visited);
	}
}
