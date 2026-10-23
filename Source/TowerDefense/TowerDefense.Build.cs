// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TowerDefense : ModuleRules
{
	public TowerDefense(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Niagara", "UMG", "GameplayTags", "EnhancedInput" });

        PublicIncludePaths.AddRange(new string[] {"TowerDefense/01_Tower", 
											      "TowerDefense/02_Enemy",
											      "TowerDefense/03_Core",
											      "TowerDefense/04_UI",
												  "TowerDefense/05_Gacha",
                                                  "TowerDefense/06_Components",
                                                  "TowerDefense/06_Components/Attack",
                                                  "TowerDefense/06_Components/Buff",
                                                  "TowerDefense/06_Components/Stat",
                                                  "TowerDefense/06_Components/Mana",
                                                  "TowerDefense/06_Components/Enemy",
                                                  "TowerDefense/07_Data"
                                                                        });


        PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
