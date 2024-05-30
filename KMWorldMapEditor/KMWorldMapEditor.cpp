// Fill out your copyright notice in the Description page of Project Settings.

#include "KMWorldMapEditor.h"
#include "MenuTool/MenuTool.h"
#include "TabTool/TabTool.h"
#include "EditorMode/KMWorldMapEdModeTool.h"

#include "ISettingsModule.h"
#include "Developer/Settings/Public/ISettingsContainer.h"
#include "CustomProjectSettings/ExampleSettings.h"

IMPLEMENT_GAME_MODULE(FKMWorldMapEditor, ToolExampleEditor)

TSharedRef<FWorkspaceItem> FKMWorldMapEditor::MenuRoot = FWorkspaceItem::NewGroup(FText::FromString("Menu Root"));

void FKMWorldMapEditor::AddModuleListeners()
{
	// add tools
	/*ModuleListeners.Add(MakeShareable(new MenuTool));
	ModuleListeners.Add(MakeShareable(new TabTool));*/
	ModuleListeners.Add(MakeShareable(new KMWorldMapEdModeTool));
}

void FKMWorldMapEditor::StartupModule()
{
	if (!IsRunningCommandlet())
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
		LevelEditorMenuExtensibilityManager = LevelEditorModule.GetMenuExtensibilityManager();
		MenuExtender = MakeShareable(new FExtender);
		MenuExtender->AddMenuBarExtension("Window", EExtensionHook::After, NULL, FMenuBarExtensionDelegate::CreateRaw(this, &FKMWorldMapEditor::MakePulldownMenu));
		LevelEditorMenuExtensibilityManager->AddExtender(MenuExtender);
	}
	//{		
	//	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	//	// add custom category
	//	EAssetTypeCategories::Type ExampleCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("Example")), FText::FromString("Example"));
	//	// register our custom asset with example category
	//	TSharedPtr<IAssetTypeActions> Action = MakeShareable(new FExampleDataTypeActions(ExampleCategory));
	//	AssetTools.RegisterAssetTypeActions(Action.ToSharedRef());
	//	// saved it here for unregister later
	//	CreatedAssetTypeActions.Add(Action);
	//}

	// register settings:
	{
		ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
		if (SettingsModule)
		{
			TSharedPtr<ISettingsContainer> ProjectSettingsContainer = SettingsModule->GetContainer("Project");
			ProjectSettingsContainer->DescribeCategory("ExampleCategory", FText::FromString("Example Category"), FText::FromString("Example settings description text here"));

			SettingsModule->RegisterSettings("Project", "ExampleCategory", "ExampleSettings",
				FText::FromString("Example Settings"),
				FText::FromString("Configure Example Settings"),
				GetMutableDefault<UExampleSettings>()
			);
		}
	}
	
	IKMWorldMapModuleInterface::StartupModule();
}

void FKMWorldMapEditor::ShutdownModule()
{	
	
	//// Unregister all the asset types that we registered
	//if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	//{
	//	IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
	//	for (int32 i = 0; i < CreatedAssetTypeActions.Num(); ++i)
	//	{
	//		AssetTools.UnregisterAssetTypeActions(CreatedAssetTypeActions[i].ToSharedRef());
	//	}
	//}
	CreatedAssetTypeActions.Empty();

	// unregister settings
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule)
	{
		SettingsModule->UnregisterSettings("Project", "ExampleCategory", "ExampleSettings");
	}

	IKMWorldMapModuleInterface::ShutdownModule();
}


void FKMWorldMapEditor::AddMenuExtension(const FMenuExtensionDelegate &extensionDelegate, FName extensionHook, const TSharedPtr<FUICommandList> &CommandList, EExtensionHook::Position position)
{
	MenuExtender->AddMenuExtension(extensionHook, position, CommandList, extensionDelegate);
}

void FKMWorldMapEditor::MakePulldownMenu(FMenuBarBuilder &menuBuilder)
{
	/*menuBuilder.AddPullDownMenu(
		FText::FromString("Example"),
		FText::FromString("Open the Example menu"),
		FNewMenuDelegate::CreateRaw(this, &FKMWorldMapEditor::FillPulldownMenu),
		"Example",
		FName(TEXT("ExampleMenu"))
	);*/
}

void FKMWorldMapEditor::FillPulldownMenu(FMenuBuilder &menuBuilder)
{
	menuBuilder.BeginSection("ExampleSection", FText::FromString("Section 1"));
	menuBuilder.AddMenuSeparator(FName("Section_1"));
	menuBuilder.EndSection();

	menuBuilder.BeginSection("ExampleSection", FText::FromString("Section 2"));
	menuBuilder.AddMenuSeparator(FName("Section_2"));
	menuBuilder.EndSection();
}
