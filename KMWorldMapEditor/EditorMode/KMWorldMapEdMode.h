#pragma once

#include "EditorModes.h"
#include "EdMode.h"

class WorldMapInfo;
class UKMWorldMapEditorSubsystem;
class FKMWorldMapEdMode : public FEdMode
{
public:

	const static FEditorModeID EM_WorldMap;

	// FEdMode interface
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;
	virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;
	virtual bool HandleClick(FEditorViewportClient* InViewportClient, HHitProxy *HitProxy, const FViewportClick &Click) override;
	virtual bool InputDelta(FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector& InDrag, FRotator& InRot, FVector& InScale) override;
	virtual bool InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event) override;
	virtual bool MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 X, int32 Y) override;
	virtual bool CapturedMouseMove(FEditorViewportClient* InViewportClient, FViewport* InViewport, int32 InMouseX, int32 InMouseY) override;
	virtual bool ShowModeWidgets() const override;
	virtual bool ShouldDrawWidget() const override;
	virtual bool UsesTransformWidget() const override;
	virtual bool IsSelectionAllowed(AActor* InActor, bool bInSelection) const override;
	//virtual bool GetCustomDrawingCoordinateSystem(FMatrix& InMatrix, void* InData) override;
	//virtual bool GetCustomInputCoordinateSystem(FMatrix& InMatrix, void* InData) override;
	//virtual void ActorSelectionChangeNotify() override;
	//virtual void MapChangeNotify() override;
	//virtual void SelectionChanged() override;
	//virtual bool IsCompatibleWith(FEditorModeID OtherModeID) const override;
	// End of FEdMode interface

	FKMWorldMapEdMode();
	~FKMWorldMapEdMode();

	bool IsSelectionAllowed() const;
	void SetSelectionAllowed(bool bInSelectionAllowed);


	TSharedPtr<FUICommandList> ExampleEdModeActions;
	void MapCommands();
	UPROPERTY()
	TWeakObjectPtr<class AWorldMapTileActor> WorldMapTileActor;

private:
	bool bPlayInEditor = false;
	bool bIsSelectionAllowed = true;
	FKey MouseButtonHoldType;

	UKMWorldMapEditorSubsystem* GetWorldMapEditorSubsystem();
	FTimerHandle TimerHandle;
};