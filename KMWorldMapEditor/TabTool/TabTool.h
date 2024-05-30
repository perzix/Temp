#pragma once

#include "KMWorldMapEditor/KMWorldMapTabToolBase.h"

class TabTool : public FKMWorldMapTabToolBase
{
public:
	virtual ~TabTool() {}
	virtual void OnStartupModule() override;
	virtual void OnShutdownModule() override;
	virtual void Initialize() override;
	virtual TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& TabSpawnArgs) override;
};