#pragma once

#include "Framework/APplication/SlateApplication.h"

class UKMWorldMapEditorSubsystem;
class SGridPanel;

class SKMWorldMapEdModeWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SKMWorldMapEdModeWidget) {}
	SLATE_END_ARGS();

	void Construct(const FArguments& InArgs);

	void UpdateDynamicUIContainer();
	
	// Util Functions
	class FKMWorldMapEdMode* GetEdMode() const;

	FReply OnMakeNewFile();
	FReply OnLoadJsonFile();
	FReply OnLoadProtoFile();
	bool CanLoadFile() const;
	FReply OnSaveJsonFile();
	FReply OnSaveProtoFile();
	bool CanSaveFile() const;
	FReply OnSetWorldSize(int32 NewWorldSize);
	FLinearColor GetCurrentColor() const;
	FReply OnColorBlockClicked(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	void OnColorCommitted(FLinearColor NewColor);
	TSharedRef<SWidget> GenerateWidgetForBrushSize(TSharedPtr<int32> BrushSize);
	void OnBrushSizeSelected(TSharedPtr<int32> SelectedBrushSize, ESelectInfo::Type SelectInfo);
	TSharedRef<SVerticalBox> DefaultSelectContents();
	void OnTileSelected(FIntPoint TileIndex);

	float GetSliderValue() const;
	void SetSliderValue(float NewValue);
private:
	TArray<TSharedPtr<int32>> BrushSizes;
	TArray<FColor> ColorBlocks; //섹터 아이디에 따라 다른 컬러를 가져야함

	UKMWorldMapEditorSubsystem* WorldMapEditorSubsystem;
	float SliderValue = 10.f;

	TSharedPtr<SBox> DynamicUIContainer;
	TSharedPtr<STextBlock> OpenFileNameTextBlock;
	TSharedRef<SGridPanel> CreateColorGridPanel();
	TSharedRef<SVerticalBox> CreateSelectSectorPanel();
	

};