// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.IO;

public class KMWorldMapEditor : ModuleRules
{
	public KMWorldMapEditor(ReadOnlyTargetRules Target) : base(Target)
    {

        //PrivatePCHHeaderFile = "ToolExampleEditor.h";  // TODO - see if adding this back in helps much with normal dev cycles

        PublicIncludePaths.AddRange(
            new string[] {
                // ... add public include paths required here ...
                "KMWorldMapEditor"
            }
            );

        PrivateIncludePaths.AddRange(
            new string[] {
                // ... add other private include paths required here ...
            }
            );

        PublicDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "Engine",
                "CoreUObject",
                "InputCore",
                "LevelEditor",
                "Slate",
                "EditorStyle",
                "AssetTools",
                "EditorWidgets",
                "UnrealEd",
                "BlueprintGraph",
                "AnimGraph",
                "ComponentVisualizers",
                "karma",
                "Landscape",
                "Json",
                "JsonUtilities",
                "Protobuf"
        }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "AppFramework",
                "SlateCore",
                "EditorFramework",
                "AnimGraph",
                "UnrealEd",
                "KismetWidgets",
                "MainFrame",
                "PropertyEditor",
                "ComponentVisualizers",
                "karma"
                // ... add private dependencies that you statically link with here ...
            }
            );
    }
}
