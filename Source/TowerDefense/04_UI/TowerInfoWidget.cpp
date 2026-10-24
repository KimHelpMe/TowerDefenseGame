#include "TowerInfoWidget.h"
#include "TowerRangeDecalComponent.h"
#include "UIEffectFormatting.h"

#include "FantasyUITheme.h"
#include "TowerDefenseGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "TowerStatComponent.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

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
}

void UTowerInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();
	FFantasyUITheme::ApplyToWidgetTree(this);
}


void UTowerInfoWidget::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
    Super::NativeTick(Geometry, DeltaTime);
    if (!IsValid(TowerStat)) return;
    auto SetText=[this](const TCHAR* Name,const FString& Value) { if (UTextBlock* Text=Cast<UTextBlock>(GetWidgetFromName(Name))) { const FText New=FText::FromString(Value); if(!Text->GetText().EqualTo(New)) Text->SetText(New); } };
    auto EffectLine=[](const FBuffEffect& E) { return FString::Printf(TEXT("%s %+.2f%s"),*TowerUI::TagName(E.StatTag), E.ValueType==EBuffValueType::Percent ? E.Value*100.f:E.Value,E.ValueType==EBuffValueType::Percent?TEXT("%"):TEXT("")); };
    SetText(TEXT("TEXT_TowerName"),TowerStat->TowerName);
    SetText(TEXT("TEXT_TowerMeta"),TowerStat->Grade+TEXT(" · ")+TowerStat->TowerType);
    SetText(TEXT("TEXT_Level"),FString::Printf(TEXT("%d / %d"),TowerStat->CurrentLevel,TowerStat->MaxLevel));
    if(UImage* Icon=Cast<UImage>(GetWidgetFromName(TEXT("TowerIcon")))) Icon->SetBrushFromTexture(TowerStat->TowerIcon);
    const FName Key(*FString::Printf(TEXT("%s_Lv%d"),*TowerStat->TowerRowName.ToString(),TowerStat->CurrentLevel));
    const FTowerDataRow* Row=TowerStat->TowerDataTable?TowerStat->TowerDataTable->FindRow<FTowerDataRow>(Key,TEXT("Tower UI")):nullptr;
    const bool bSupport=Row && Row->BuffSetting.BuffType!=EBuffComponentType::None;
    FString Info;
    if(bSupport)
    {
        const FBuffConfig& B=Row->BuffSetting;
        SetText(TEXT("LBL_ATK"),B.BuffEffects.IsEmpty()?TEXT("버프 효과"):TowerUI::TagName(B.BuffEffects[0].StatTag));
        SetText(TEXT("TEXT_ATK"),B.BuffEffects.IsEmpty()?TEXT("없음"):FString::Printf(TEXT("%+.2f%s"),B.BuffEffects[0].ValueType==EBuffValueType::Percent?B.BuffEffects[0].Value*100.f:B.BuffEffects[0].Value,B.BuffEffects[0].ValueType==EBuffValueType::Percent?TEXT("%"):TEXT("")));
        SetText(TEXT("LBL_ATK_Speed"),TEXT("지속시간"));SetText(TEXT("TEXT_ATK_Speed"),B.BuffType==EBuffComponentType::Passive?TEXT("범위 내 유지"):FString::Printf(TEXT("%.1f초"),B.BuffDuration));
        SetText(TEXT("LBL_Range"),TEXT("버프 범위"));SetText(TEXT("TEXT_Range"),FString::Printf(TEXT("%.0f"),IsValid(OwnerTower)?OwnerTower->GetRangeRadius():B.BuffRadius));
        SetText(TEXT("LBL_Type"),TEXT("적용 대상"));SetText(TEXT("TEXT_Type"),B.BuffTarget==EBuffTarget::AllyTower?TEXT("아군 타워"):TEXT("적"));
        Info=B.BuffType==EBuffComponentType::Active?TEXT("액티브 효과 · 자동 발동\n"):TEXT("지속 효과\n");
        for(const FBuffEffect& E:B.BuffEffects) Info+=EffectLine(E)+TEXT("\n");
        if(B.BuffType==EBuffComponentType::Active) Info+=FString::Printf(TEXT("지속 %.1f초 · 재사용 %.1f초\n"),B.BuffDuration,B.BuffCooldown);
        else Info+=FString::Printf(TEXT("적용 주기 %.1f초 · 범위 내 유지\n"),B.BuffInterval);
    }
    else
    {
        SetText(TEXT("LBL_ATK"),TEXT("공격력"));SetText(TEXT("TEXT_ATK"),FString::Printf(TEXT("%.0f"),TowerStat->GetStat(UStatComponent::GetTag(TEXT("Stat.AttackDamage")))));
        float SpeedBonus=0.f;
        if(const ATowerDefenseGameMode* Mode=Cast<ATowerDefenseGameMode>(UGameplayStatics::GetGameMode(this)))
            for(const FBuffEffect& Effect:Mode->InGameUpgrades)
                if(Effect.StatTag==UStatComponent::GetTag(TEXT("Upgrade.AttackSpeed"))) SpeedBonus+=Effect.Value;
        const float Interval=FMath::Max(0.01f,TowerStat->GetStat(UStatComponent::GetTag(TEXT("Stat.AttackSpeed")))*(1.f-SpeedBonus));
        SetText(TEXT("LBL_ATK_Speed"),TEXT("초당 공격 횟수"));SetText(TEXT("TEXT_ATK_Speed"),FString::Printf(TEXT("%.2f /s"),1.f/Interval));
        SetText(TEXT("LBL_Range"),TEXT("공격 사거리"));SetText(TEXT("TEXT_Range"),FString::Printf(TEXT("%.0f"),IsValid(OwnerTower)?OwnerTower->GetRangeRadius():0.f));
        SetText(TEXT("LBL_Type"),TEXT("타워 유형"));SetText(TEXT("TEXT_Type"),TowerStat->TowerType);
        Info=TEXT("공격 효과\n");
        if(!TowerStat->TowerInfo.IsEmpty() && TowerStat->TowerInfo!=TEXT("None")) Info+=TowerStat->TowerInfo+TEXT("\n");
        if(!TowerStat->Elements.IsEmpty()) Info+=TowerUI::TagList(TowerStat->Elements)+TEXT("\n");
        if(!TowerStat->AttackTypes.IsEmpty()) Info+=TowerUI::TagList(TowerStat->AttackTypes)+TEXT("\n");
        if(!TowerStat->Abilities.IsEmpty()) Info+=TowerUI::TagList(TowerStat->Abilities)+TEXT("\n");
    }
    Info+=TEXT("\n적용 중인 효과 · 시너지 / 버프\n");
    if(TowerStat->ActiveBuffs.IsEmpty()) Info+=TEXT("적용 중인 효과 없음\n");
    for(const FBuffEffect& E:TowerStat->ActiveBuffs) Info+=EffectLine(E)+TEXT(" · ")+(E.SourceKey.ToString().StartsWith(TEXT("Synergy_")) ? TEXT("시너지") : TEXT("타워 효과"))+TEXT("\n");
    FString Bonuses;
    for(const auto& Stat:TowerStat->CurrentStats)
    {
        const float Base=TowerStat->GetBaseStat(Stat.Key),Diff=Stat.Value-Base;
        if(!FMath::IsNearlyZero(Diff)) Bonuses+=FString::Printf(TEXT("%s %.2f (%+.2f)\n"),*TowerUI::TagName(Stat.Key),Stat.Value,Diff);
    }
    if(!Bonuses.IsEmpty()) Info+=TEXT("\n기본 대비 보너스\n")+Bonuses;
    Info+=TowerStat->CurrentLevel<TowerStat->MaxLevel?FString::Printf(TEXT("\n다음 강화 %.0f G"),TowerStat->GetNextLevelGoldCost()):TEXT("\n최대 레벨");
    SetText(TEXT("DetailedEffects"),Info);
    SetText(TEXT("TEXT_UpgradeCost"), TowerStat->CurrentLevel<TowerStat->MaxLevel ? FString::Printf(TEXT("강화 · %.0f G"),TowerStat->GetNextLevelGoldCost()) : TEXT("최대 레벨"));
    if(UWidget* Skill=GetWidgetFromName(TEXT("Border_245"))) Skill->SetVisibility(IsValid(TowerMana)?ESlateVisibility::Visible:ESlateVisibility::Collapsed);
    if(IsValid(TowerMana))
    {
        SetText(TEXT("TextBlock_916"),TEXT("스킬 사용"));
        SetText(TEXT("Text_AutoFire"),TowerMana->bAutoFire?TEXT("자동 ON"):TEXT("자동 OFF"));
    }
}
