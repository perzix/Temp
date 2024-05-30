#include "KMWorldMapEdModeTool.h"
#include "KMWorldMapEditor/KMWorldMapEditor.h"
#include "KMWorldMapEdMode.h"

#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(StyleSet->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)

TSharedPtr< FSlateStyleSet > KMWorldMapEdModeTool::StyleSet = nullptr;

void KMWorldMapEdModeTool::OnStartupModule()
{
	RegisterStyleSet();
	RegisterEditorMode();
}

void KMWorldMapEdModeTool::OnShutdownModule()
{
	UnregisterStyleSet();
	UnregisterEditorMode();
}

void KMWorldMapEdModeTool::RegisterStyleSet()
{
	// Const icon sizes
	const FVector2D Icon20x20(20.0f, 20.0f);
	const FVector2D Icon40x40(40.0f, 40.0f);

	// Only register once
	if (StyleSet.IsValid())
	{
		return;
	}

	StyleSet = MakeShareable(new FSlateStyleSet("ExampleEdModeToolStyle"));
	StyleSet->SetContentRoot(FPaths::ProjectDir() / TEXT("Content/EditorResources"));
	StyleSet->SetCoreContentRoot(FPaths::ProjectDir() / TEXT("Content/EditorResources"));

	// Spline editor
	{
		StyleSet->Set("ExampleEdMode", new IMAGE_BRUSH(TEXT("IconExampleEditorMode"), Icon40x40));
		StyleSet->Set("ExampleEdMode.Small", new IMAGE_BRUSH(TEXT("IconExampleEditorMode"), Icon20x20));
	}

	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
}

void KMWorldMapEdModeTool::UnregisterStyleSet()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}

void KMWorldMapEdModeTool::RegisterEditorMode()
{
	FEditorModeRegistry::Get().RegisterMode<FKMWorldMapEdMode>(
		FKMWorldMapEdMode::EM_WorldMap,
		FText::FromString("WorldMap Editor Mode"),
		FSlateIcon(StyleSet->GetStyleSetName(), "ExampleEdMode", "ExampleEdMode.Small"),
		true, 500
		);
}

void KMWorldMapEdModeTool::UnregisterEditorMode()
{
	FEditorModeRegistry::Get().UnregisterMode(FKMWorldMapEdMode::EM_WorldMap);
}

#undef IMAGE_BRUSH