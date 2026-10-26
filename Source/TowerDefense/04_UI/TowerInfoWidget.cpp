#include "TowerInfoWidget.h"
#include "TowerDefenseTags.h"
#include "TowerRangeDecalComponent.h"
#include "UIEffectFormatting.h"

#include "FantasyUITheme.h"
#include "TowerDefenseGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "TowerStatComponent.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Widget.h"
#include "Components/WrapBox.h"
#include "Engine/Texture2D.h"
#include "Blueprint/WidgetTree.h"

namespace
{
	const FLinearColor DetailPanelColor(0.035f, 0.048f, 0.075f, 0.98f);
	const FLinearColor DetailRowColor(0.065f, 0.080f, 0.115f, 0.96f);
	const FLinearColor DetailGoldColor(0.91f, 0.72f, 0.36f, 1.f);
	const FLinearColor DetailTextColor(0.89f, 0.91f, 0.96f, 1.f);
	const FLinearColor DetailMutedColor(0.60f, 0.68f, 0.79f, 1.f);
	const FLinearColor DetailCyanColor(0.39f, 0.79f, 0.88f, 1.f);
	const FLinearColor DetailVioletColor(0.70f, 0.55f, 0.95f, 1.f);

	FName MakeWidgetName(UUserWidget* Owner, UClass* WidgetClass, const TCHAR* Prefix)
	{
		return MakeUniqueObjectName(Owner->WidgetTree, WidgetClass, FName(Prefix));
	}

	UTextBlock* MakeDetailText(UUserWidget* Owner, const FString& Value, int32 FontSize,
		const FLinearColor& Color, const TCHAR* Prefix, bool bBold = false)
	{
		UTextBlock* Text = Owner->WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(), MakeWidgetName(Owner, UTextBlock::StaticClass(), Prefix));
		Text->SetText(FText::FromString(Value));
		FSlateFontInfo Font = Text->GetFont();
		Font.Size = FontSize;
		if (bBold) Font.TypefaceFontName = FName(TEXT("SemiBold"));
		Text->SetFont(Font);
		Text->SetColorAndOpacity(Color);
		Text->SetAutoWrapText(true);
		return Text;
	}

	void AddTagPills(UUserWidget* Owner, UVerticalBox* Parent, const FString& Line)
	{
		// 태그가 많으면 패널 밖으로 밀려나므로 줄바꿈되는 WrapBox에 담는다.
		UWrapBox* Tags = Owner->WidgetTree->ConstructWidget<UWrapBox>(
			UWrapBox::StaticClass(), MakeWidgetName(Owner, UWrapBox::StaticClass(), TEXT("TD_TagRow")));
		Tags->SetInnerSlotPadding(FVector2D(6.f, 4.f));
		TArray<FString> Parts;
		Line.ParseIntoArray(Parts, TEXT("·"), true);
		for (FString Part : Parts)
		{
			Part.TrimStartAndEndInline();
			if (Part.IsEmpty()) continue;

			UBorder* Pill = Owner->WidgetTree->ConstructWidget<UBorder>(
				UBorder::StaticClass(), MakeWidgetName(Owner, UBorder::StaticClass(), TEXT("TD_TagPill")));
			Pill->SetBrushColor(FLinearColor(0.12f, 0.17f, 0.25f, 1.f));
			Pill->SetPadding(FMargin(8.f, 4.f));
			UTextBlock* PillText = MakeDetailText(Owner, Part, 12, DetailCyanColor, TEXT("TD_TagText"));
			PillText->SetAutoWrapText(false);
			Pill->SetContent(PillText);
			Tags->AddChildToWrapBox(Pill);
		}
		Parent->AddChildToVerticalBox(Tags)->SetPadding(FMargin(0.f, 2.f));
	}

	void AddLabelValueRow(UUserWidget* Owner, UVerticalBox* Parent, const FString& Label,
		const FString& Value, bool bEmphasize = false)
	{
		UHorizontalBox* Row = Owner->WidgetTree->ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(), MakeWidgetName(Owner, UHorizontalBox::StaticClass(), TEXT("TD_DetailRow")));
		Row->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		// 값이 길어도 패널 폭 안에서 줄바꿈되도록 라벨·값 모두 비율 폭(55:45)을 준다.
		UTextBlock* LabelText = MakeDetailText(Owner, Label, 13, DetailMutedColor, TEXT("TD_DetailLabel"));
		FSlateChildSize LabelSize(ESlateSizeRule::Fill);
		LabelSize.Value = 0.55f;
		Row->AddChildToHorizontalBox(LabelText)->SetSize(LabelSize);

		UTextBlock* ValueText = MakeDetailText(Owner, Value, 13,
			bEmphasize ? DetailGoldColor : DetailTextColor, TEXT("TD_DetailValue"), bEmphasize);
		ValueText->SetJustification(ETextJustify::Right);
		UHorizontalBoxSlot* ValueSlot = Row->AddChildToHorizontalBox(ValueText);
		FSlateChildSize ValueSize(ESlateSizeRule::Fill);
		ValueSize.Value = 0.45f;
		ValueSlot->SetSize(ValueSize);
		ValueSlot->SetPadding(FMargin(10.f, 0.f, 0.f, 0.f));
		Parent->AddChildToVerticalBox(Row)->SetPadding(FMargin(0.f, 3.f));
	}

	void AddEffectRow(UUserWidget* Owner, UVerticalBox* Parent, const FString& Line, const FLinearColor& Accent)
	{
		FString EffectText;
		FString Source;
		if (!Line.Split(TEXT("·"), &EffectText, &Source, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
		{
			Parent->AddChildToVerticalBox(MakeDetailText(Owner, Line, 13, DetailTextColor, TEXT("TD_EffectText")))
				->SetPadding(FMargin(0.f, 3.f));
			return;
		}

		UHorizontalBox* Row = Owner->WidgetTree->ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(), MakeWidgetName(Owner, UHorizontalBox::StaticClass(), TEXT("TD_EffectRow")));
		UBorder* AccentBar = Owner->WidgetTree->ConstructWidget<UBorder>(
			UBorder::StaticClass(), MakeWidgetName(Owner, UBorder::StaticClass(), TEXT("TD_EffectAccent")));
		AccentBar->SetBrushColor(Accent);
		USizeBox* AccentSize = Owner->WidgetTree->ConstructWidget<USizeBox>(
			USizeBox::StaticClass(), MakeWidgetName(Owner, USizeBox::StaticClass(), TEXT("TD_EffectAccentSize")));
		AccentSize->SetWidthOverride(3.f);
		AccentSize->SetHeightOverride(22.f);
		AccentSize->SetContent(AccentBar);
		Row->AddChildToHorizontalBox(AccentSize)->SetPadding(FMargin(0.f, 0.f, 8.f, 0.f));

		EffectText.TrimStartAndEndInline();
		Source.TrimStartAndEndInline();
		Row->AddChildToHorizontalBox(MakeDetailText(Owner, EffectText, 13, DetailTextColor, TEXT("TD_EffectValue")))
			->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

		UBorder* Badge = Owner->WidgetTree->ConstructWidget<UBorder>(
			UBorder::StaticClass(), MakeWidgetName(Owner, UBorder::StaticClass(), TEXT("TD_EffectBadge")));
		Badge->SetBrushColor(Accent * FLinearColor(.28f, .28f, .28f, 1.f));
		Badge->SetPadding(FMargin(7.f, 3.f));
		Badge->SetContent(MakeDetailText(Owner, Source, 11, Accent, TEXT("TD_EffectSource"), true));
		Row->AddChildToHorizontalBox(Badge)->SetPadding(FMargin(8.f, 0.f, 0.f, 0.f));

		UBorder* RowCard = Owner->WidgetTree->ConstructWidget<UBorder>(
			UBorder::StaticClass(), MakeWidgetName(Owner, UBorder::StaticClass(), TEXT("TD_EffectCard")));
		RowCard->SetBrushColor(DetailRowColor);
		RowCard->SetPadding(FMargin(9.f, 7.f));
		RowCard->SetContent(Row);
		Parent->AddChildToVerticalBox(RowCard)->SetPadding(FMargin(0.f, 3.f));
	}

	void WrapStatRow(UUserWidget* Owner, const TCHAR* WidgetName)
	{
		UWidget* Row = Owner->GetWidgetFromName(WidgetName);
		UPanelWidget* Parent = Row ? Row->GetParent() : nullptr;
		if (!Row || !Parent) return;

		if (UBorder* ExistingCard = Cast<UBorder>(Parent))
		{
			if (ExistingCard->GetName().StartsWith(TEXT("TD_"))) return;
		}

		const int32 Index = Parent->GetChildIndex(Row);
		UPanelSlot* SlotTemplate = Row->Slot;
		if (Index == INDEX_NONE || !Parent->RemoveChildAt(Index)) return;

		UBorder* Card = Owner->WidgetTree->ConstructWidget<UBorder>(
			UBorder::StaticClass(), MakeWidgetName(Owner, UBorder::StaticClass(), TEXT("TD_StatCard")));
		Card->SetBrushColor(DetailRowColor);
		Card->SetPadding(FMargin(8.f, 7.f));

		// 카드 왼쪽에 해당 스탯의 업그레이드 아이콘 자리를 둔다(이미지는 RefreshInfo에서 지정).
		UHorizontalBox* CardRow = Owner->WidgetTree->ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(), MakeWidgetName(Owner, UHorizontalBox::StaticClass(), TEXT("TD_StatCardRow")));
		USizeBox* IconSize = Owner->WidgetTree->ConstructWidget<USizeBox>(
			USizeBox::StaticClass(), MakeWidgetName(Owner, USizeBox::StaticClass(), TEXT("TD_StatIconSize")));
		IconSize->SetWidthOverride(26.f);
		IconSize->SetHeightOverride(26.f);
		UImage* Icon = Owner->WidgetTree->ConstructWidget<UImage>(
			UImage::StaticClass(), FName(*FString::Printf(TEXT("TD_StatIcon_%s"), WidgetName)));
		Icon->SetVisibility(ESlateVisibility::Collapsed);
		IconSize->SetContent(Icon);
		UHorizontalBoxSlot* IconSlot = CardRow->AddChildToHorizontalBox(IconSize);
		IconSlot->SetVerticalAlignment(VAlign_Center);
		IconSlot->SetPadding(FMargin(0.f, 0.f, 7.f, 0.f));
		CardRow->AddChildToHorizontalBox(Row)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		Card->SetContent(CardRow);
		Parent->InsertChildAt(Index, Card, SlotTemplate);
	}

	// 스탯 카드 아이콘. Path가 비면 숨긴다.
	void SetStatIcon(UUserWidget* Owner, const TCHAR* RowName, const TCHAR* Path)
	{
		UImage* Icon = Cast<UImage>(Owner->GetWidgetFromName(FName(*FString::Printf(TEXT("TD_StatIcon_%s"), RowName))));
		if (!Icon) return;
		UTexture2D* Texture = Path ? LoadObject<UTexture2D>(nullptr, Path) : nullptr;
		if (Texture && Icon->GetBrush().GetResourceObject() != Texture) Icon->SetBrushFromTexture(Texture);
		const ESlateVisibility Visibility = Texture ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
		if (Icon->GetVisibility() != Visibility) Icon->SetVisibility(Visibility);
	}

	const TCHAR* const IconAttackPhysical = TEXT("/Game/09_Textures/03_FantasyIcons/Upgrades/Icon_Upgrade_Attack_Physical.Icon_Upgrade_Attack_Physical");
	const TCHAR* const IconAttackSpeed = TEXT("/Game/09_Textures/03_FantasyIcons/Upgrades/Icon_Upgrade_Attack_Speed.Icon_Upgrade_Attack_Speed");
	const TCHAR* const IconAttackRange = TEXT("/Game/09_Textures/03_FantasyIcons/Upgrades/Icon_Upgrade_Attack_Range.Icon_Upgrade_Attack_Range");
	const TCHAR* const IconBuffDuration = TEXT("/Game/09_Textures/03_FantasyIcons/Upgrades/Icon_Upgrade_Buff_Duration.Icon_Upgrade_Buff_Duration");
}

void UTowerInfoWidget::LevelUp()
{
	if (!TowerStat) return;
	TowerStat->LevelUp();
}

void UTowerInfoWidget::CloseWidget()
{
	if (IsValid(OwnerTower) && OwnerTower->TowerRangeDecal) OwnerTower->TowerRangeDecal->SetRangeVisible(false);
	RemoveFromParent();
}

void UTowerInfoWidget::InitWidget_Implementation(UTowerStatComponent* Stat)
{
	if (!Stat) return;
	TowerStat = Stat;
	TowerMana = TowerStat->GetOwner()->GetComponentByClass<UManaComponent>();
	OwnerTower = Cast<ABaseTower>(TowerStat->GetOwner());
	// 새 타워가 지정되면 갱신 주기를 기다리지 않고 다음 틱에 바로 표시.
	bRefreshPending = true;
}

void UTowerInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();
	FFantasyUITheme::ApplyToWidgetTree(this);
	StyleStatCards();
	EnsureDetailsPanel();
}

void UTowerInfoWidget::StyleStatCards()
{
	WrapStatRow(this, TEXT("StatRow_ATK"));
	WrapStatRow(this, TEXT("StatRow_Range"));
	WrapStatRow(this, TEXT("StatRow_ATK_SPEED"));
	WrapStatRow(this, TEXT("StatRow_Type"));
}

void UTowerInfoWidget::EnsureDetailsPanel()
{
	if (IsValid(DetailedEffectsPanel)) return;

	UWidget* LegacyText = GetWidgetFromName(TEXT("DetailedEffects"));
	UPanelWidget* Parent = LegacyText ? LegacyText->GetParent() : nullptr;
	if (!Parent || !WidgetTree) return;

	const int32 Index = Parent->GetChildIndex(LegacyText);
	UPanelSlot* SlotTemplate = LegacyText->Slot;
	if (Index == INDEX_NONE || !Parent->RemoveChildAt(Index)) return;

	DetailedEffectsPanel = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), MakeWidgetName(this, UVerticalBox::StaticClass(), TEXT("TD_DetailsPanel")));
	DetailedEffectsPanel->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	Parent->InsertChildAt(Index, DetailedEffectsPanel, SlotTemplate);
}

void UTowerInfoWidget::BuildDetailsPanel(const FString& Details)
{
	EnsureDetailsPanel();
	if (!DetailedEffectsPanel || LastRenderedDetails == Details) return;

	DetailedEffectsPanel->ClearChildren();
	TArray<FString> Sections;
	Details.ParseIntoArray(Sections, TEXT("\n\n"), true);

	for (FString SectionText : Sections)
	{
		SectionText.TrimStartAndEndInline();
		TArray<FString> Lines;
		SectionText.ParseIntoArrayLines(Lines, true);
		if (Lines.IsEmpty()) continue;

		const FString Heading = Lines[0].TrimStartAndEnd();
		const bool bIsSynergy = Heading.Contains(TEXT("시너지"));
		const FLinearColor Accent = bIsSynergy ? DetailVioletColor : DetailCyanColor;

		UBorder* SectionCard = WidgetTree->ConstructWidget<UBorder>(
			UBorder::StaticClass(), MakeWidgetName(this, UBorder::StaticClass(), TEXT("TD_DetailSection")));
		SectionCard->SetBrushColor(DetailPanelColor);
		SectionCard->SetPadding(FMargin(11.f, 10.f));
		DetailedEffectsPanel->AddChildToVerticalBox(SectionCard)->SetPadding(FMargin(0.f, 0.f, 0.f, 7.f));

		UVerticalBox* Content = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(), MakeWidgetName(this, UVerticalBox::StaticClass(), TEXT("TD_SectionContent")));
		SectionCard->SetContent(Content);

		UHorizontalBox* TitleRow = WidgetTree->ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(), MakeWidgetName(this, UHorizontalBox::StaticClass(), TEXT("TD_SectionTitleRow")));
		USizeBox* MarkerSize = WidgetTree->ConstructWidget<USizeBox>(
			USizeBox::StaticClass(), MakeWidgetName(this, USizeBox::StaticClass(), TEXT("TD_SectionMarkerSize")));
		MarkerSize->SetWidthOverride(3.f);
		MarkerSize->SetHeightOverride(17.f);
		UBorder* Marker = WidgetTree->ConstructWidget<UBorder>(
			UBorder::StaticClass(), MakeWidgetName(this, UBorder::StaticClass(), TEXT("TD_SectionMarker")));
		Marker->SetBrushColor(Accent);
		MarkerSize->SetContent(Marker);
		TitleRow->AddChildToHorizontalBox(MarkerSize)->SetPadding(FMargin(0.f, 0.f, 8.f, 0.f));
		TitleRow->AddChildToHorizontalBox(MakeDetailText(this, Heading, 13, DetailGoldColor, TEXT("TD_SectionTitle"), true))
			->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		Content->AddChildToVerticalBox(TitleRow)->SetPadding(FMargin(0.f, 0.f, 0.f, 7.f));

		for (int32 LineIndex = 1; LineIndex < Lines.Num(); ++LineIndex)
		{
			FString Line = Lines[LineIndex].TrimStartAndEnd();
			if (Line.IsEmpty()) continue;

			if (Line.Contains(TEXT("적용 중인 효과 없음")))
			{
				Content->AddChildToVerticalBox(MakeDetailText(this, Line, 12, DetailMutedColor, TEXT("TD_EmptyEffect")))
					->SetPadding(FMargin(0.f, 2.f));
				continue;
			}

			if (Heading.Contains(TEXT("적용 중인 효과")) && Line.Contains(TEXT("·")))
			{
				AddEffectRow(this, Content, Line, Line.Contains(TEXT("시너지")) ? DetailVioletColor : DetailCyanColor);
				continue;
			}

			FString Label;
			FString Value;
			if (Line.Split(TEXT(":"), &Label, &Value))
			{
				AddLabelValueRow(this, Content, Label.TrimStartAndEnd(), Value.TrimStartAndEnd(), true);
				continue;
			}

			if (Heading.Contains(TEXT("기본 대비 보너스")) && Line.Split(TEXT(" "), &Label, &Value))
			{
				AddLabelValueRow(this, Content, Label.TrimStartAndEnd(), Value.TrimStartAndEnd(), true);
				continue;
			}

			const bool bTagLine = Line.Contains(TEXT("·")) &&
				!Line.Contains(TEXT("초")) && !Line.Contains(TEXT("유지"));
			if (bTagLine)
			{
				AddTagPills(this, Content, Line);
				continue;
			}

			Content->AddChildToVerticalBox(MakeDetailText(this, Line, 12, DetailTextColor, TEXT("TD_DetailBody")))
				->SetPadding(FMargin(0.f, 2.f));
		}
	}

	LastRenderedDetails = Details;
}

void UTowerInfoWidget::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
	Super::NativeTick(Geometry, DeltaTime);

	// 정보 문자열 조립·데이터 테이블 조회·위젯 검색이 무거우므로 매 프레임이 아닌 0.1초 간격으로 갱신.
	TimeSinceRefresh += DeltaTime;
	if (!bRefreshPending && TimeSinceRefresh < RefreshInterval) return;
	bRefreshPending = false;
	TimeSinceRefresh = 0.f;
	RefreshInfo();
}

void UTowerInfoWidget::RefreshInfo()
{
	if (!IsValid(TowerStat)) return;

	auto SetText = [this](const TCHAR* Name, const FString& Value)
	{
		if (UTextBlock* Text = Cast<UTextBlock>(GetWidgetFromName(Name)))
		{
			const FText NewText = FText::FromString(Value);
			if (!Text->GetText().EqualTo(NewText)) Text->SetText(NewText);
		}
	};
	auto EffectLine = [](const FBuffEffect& Effect)
	{
		return FString::Printf(TEXT("%s %+.2f%s"), *TowerUI::TagName(Effect.StatTag),
			Effect.ValueType == EBuffValueType::Percent ? Effect.Value * 100.f : Effect.Value,
			Effect.ValueType == EBuffValueType::Percent ? TEXT("%") : TEXT(""));
	};

	SetText(TEXT("TEXT_TowerName"), TowerStat->TowerName);
	SetText(TEXT("TEXT_TowerMeta"), TowerStat->Grade + TEXT(" · ") + TowerStat->TowerType);
	SetText(TEXT("TEXT_Level"), FString::Printf(TEXT("%d / %d"), TowerStat->CurrentLevel, TowerStat->MaxLevel));
	if (UImage* Icon = Cast<UImage>(GetWidgetFromName(TEXT("TowerIcon"))))
	{
		if (TowerStat->TowerIcon) Icon->SetBrushFromTexture(TowerStat->TowerIcon);
	}

	const FName Key(*FString::Printf(TEXT("%s_Lv%d"), *TowerStat->TowerRowName.ToString(), TowerStat->CurrentLevel));
	const FTowerDataRow* Row = TowerStat->TowerDataTable
		? TowerStat->TowerDataTable->FindRow<FTowerDataRow>(Key, TEXT("Tower UI")) : nullptr;
	const bool bSupport = Row && Row->BuffSetting.BuffType != EBuffComponentType::None;
	FString Info;

	if (bSupport)
	{
		const FBuffConfig& Buff = Row->BuffSetting;
		SetText(TEXT("LBL_ATK"), Buff.BuffEffects.IsEmpty() ? TEXT("버프 효과") : TowerUI::TagName(Buff.BuffEffects[0].StatTag));
		SetText(TEXT("TEXT_ATK"), Buff.BuffEffects.IsEmpty() ? TEXT("없음") : FString::Printf(TEXT("%+.2f%s"),
			Buff.BuffEffects[0].ValueType == EBuffValueType::Percent ? Buff.BuffEffects[0].Value * 100.f : Buff.BuffEffects[0].Value,
			Buff.BuffEffects[0].ValueType == EBuffValueType::Percent ? TEXT("%") : TEXT("")));
		SetText(TEXT("LBL_ATK_Speed"), TEXT("지속시간"));
		SetText(TEXT("TEXT_ATK_Speed"), Buff.BuffType == EBuffComponentType::Passive
			? TEXT("범위 내 유지") : FString::Printf(TEXT("%.1f초"), Buff.BuffDuration));
		SetText(TEXT("LBL_Range"), TEXT("버프 범위"));
		SetText(TEXT("TEXT_Range"), FString::Printf(TEXT("%.0f"), IsValid(OwnerTower) ? OwnerTower->GetRangeRadius() : Buff.BuffRadius));
		SetText(TEXT("LBL_Type"), TEXT("적용 대상"));
		SetText(TEXT("TEXT_Type"), Buff.BuffTarget == EBuffTarget::AllyTower ? TEXT("아군 타워") : TEXT("적"));
		SetStatIcon(this, TEXT("StatRow_ATK"), nullptr);
		SetStatIcon(this, TEXT("StatRow_ATK_SPEED"), IconBuffDuration);
		SetStatIcon(this, TEXT("StatRow_Range"), IconAttackRange);

		Info = Buff.BuffType == EBuffComponentType::Active ? TEXT("액티브 효과\n") : TEXT("지속 효과\n");
		for (const FBuffEffect& Effect : Buff.BuffEffects) Info += EffectLine(Effect) + TEXT("\n");
		if (Buff.BuffType == EBuffComponentType::Active)
			Info += FString::Printf(TEXT("범위: %.0f\n지속 %.1f초 · 재사용 %.1f초\n"), Buff.BuffRadius, Buff.BuffDuration, Buff.BuffCooldown);
		else
			Info += FString::Printf(TEXT("범위: %.0f\n적용 주기 %.1f초 · 범위 내 유지\n"), Buff.BuffRadius, Buff.BuffInterval);
	}
	else
	{
		SetText(TEXT("LBL_ATK"), TEXT("공격력"));
		SetText(TEXT("TEXT_ATK"), FString::Printf(TEXT("%.0f"), TowerStat->GetStat(TDTags::Stat_AttackDamage)));

		float SpeedBonus = 0.f;
		if (const ATowerDefenseGameMode* Mode = Cast<ATowerDefenseGameMode>(UGameplayStatics::GetGameMode(this)))
		{
			for (const FBuffEffect& Effect : Mode->InGameUpgrades)
				if (Effect.StatTag == TDTags::Upgrade_AttackSpeed) SpeedBonus += Effect.Value;
		}
		const float Interval = FMath::Max(0.01f,
			TowerStat->GetStat(TDTags::Stat_AttackSpeed) * (1.f - SpeedBonus));
		SetText(TEXT("LBL_ATK_Speed"), TEXT("초당 공격 횟수"));
		SetText(TEXT("TEXT_ATK_Speed"), FString::Printf(TEXT("%.2f /s"), 1.f / Interval));
		SetText(TEXT("LBL_Range"), TEXT("공격 사거리"));
		SetText(TEXT("TEXT_Range"), FString::Printf(TEXT("%.0f"), IsValid(OwnerTower) ? OwnerTower->GetRangeRadius() : 0.f));
		SetText(TEXT("LBL_Type"), TEXT("타워 유형"));
		SetText(TEXT("TEXT_Type"), TowerStat->TowerType);
		SetStatIcon(this, TEXT("StatRow_ATK"), IconAttackPhysical);
		SetStatIcon(this, TEXT("StatRow_ATK_SPEED"), IconAttackSpeed);
		SetStatIcon(this, TEXT("StatRow_Range"), IconAttackRange);

		Info = TEXT("공격 효과\n");
		if (!TowerStat->TowerInfo.IsEmpty() && TowerStat->TowerInfo != TEXT("None"))
			Info += TowerStat->TowerInfo + TEXT("\n");
		if (!TowerStat->Elements.IsEmpty()) Info += TowerUI::TagList(TowerStat->Elements) + TEXT("\n");
		if (!TowerStat->AttackTypes.IsEmpty()) Info += TowerUI::TagList(TowerStat->AttackTypes) + TEXT("\n");
		if (!TowerStat->Abilities.IsEmpty()) Info += TowerUI::TagList(TowerStat->Abilities) + TEXT("\n");
	}

	Info += TEXT("\n적용 중인 효과 · 시너지 / 버프\n");
	if (TowerStat->ActiveBuffs.IsEmpty()) Info += TEXT("적용 중인 효과 없음\n");
	for (const FBuffEffect& Effect : TowerStat->ActiveBuffs)
	{
		Info += EffectLine(Effect) + TEXT(" · ") +
			(Effect.SourceKey.ToString().StartsWith(TEXT("Synergy_")) ? TEXT("시너지") : TEXT("타워 효과")) + TEXT("\n");
	}

	FString Bonuses;
	for (const auto& Stat : TowerStat->CurrentStats)
	{
		const float Base = TowerStat->GetBaseStat(Stat.Key);
		const float Diff = Stat.Value - Base;
		if (!FMath::IsNearlyZero(Diff))
			Bonuses += FString::Printf(TEXT("%s %.2f (%+.2f)\n"), *TowerUI::TagName(Stat.Key), Stat.Value, Diff);
	}
	if (!Bonuses.IsEmpty()) Info += TEXT("\n기본 대비 보너스\n") + Bonuses;
	// 다음 강화 비용·최대 레벨은 강화 버튼(TEXT_UpgradeCost)에 표시하므로 상세 패널에는 넣지 않는다.

	if (Info != LastRenderedDetails)
	{
		if (DetailedEffectsPanel) BuildDetailsPanel(Info);
		else SetText(TEXT("DetailedEffects"), Info);
	}

	SetText(TEXT("TEXT_UpgradeCost"), TowerStat->CurrentLevel < TowerStat->MaxLevel
		? FString::Printf(TEXT("강화 · %.0f G"), TowerStat->GetNextLevelGoldCost())
		: TEXT("최대 레벨"));
	if (UWidget* Skill = GetWidgetFromName(TEXT("Border_245")))
		Skill->SetVisibility(IsValid(TowerMana) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (IsValid(TowerMana))
	{
		SetText(TEXT("TextBlock_916"), TEXT("스킬 사용"));
		SetText(TEXT("Text_AutoFire"), TowerMana->bAutoFire ? TEXT("자동 ON") : TEXT("자동 OFF"));
	}
}
