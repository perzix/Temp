#pragma once

#include "Toolkits/BaseToolkit.h"
#include "KMWorldMapEdMode.h"
#include "SKMWorldMapEdModeWidget.h"

class FKMWorldMapEdModeToolkit: public FModeToolkit
{
public:

	FKMWorldMapEdModeToolkit()
	{
		SAssignNew(WorldMapEdModeWidget, SKMWorldMapEdModeWidget);
	}

	/** IToolkit interface */
	virtual FName GetToolkitFName() const override { return FName("WorldMapEdMode"); }
	virtual FText GetBaseToolkitName() const override { return NSLOCTEXT("BuilderModeToolkit", "DisplayName", "Builder"); }
	virtual class FEdMode* GetEditorMode() const override { return GLevelEditorModeTools().GetActiveMode(FKMWorldMapEdMode::EM_WorldMap); }
	virtual TSharedPtr<class SWidget> GetInlineContent() const override { return WorldMapEdModeWidget; }

private:

	TSharedPtr<SKMWorldMapEdModeWidget> WorldMapEdModeWidget;
};
