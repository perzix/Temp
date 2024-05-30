// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldMapTileActor.generated.h"
class FMapInfo;
class UKMWorldMapEditorSubsystem;
UCLASS()
class KMWORLDMAPEDITOR_API AWorldMapTileActor : public AActor
{
	GENERATED_BODY()
private:
	static const int32 MaxUndoCount = 100;
public:	
	
	// Sets default values for this actor's properties
	AWorldMapTileActor();
	~AWorldMapTileActor();
	void Init();
	void InitWorldTileTexture();
	UFUNCTION(BlueprintCallable, CallInEditor)
	void WriteGridTexture();
	void MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 x, int32 y);
	void UpdateMapTileTexture();
	void MouseClick(FEditorViewportClient* InViewportClient, HHitProxy* HitProxy, const FViewportClick& Click);
	void MouseClick(bool bIsDrag, bool bIsEraser);
	void DragLine(bool bIsRemove);
	void ProcessUndo();
	
	
protected:
	const FIntPoint GetTileIndex(int32 WorldHalfSize, int32 TileSize, FVector StartLocation, const FVector& HitLocation);
	const TArray<FIntPoint> GetTileIndexArray(int32 WorldHalfSize, int32 TileSize, FVector StartLocation, const FVector& HitLocation);
	const TArray<FIntPoint> GetTileIndexArray(const FIntPoint& TileIndex);
	int32 GetWorldSizeFromLandscape();
	void OnBrushSizeChanged();
	void OnBrushColorChanged();
	void OnMapInfoChanged();
	void OnControlTypeChanged();
	void OnSectorAlphaChanged();
	UPROPERTY(EditAnywhere)
	TObjectPtr<UDecalComponent> GridDecalComponent;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UDecalComponent> WorldTileDecalComponent;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UDecalComponent> BrushDecalComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<class UMaterialInstance> GridDecalMaterial;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<class UMaterialInstance> WorldTileDecalMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<class UMaterialInstance> BrushDecalMaterial;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USceneComponent> DefaultSceneRoot;

private:
	UPROPERTY()
	TWeakObjectPtr<UMaterialInstanceDynamic> DynamicGridMaterial;
	UPROPERTY()
	TWeakObjectPtr<UMaterialInstanceDynamic> DynamicWorldTileMaterial;
	UPROPERTY()
	TWeakObjectPtr<UMaterialInstanceDynamic> DynamicBrushMaterial;
	UPROPERTY()
	TObjectPtr<UTexture2D> MapTileTexture;
	UPROPERTY()
	TMap<FIntPoint, FColor> MouseClickCacheData;
	TArray<TArray<TTuple<FIntPoint, FColor>>> UndoDatas;

	FIntPoint DragLastTileIndex;
	FVector2D DragLineDirection;
	float TextureScale;
	FTimerHandle TimerHandle;
};
