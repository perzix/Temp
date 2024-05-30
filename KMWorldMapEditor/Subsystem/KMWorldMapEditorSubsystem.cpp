// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/KMWorldMapEditorSubsystem.h"
#include "Tile/MapBorderActor.h"
#include "EngineUtils.h"
#include "Landscape.h"
#include "MapInfo.h"
#include "WorldMode/KMWorldGameMode.h"

UKMWorldMapEditorSubsystem* UKMWorldMapEditorSubsystem::Get()
{
	if (GEditor == nullptr )
	{
		return nullptr;
	}
	return GEditor->GetEditorSubsystem<UKMWorldMapEditorSubsystem>();
}

void UKMWorldMapEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	//Collection.InitializeDependency(UKMWorldMapEditorSubsystem::StaticClass());
	Super::Initialize(Collection);
	FEditorDelegates::OnMapOpened.AddUObject(this, &UKMWorldMapEditorSubsystem::OnLevelLoaded);
	BrushSize = 1;
	OnMapInfoChanged.Clear();
	OnBrushSizeChanged.Clear();
	OnBrushColorChanged.Clear();
	OnControlTypeChanged.Clear();
	OnTileColorChanged.Clear();
	UE_LOG(LogTemp, Warning, TEXT("UKMWorldMapEditorSubsystem::Initialize"));
	//TimerDelegate.BindUObject(this, UKMWorldMapEditorSubsystem::UpdateSectorInfo);
}

void UKMWorldMapEditorSubsystem::Deinitialize()
{
	Super::Deinitialize();
	FEditorDelegates::OnMapOpened.RemoveAll(this);
	OnMapInfoChanged.Clear();
	OnBrushSizeChanged.Clear();
	OnBrushColorChanged.Clear();
	OnControlTypeChanged.Clear();
	OnTileColorChanged.Clear();
	UE_LOG(LogTemp, Warning, TEXT("UKMWorldMapEditorSubsystem::Deinitialize"));
}

void UKMWorldMapEditorSubsystem::InitMapInfo()
{
	FMapInfo* MapInfo = new FMapInfo();
	TSharedPtr<FMapInfo> SharedMapInfo = MakeShareable(MapInfo);
	SetCurrentMapInfo(SharedMapInfo);
}

void UKMWorldMapEditorSubsystem::SetCurrentMapInfo(TSharedPtr<FMapInfo> NewMapInfo)
{
	ClearMapBorder();
	CurrentMapInfo = NewMapInfo; 
	OnMapInfoChanged.Broadcast();
	// Ensure that the broadcast is done on the main thread
	//if (IsInGameThread())
	//{
	//	OnMapInfoChanged.Broadcast();
	//}
	//else
	//{
	//	// If we're not in the game thread, set a timer to do the broadcast on the next tick
	//	FTimerDelegate TimerDel;
	//	TimerDel.BindLambda([this]()
	//		{
	//			OnMapInfoChanged.Broadcast();
	//		});

	//	GetWorld()->GetTimerManager().SetTimerForNextTick(TimerDel);
	//}
}

void UKMWorldMapEditorSubsystem::SetBrushSize(int32 NewBrushSize)
{
	BrushSize = NewBrushSize; OnBrushSizeChanged.Broadcast();
}

void UKMWorldMapEditorSubsystem::SetBrushColor(FLinearColor NewBrushColor)
{
	BrushColor = NewBrushColor; OnBrushColorChanged.Broadcast();
}

const FString UKMWorldMapEditorSubsystem::GetControlTypeString() const
{
	switch (CurrentControlType)
	{
	case EWorldMapEditorControlType::None:
		return TEXT("None");
	case EWorldMapEditorControlType::Brush:
		return TEXT("Brush");
	case EWorldMapEditorControlType::Eraser:
		return TEXT("Eraser");
	case EWorldMapEditorControlType::Select:
		return TEXT("Select");
	default:
		return TEXT("Unknown");
	}
}

void UKMWorldMapEditorSubsystem::SetControlType(EWorldMapEditorControlType NewControlType)
{
	bool bIsChanged = CurrentControlType != NewControlType;
	CurrentControlType = NewControlType;
	if (bIsChanged)
	{
		OnControlTypeChanged.Broadcast();
	}
	if (CurrentControlType == EWorldMapEditorControlType::Select)
	{
		
	}
	else
	{
		ClearMapBorder();
	}

}

void UKMWorldMapEditorSubsystem::SetSectorAlpha(float NewSectorAlpha)
{
	SectorAlpha = NewSectorAlpha;
	OnSectorAlphaChanged.Broadcast();
}

void UKMWorldMapEditorSubsystem::SetOpenFileName(const FString& NewOpenFileName)
{
	OpenFileName = NewOpenFileName;
}

void UKMWorldMapEditorSubsystem::Spline()
{
	ClearMapBorder();
	if (GetCurrentMapInfo().IsValid() == false)
	{
		return;
	}
	TMap<int32, TArray<FIntPoint>> BoundaryTiles = GetBoundaryTiles();
	for (const auto& Elem : BoundaryTiles)
	{
		const TArray<FIntPoint> BoundaryTileArray = Elem.Value;
		CreateBoundarySpline(BoundaryTileArray);
	}
	
}
void UKMWorldMapEditorSubsystem::ShowSelectedSector(int32 SectorId)
{
	ClearMapBorder();
	if (GetCurrentMapInfo().IsValid() == false)
	{
		return;
	}
	if(SectorId == -1)
	{
		return;
	}
	TArray<FIntPoint> BoundaryTiles = GetBoundaryTiles(SectorId);
	CreateBoundarySpline(BoundaryTiles);
}
TArray<FIntPoint> UKMWorldMapEditorSubsystem::GetBoundaryTiles(int32 InSectorId)
{
	TArray<FIntPoint> BoundaryTiles;
	if (GetCurrentMapInfo().IsValid() == false)
	{
		return BoundaryTiles;
	}

	FIntPoint Offsets[4] =
	{
		FIntPoint(-1, 0),
		FIntPoint(1, 0),
		FIntPoint(0, -1),
		FIntPoint(0, 1),

	};

	for (const auto& Tile : CurrentMapInfo->TileInfoMap)
	{
		FIntPoint TileIndex = Tile.Key;
		if (Tile.Value.SectorId != InSectorId)
		{
			continue;
		}
		int32 SectorId = Tile.Value.SectorId;
		TArray<FIntPoint> DiagonalTiles;
		DiagonalTiles.Add(FIntPoint(TileIndex.X - 1, TileIndex.Y - 1)); // Top-left
		DiagonalTiles.Add(FIntPoint(TileIndex.X + 1, TileIndex.Y - 1)); // Top-right
		DiagonalTiles.Add(FIntPoint(TileIndex.X - 1, TileIndex.Y + 1)); // Bottom-left
		DiagonalTiles.Add(FIntPoint(TileIndex.X + 1, TileIndex.Y + 1)); // Bottom-right
		TArray<FIntPoint> DifferentColorTileIndexArray;
		int32 DifferentColorCount = 0;
		for (const FIntPoint& Offset : Offsets)
		{
			FIntPoint NeighborPos = TileIndex + Offset;

			FTileInfo* Neighbor = CurrentMapInfo->TileInfoMap.Find(NeighborPos);

			if (Neighbor == nullptr || Neighbor->SectorId != SectorId)
			{
				BoundaryTiles.Add(TileIndex);
			}
		}
	}
	LinkPoints(BoundaryTiles);
	return BoundaryTiles;
}
TMap<int32, TArray<FIntPoint>> UKMWorldMapEditorSubsystem::GetBoundaryTiles()
{
	if (GetCurrentMapInfo().IsValid() == false)
	{
		return TMap<int32, TArray<FIntPoint>>();
	}

	TMap<int32, TArray<FIntPoint>> BoundaryTiles;
	FIntPoint Offsets[4] =
	{
		FIntPoint(-1, 0),
		FIntPoint(1, 0),
		FIntPoint(0, -1),
		FIntPoint(0, 1),

	};

	for (const auto& Tile : CurrentMapInfo->TileInfoMap)
	{
		FIntPoint TileIndex = Tile.Key;
		int32 SectorId = Tile.Value.SectorId;
		TArray<FIntPoint> DiagonalTiles;
		DiagonalTiles.Add(FIntPoint(TileIndex.X - 1, TileIndex.Y - 1)); // Top-left
		DiagonalTiles.Add(FIntPoint(TileIndex.X + 1, TileIndex.Y - 1)); // Top-right
		DiagonalTiles.Add(FIntPoint(TileIndex.X - 1, TileIndex.Y + 1)); // Bottom-left
		DiagonalTiles.Add(FIntPoint(TileIndex.X + 1, TileIndex.Y + 1)); // Bottom-right
		TArray<FIntPoint> DifferentColorTileIndexArray;
		int32 DifferentColorCount = 0;
		for (const FIntPoint& Offset : Offsets)
		{
			FIntPoint NeighborPos = TileIndex + Offset;
			//이 이웃 타일이 다른색이고 주변에 내 컬러로 되어있으면 경계선이다

			FTileInfo* Neighbor = CurrentMapInfo->TileInfoMap.Find(NeighborPos);

			// If the neighboring tile is of a different color or doesn't exist, this tile is a boundary tile
			if (Neighbor == nullptr || Neighbor->SectorId != SectorId)
			{
				BoundaryTiles.FindOrAdd(SectorId).Add(TileIndex);
			}
		}
	}
	for (auto& Element : BoundaryTiles)
	{
		LinkPoints(Element.Value);
	}


	return BoundaryTiles;
}

void UKMWorldMapEditorSubsystem::LinkPoints(TArray<FIntPoint>& InPoints)
{
	TArray<FIntPoint> SortedPoints;

	FIntPoint Offsets[8] =
	{
		FIntPoint(1, 0),
		FIntPoint(0, 1),
		FIntPoint(0, -1),
		FIntPoint(-1, 0),
		FIntPoint(1, 1),
		FIntPoint(-1, 1),
		FIntPoint(1, -1),
		FIntPoint(-1, -1),
	};
	SortedPoints.Add(InPoints[0]);
	for (int PointIndex = 0; PointIndex < InPoints.Num(); ++PointIndex)
	{
		for (int i = 0; i < 8; ++i)
		{
			FIntPoint Target = Offsets[i] + SortedPoints.Last();
			auto FindTarget = InPoints.Find(Target);
			if (FindTarget != INDEX_NONE && SortedPoints.Contains(InPoints[FindTarget]) == false)
			{
				SortedPoints.Add(InPoints[FindTarget]);
				break;
			}
		}
	}
	//TODO : 추후 디렉션이 같은건 지워주자

	Swap(InPoints, SortedPoints);
	return;
}
void UKMWorldMapEditorSubsystem::CreateBoundarySpline(const TArray<FIntPoint>& InBoundaryTiles)
{
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.ObjectFlags = RF_Transient;
	UWorld* World = GetEditorWorld();
	UBlueprint* LoadObject = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), NULL, TEXT("/Game/kds218/Blueprints/BP_MapBorder.BP_MapBorder")));
	AMapBorderActor* MapBorderActor = nullptr;
	if (LoadObject != nullptr)
	{
		MapBorderActor = World->SpawnActor<AMapBorderActor>(LoadObject->GeneratedClass, SpawnParams);
	}
	if (MapBorderActor != nullptr)
	{
		MapBorderActor->CreateBoundarySpline(InBoundaryTiles);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AMapBorderActor is not found"));
	}
}

const FVector UKMWorldMapEditorSubsystem::TileIndexToWorldPosition(const FIntPoint TileIndex)
{
	int32 WorldHalfSize = WorldSize / 2;
	float HalfTileSize = AKMWorldGameMode::TileSize / 2;
	FVector StartLocation = FVector(-WorldHalfSize + HalfTileSize, -WorldHalfSize + HalfTileSize, 0);
	return FVector(StartLocation.X + TileIndex.X * AKMWorldGameMode::TileSize, StartLocation.Y + TileIndex.Y * AKMWorldGameMode::TileSize, 0);
}
const FIntPoint UKMWorldMapEditorSubsystem::GetTileIndex(const FVector& HitLocation)
{
	int32 WorldHalfSize = WorldSize / 2.f;
	FVector StartLocation = FVector(-WorldHalfSize, -WorldHalfSize, 0);

	int32 TileX = (HitLocation.X + WorldHalfSize) / AKMWorldGameMode::TileSize;
	int32 TileY = (HitLocation.Y + WorldHalfSize) / AKMWorldGameMode::TileSize;
	return FIntPoint(TileX, TileY);
}


void UKMWorldMapEditorSubsystem::OnLevelLoaded(const FString& MapName, bool bAsTemplate)
{
	UWorld* World = GetEditorWorld();
	ALandscape* Landscape_ = nullptr;
	for (TActorIterator<ALandscape> ActorItr(World); ActorItr; ++ActorItr)
	{
		Landscape_ = *ActorItr;
		break;
	}
	if (Landscape_ == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Landscape is not found"));
		return;
	}
	FVector LandsLocation_;
	LandsLocation_ = Landscape_->GetActorTransform().GetLocation();
	WorldSize = FMath::Abs(LandsLocation_.X * 2);
	UE_LOG(LogTemp, Warning, TEXT("OnLevelLoaded WorldSize %d"), WorldSize);
}

void UKMWorldMapEditorSubsystem::OnLevelsChanged()
{
	//OnLevelLoaded(GEditor->GetEditorWorldContext().World());
}
void UKMWorldMapEditorSubsystem::RefreshSectorId()
{
	//CurrentMapInfo 를 TempMapInfo에 복사한다
	
	
	//UpdateSectorId를 백그라운드 쓰레드에서 실행한다
	//결과를 메인쓰레드에서 CurrentMapInfo에 복사한다

	//FMapInfo TempMapInfo = *CurrentMapInfo;

	 CurrentMapInfo->TileInfoMap;
	for (auto& Elem : CurrentMapInfo->TileInfoMap)
	{
		Elem.Value.SectorId = -1;
	}
	CurrentMapInfo->UpdateSecotorId();

}
float UKMWorldMapEditorSubsystem::GetLandscapeHeight(const FVector& InWorldPosition)
{
	UWorld* World_ = GetEditorWorld();
	ALandscape* Landscape_ = nullptr;
	for (TActorIterator<ALandscape> ActorItr(World_); ActorItr; ++ActorItr)
	{
		Landscape_ = *ActorItr;
		break;
	}
	if (Landscape_ == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Landscape is not found"));
		return 0.0f;
	}
	TOptional<float> Height = Landscape_->GetHeightAtLocation(InWorldPosition);
	if (Height.IsSet() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Height is not set"));
		return 0.0f;
	}
	return Height.GetValue();
}
void UKMWorldMapEditorSubsystem::UpdateTileColor(TArray<FIntPoint> TileIndexArray, FColor InBrushColor, bool bIsRemove)
{

	if (CurrentMapInfo.IsValid() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrentMapInfo is nullptr"));
		return;
	}

	TArray<FIntPoint> NeighborTileIndexArray;
	for (int32 i = 0; i < TileIndexArray.Num(); ++i)
	{
		FIntPoint Directions[4] = { FIntPoint(1, 0), FIntPoint(-1, 0), FIntPoint(0, 1), FIntPoint(0, -1) };

		for (const FIntPoint& Dir : Directions)
		{
			FIntPoint NextDir = TileIndexArray[i] + Dir;
			FTileInfo* NextTileInfo = CurrentMapInfo->TileInfoMap.Find(NextDir);
			if (NextTileInfo == nullptr)
			{
				continue;
			}
			if (NextTileInfo->SectorColor != InBrushColor || bIsRemove)
			{
				NeighborTileIndexArray.Add(NextDir);
				continue;
			}

		}
	}

	for (int32 i = 0; i < TileIndexArray.Num(); ++i)
	{
		if (bIsRemove)
		{
			CurrentMapInfo->TileInfoMap.Remove(TileIndexArray[i]);
			continue;
		}
		auto& Info = CurrentMapInfo->TileInfoMap.FindOrAdd(TileIndexArray[i]);
		Info.SectorColor = InBrushColor;
		Info.SectorId = -1;
	}
	for (int32 i = 0; i < NeighborTileIndexArray.Num(); ++i)
	{
		FTileInfo* FindTileInfo = CurrentMapInfo->TileInfoMap.Find(NeighborTileIndexArray[i]);
		if(FindTileInfo == nullptr)
		{
			continue;
		}
		FindTileInfo->SectorId = -1;
	}
	//CurrentMapInfo->UpdateSecotorId();
}

void UKMWorldMapEditorSubsystem::PrintTileInfo(const FVector& InWorldPosition)
{
	FIntPoint TileIndex_ = GetTileIndex(InWorldPosition);
	UE_LOG(LogTemp, Log, TEXT("[UKMWorldMapEditorSubsystem::PrintTileInfo] WorldPosition : %s, TileIndex : %s"), *InWorldPosition.ToString(), *TileIndex_.ToString());
	auto FindTileInfo_ = CurrentMapInfo->TileInfoMap.Find(TileIndex_);
	if (FindTileInfo_ == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UKMWorldMapEditorSubsystem::PrintTileInfo] TileInfo is nullptr"));
		return;
	}
	OnTileSelected.Broadcast(TileIndex_);
	ShowSelectedSector(FindTileInfo_->SectorId);
	UE_LOG(LogTemp, Log, TEXT("[PrintTileInfo] Index : (%d, %d), Color : %s, SectorId : %d"), TileIndex_.X, TileIndex_.Y, *FindTileInfo_->SectorColor.ToString(), FindTileInfo_->SectorId);
}

FVector UKMWorldMapEditorSubsystem::GetMousePositionToWorldPosition(FEditorViewportClient* ViewportClient, FViewport* Viewport, const FVector2D& MousePosition)
{
	FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
		Viewport,
		ViewportClient->GetScene(),
		ViewportClient->EngineShowFlags)
		.SetRealtimeUpdate(ViewportClient->IsRealtime()));
	FSceneView* SceneView = ViewportClient->CalcSceneView(&ViewFamily);
	//LineTrace를 통해 마우스 위치에 대한 정보를 얻어온다.
	FVector WorldOrigin;
	FVector WorldDirection;
	SceneView->DeprojectFVector2D(MousePosition, WorldOrigin, WorldDirection);
	FHitResult HitResult;
	FCollisionQueryParams CollisionQueryParams;
	if (GetEditorWorld()->LineTraceSingleByChannel(HitResult, WorldOrigin, WorldOrigin + WorldDirection * 9999999, ECollisionChannel::ECC_Visibility, CollisionQueryParams))
	{
		return HitResult.Location;
	}
	return WorldOrigin;
}


UWorld* UKMWorldMapEditorSubsystem::GetEditorWorld()
{
	return GEditor->GetEditorWorldContext().World();
}

void UKMWorldMapEditorSubsystem::ClearMapBorder()
{
	UWorld* World = GetEditorWorld();
	if (World == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("World is nullptr"));
		return;
	}
	for (TActorIterator<AMapBorderActor> ActorItr(World); ActorItr; ++ActorItr)
	{
		ActorItr->Destroy();
	}
}
