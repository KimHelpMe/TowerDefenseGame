#include "TowerDefenseTags.h"

namespace TDTags
{
	UE_DEFINE_GAMEPLAY_TAG(Stat_HP, "Stat.HP");
	UE_DEFINE_GAMEPLAY_TAG(Stat_MaxHP, "Stat.MaxHP");
	UE_DEFINE_GAMEPLAY_TAG(Stat_MoveSpeed, "Stat.MoveSpeed");
	UE_DEFINE_GAMEPLAY_TAG(Stat_AttackDamage, "Stat.AttackDamage");
	UE_DEFINE_GAMEPLAY_TAG(Stat_AttackSpeed, "Stat.AttackSpeed");
	UE_DEFINE_GAMEPLAY_TAG(Stat_AttackRange, "Stat.AttackRange");
	UE_DEFINE_GAMEPLAY_TAG(Stat_Defense, "Stat.Defense");

	UE_DEFINE_GAMEPLAY_TAG(Ability_AttackAir, "Ability.AttackAir");
	UE_DEFINE_GAMEPLAY_TAG(Ability_AttackGround, "Ability.AttackGround");

	UE_DEFINE_GAMEPLAY_TAG(AttackTypes_Physical, "AttackTypes.Physical");
	UE_DEFINE_GAMEPLAY_TAG(AttackTypes_Magic, "AttackTypes.Magic");
	UE_DEFINE_GAMEPLAY_TAG(AttackTypes_Chaos, "AttackTypes.Chaos");

	UE_DEFINE_GAMEPLAY_TAG(Element_Fire, "Element.Fire");
	UE_DEFINE_GAMEPLAY_TAG(Element_Water, "Element.Water");
	UE_DEFINE_GAMEPLAY_TAG(Element_Wind, "Element.Wind");
	UE_DEFINE_GAMEPLAY_TAG(Element_Earth, "Element.Earth");

	UE_DEFINE_GAMEPLAY_TAG(Upgrade_AttackRange, "Upgrade.AttackRange");
	UE_DEFINE_GAMEPLAY_TAG(Upgrade_AttackSpeed, "Upgrade.AttackSpeed");
	UE_DEFINE_GAMEPLAY_TAG(Upgrade_ManaRegen, "Upgrade.ManaRegen");
	UE_DEFINE_GAMEPLAY_TAG(Upgrade_BuffDuration, "Upgrade.BuffDuration");
	UE_DEFINE_GAMEPLAY_TAG(Upgrade_DefensePenetration, "Upgrade.DefensePenetration");
	UE_DEFINE_GAMEPLAY_TAG(Upgrade_AttackDamage_Physical, "Upgrade.AttackDamage.Physical");
	UE_DEFINE_GAMEPLAY_TAG(Upgrade_AttackDamage_Magic, "Upgrade.AttackDamage.Magic");
}
