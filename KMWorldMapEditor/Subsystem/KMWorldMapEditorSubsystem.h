// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/UnrealEditorSubsystem.h"
#include "KMWorldMapEditorSubsystem.generated.h"

class FMapInfo;

UENUM()
enum class EWorldMapEditorControlType : uint8
{
	None UMETA(DisplayName = "None"),
	Brush UMETA(DisplayName = "Brush"),
	Eraser UMETA(DisplayName = "Eraser"),
	Select UMETA(DisplayName = "Select"),
};
DECLARE_MULTICAST_DELEGATE(FOnMapInfoChanged);
DECLARE_MULTICAST_DELEGATE(FOnBrushSizeChanged);
DECLARE_MULTICAST_DELEGATE(FOnBrushColorChanged);
DECLARE_MULTICAST_DELEGATE(FOnControlTypeChanged);
DECLARE_MULTICAST_DELEGATE(FOnTileInfoChanged);
DECLARE_MULTICAST_DELEGATE(FOnSectorAlphaChanged);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSectorSelected, FIntPoint);

/**
 * 
 */
UCLASS()
class KMWORLDMAPEDITOR_API UKMWorldMapEditorSubsystem : public UUnrealEditorSubsystem
{
	GENERATED_BODY()

public:
	static UKMWorldMapEditorSubsystem* Get();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
public:
	const TArray<FColor>& GetColorBlocks() const { return ColorBlocks; }

	TSharedPtr<FMapInfo> GetCurrentMapInfo() { return CurrentMapInfo; }
	void InitMapInfo();
	void SetCurrentMapInfo(TSharedPtr<FMapInfo> NewMapInfo);
	
	int32 GetBrushSize() const { return BrushSize; }
	void SetBrushSize(int32 NewBrushSize);

	const FLinearColor GetBrushColor() const { return BrushColor; }
	void SetBrushColor(FLinearColor NewBrushColor);

	const EWorldMapEditorControlType GetControlType() const { return CurrentControlType; }
	const FString GetControlTypeString() const;
	void SetControlType(EWorldMapEditorControlType NewControlType);

	const float GetSectorAlpha() const { return SectorAlpha; }
	void SetSectorAlpha(float NewSectorAlpha);

	const FString& GetOpenFileName() const { return OpenFileName; }
	void SetOpenFileName(const FString& NewOpenFileName);

	const int32 GetWorldSize() const { return WorldSize; }

	void Spline();
	void ShowSelectedSector(int32 SectorId);
	TMap<int32, TArray<FIntPoint>> GetBoundaryTiles();
	TArray<FIntPoint> GetBoundaryTiles(int32 InSectorId);
	void LinkPoints(TArray<FIntPoint>& InPoints);
	void CreateBoundarySpline(const TArray<FIntPoint>& InBoundaryTiles);
	const FVector TileIndexToWorldPosition(const FIntPoint TileIndex);
	const FIntPoint GetTileIndex(const FVector& HitLocation);

	void UpdateTileColor(TArray<FIntPoint> TileIndexArray, FColor BrushColor, bool bIsRemove);
	void PrintTileInfo(const FVector& InWorldPosition);
	FVector GetMousePositionToWorldPosition(FEditorViewportClient* ViewportClient, FViewport* Viewport, const FVector2D& MousePosition);
	void RefreshSectorId();
	float GetLandscapeHeight(const FVector& InWorldPosition);

	bool IsBrushLineMode() const { return bIsBrushLineMode; }
	void SetBrushLineMode(bool bNewBrushLineMode) { bIsBrushLineMode = bNewBrushLineMode; }
public:
	FOnMapInfoChanged& GetOnMapInfoChanged() { return OnMapInfoChanged; }
	FOnBrushSizeChanged& GetOnBrushSizeChanged() { return OnBrushSizeChanged; }
	FOnBrushColorChanged& GetOnBrushColorChanged() { return OnBrushColorChanged; }
	FOnControlTypeChanged& GetOnControlTypeChanged() { return OnControlTypeChanged; }
	FOnTileInfoChanged& GetOnTileInfoChanged() { return OnTileColorChanged; }
	FOnSectorAlphaChanged& GetOnSectorAlphaChanged() { return OnSectorAlphaChanged; }
	FOnSectorSelected& GetOnTileSelected() { return OnTileSelected; }
private:
	FOnMapInfoChanged OnMapInfoChanged;
	FOnBrushSizeChanged OnBrushSizeChanged;
	FOnBrushColorChanged OnBrushColorChanged;
	FOnControlTypeChanged OnControlTypeChanged;
	FOnTileInfoChanged OnTileColorChanged;
	FOnSectorAlphaChanged OnSectorAlphaChanged;
	FOnSectorSelected OnTileSelected;
private:

	TArray<FColor> ColorBlocks{ FColor::Green, FColor::Red, FColor::Blue, FColor::Orange, FColor::Cyan, FColor::Magenta, FColor::Yellow, FColor::Purple }; //섹터 아이디에 따라 다른 컬러를 가져야함
	int32 BrushSize = 1;
	int32 WorldSize;			
	FLinearColor BrushColor = FLinearColor::Green;
	TSharedPtr<FMapInfo> CurrentMapInfo;
	bool bIsBrushLineMode = false;
	void OnLevelLoaded(const FString& MapName, bool bAsTemplate);
	void OnLevelsChanged();
	
	
	UWorld* GetEditorWorld();
	void ClearMapBorder();
	FTimerDelegate TimerDelegate;
	FTimerHandle TimerHandle;
	EWorldMapEditorControlType CurrentControlType = EWorldMapEditorControlType::Brush;
	float SectorAlpha = 0.7f;
	FString OpenFileName;
};
