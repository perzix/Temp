#include "TabTool.h"
//#include "AssetRegistryModule.h"
//#include "ScopedTransaction.h"
//#include "SDockTab.h"
//#include "SDockableTab.h"
//#include "SDockTabStack.h"
//#include "SlateApplication.h"
#include "KMWorldMapEditor/KMWorldMapEditor.h"
#include "TabToolPanel.h"

void TabTool::OnStartupModule()
{
	FKMWorldMapTabToolBase::OnStartupModule();
	//FKMWorldMapEditor::Get().AddMenuExtension(FMenuExtensionDelegate::CreateRaw(this, &TabTool::MakeMenuEntry), FName("Section_2"));
}

void TabTool::OnShutdownModule()
{
	FKMWorldMapTabToolBase::OnShutdownModule();
}

void TabTool::Initialize()
{
	TabName = "TabTool";
	TabDisplayName = FText::FromString("Tab Tool");
	ToolTipText = FText::FromString("Tab Tool Window");
}

TSharedRef<SDockTab> TabTool::SpawnTab(const FSpawnTabArgs& TabSpawnArgs)
{
	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(TabToolPanel)
			.Tool(SharedThis(this))
		];

	return SpawnedTab;
}