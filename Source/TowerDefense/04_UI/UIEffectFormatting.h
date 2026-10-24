#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
namespace TowerUI
{
inline FString TagName(const FGameplayTag& Tag)
{
    static const TMap<FString,FString> Labels = {
        {TEXT("Stat.AttackDamage"),TEXT("공격력")}, {TEXT("Stat.AttackSpeed"),TEXT("공격 간격(초)")},
        {TEXT("Stat.AttackRange"),TEXT("공격 사거리")}, {TEXT("Stat.Defense"),TEXT("방어력")},
        {TEXT("Stat.HP"),TEXT("체력")}, {TEXT("Stat.MaxHP"),TEXT("최대 체력")},
        {TEXT("Stat.MoveSpeed"),TEXT("이동 속도")}, {TEXT("Stat.Mana"),TEXT("마나")},
        {TEXT("Stat.MaxMana"),TEXT("최대 마나")}, {TEXT("Stat.ManaRegen"),TEXT("마나 회복")},
        {TEXT("Ability.AttackAir"),TEXT("공중 공격")}, {TEXT("Ability.AttackGround"),TEXT("지상 공격")},
        {TEXT("AttackTypes.Physical"),TEXT("물리 공격")}, {TEXT("AttackTypes.Magic"),TEXT("마법 공격")}, {TEXT("AttackTypes.Chaos"),TEXT("혼돈 공격")},
        {TEXT("Element.Fire"),TEXT("화염")}, {TEXT("Element.Water"),TEXT("물")}, {TEXT("Element.Wind"),TEXT("바람")}, {TEXT("Element.Earth"),TEXT("대지")}
    };
    if (const FString* Label = Labels.Find(Tag.ToString())) return *Label;
    FString Leaf;
    if (Tag.ToString().Split(TEXT("."), nullptr, &Leaf, ESearchCase::IgnoreCase, ESearchDir::FromEnd)) return Leaf;
    return Tag.ToString();
}
inline FString TagList(const FGameplayTagContainer& Tags)
{
    TArray<FString> Labels;
    for (const FGameplayTag& Tag : Tags) Labels.Add(TagName(Tag));
    return FString::Join(Labels, TEXT(" · "));
}}
