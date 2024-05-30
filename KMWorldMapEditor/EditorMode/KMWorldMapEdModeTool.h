#pragma once

#include "KMWorldMapEditor/KMWorldMapTabToolBase.h"

class KMWorldMapEdModeTool : public FKMWorldMapTabToolBase
{
public:
	virtual void OnStartupModule() override;
	virtual void OnShutdownModule() override;

	virtual ~KMWorldMapEdModeTool() {}
private:
	static TSharedPtr< class FSlateStyleSet > StyleSet;

	void RegisterStyleSet();
	void UnregisterStyleSet();

	void RegisterEditorMode();
	void UnregisterEditorMode();
};