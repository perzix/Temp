// Fill out your copyright notice in the Description page of Project Settings.


#include "Tile/WorldMapTileActor.h"
#include "Components/DecalComponent.h"
#include "WorldMode/KMWorldGameMode.h"
#include "Landscape.h"
#include "EngineUtils.h"
#include "MapInfo.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "MapBorderActor.h"
#include "Subsystem/KMWorldMapEditorSubsystem.h"
// Sets default values
AWorldMapTileActor::AWorldMapTileActor()
{
	TextureScale = 0.02f;
	UndoDatas.Empty(MaxUndoCount);
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	DefaultSceneRoot->SetCanEverAffectNavigation(false);
	RootComponent = DefaultSceneRoot;

	GridDecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("GridDecal"));
	GridDecalComponent->AttachToComponent(DefaultSceneRoot, FAttachmentTransformRules::KeepRelativeTransform);

	WorldTileDecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("WorldTileDecal"));
	WorldTileDecalComponent->AttachToComponent(DefaultSceneRoot, FAttachmentTransformRules::KeepRelativeTransform);

	BrushDecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("BrushDecal"));
	BrushDecalComponent->AttachToComponent(DefaultSceneRoot, FAttachmentTransformRules::KeepRelativeTransform);
	
	BrushDecalComponent->SetSortOrder(999);
}

AWorldMapTileActor::~AWorldMapTileActor()
{
	UKMWorldMapEditorSubsystem* WorldMapEditorSubsystem = UKMWorldMapEditorSubsystem::Get();
	if (WorldMapEditorSubsystem != nullptr)
	{
		WorldMapEditorSubsystem->GetOnBrushSizeChanged().RemoveAll(this);
		WorldMapEditorSubsystem->GetOnBrushColorChanged().RemoveAll(this);
		WorldMapEditorSubsystem->GetOnMapInfoChanged().RemoveAll(this);
		WorldMapEditorSubsystem->GetOnControlTypeChanged().RemoveAll(this);
		WorldMapEditorSubsystem->GetOnSectorAlphaChanged().RemoveAll(this);
	}
}

void AWorldMapTileActor::Init()
{
	if (GridDecalMaterial == nullptr || GridDecalComponent == nullptr)
	{
		return;
	}
	UKMWorldMapEditorSubsystem* WorldMapEditorSubsystem = UKMWorldMapEditorSubsystem::Get();
	if (WorldMapEditorSubsystem != nullptr)
	{
		WorldMapEditorSubsystem->GetOnBrushSizeChanged().RemoveAll(this);
		WorldMapEditorSubsystem->GetOnBrushColorChanged().RemoveAll(this);
		WorldMapEditorSubsystem->GetOnMapInfoChanged().RemoveAll(this);
		WorldMapEditorSubsystem->GetOnControlTypeChanged().RemoveAll(this);
		WorldMapEditorSubsystem->GetOnSectorAlphaChanged().RemoveAll(this);
		WorldMapEditorSubsystem->GetOnBrushSizeChanged().AddUObject(this, &AWorldMapTileActor::OnBrushSizeChanged);
		WorldMapEditorSubsystem->GetOnBrushColorChanged().AddUObject(this, &AWorldMapTileActor::OnBrushColorChanged);
		WorldMapEditorSubsystem->GetOnMapInfoChanged().AddUObject(this, &AWorldMapTileActor::OnMapInfoChanged);
		WorldMapEditorSubsystem->GetOnControlTypeChanged().AddUObject(this, &AWorldMapTileActor::OnControlTypeChanged);
		WorldMapEditorSubsystem->GetOnSectorAlphaChanged().AddUObject(this, &AWorldMapTileActor::OnSectorAlphaChanged);
	}

	
	GridDecalComponent->SetRelativeRotation(FRotator(-90, 0, 0));
	WorldTileDecalComponent->SetRelativeRotation(FRotator(-90, 0, 0));
	BrushDecalComponent->SetRelativeRotation(FRotator(-90, 0, 0));
	int32 WorldSize = WorldMapEditorSubsystem->GetWorldSize();
	float WorldHalfSize = WorldSize / 2;
	GridDecalComponent->DecalSize = FVector(50000, WorldHalfSize, WorldHalfSize);
	WorldTileDecalComponent->DecalSize = FVector(50000, WorldHalfSize, WorldHalfSize);
	if (DynamicGridMaterial == nullptr || DynamicGridMaterial.IsValid() == false)
	{
		DynamicGridMaterial = UMaterialInstanceDynamic::Create(GridDecalMaterial, this);
	}
	DynamicGridMaterial->SetScalarParameterValue(FName("GridSize"), AKMWorldGameMode::TileSize);
	GridDecalComponent->SetDecalMaterial(DynamicGridMaterial.Get());
	
	//BrushSize = 1;
	BrushDecalComponent->DecalSize = FVector(50000, AKMWorldGameMode::TileSize / 2, AKMWorldGameMode::TileSize / 2);
	InitWorldTileTexture();
	OnBrushColorChanged();
	OnBrushSizeChanged();
}

void AWorldMapTileActor::InitWorldTileTexture()
{
	UKMWorldMapEditorSubsystem* WorldMapEditorSubsystem = UKMWorldMapEditorSubsystem::Get();
	if (WorldMapEditorSubsystem == nullptr)
	{
		return;
	}
	int32 TileSize = AKMWorldGameMode::TileSize;
	int32 WorldSize = WorldMapEditorSubsystem->GetWorldSize();
	float WorldHalfSize = WorldSize / 2;
	int32 TextureSize = WorldSize * TextureScale;//10080
	if (DynamicWorldTileMaterial == nullptr || DynamicWorldTileMaterial.IsValid() == false)
	{
		DynamicWorldTileMaterial = UMaterialInstanceDynamic::Create(WorldTileDecalMaterial, this);
	}
	// 새로운 UTexture2D 객체를 생성합니다
	if (MapTileTexture == nullptr)
	{
		MapTileTexture = UTexture2D::CreateTransient(TextureSize, TextureSize, PF_B8G8R8A8);
		MapTileTexture->Modify();
		uint8* MipData = static_cast<uint8*>(MapTileTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
		for (int32 y = 0; y < TextureSize; ++y)
		{
			for (int32 x = 0; x < TextureSize; ++x)
			{
				int32 Index = x + y * TextureSize;
				FColor Color = FColor::Transparent;
				FMemory::Memcpy(MipData + Index * sizeof(FColor), &Color, sizeof(FColor));
			}
		}
		MapTileTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
	}
	MapTileTexture->UpdateResource();
	DynamicWorldTileMaterial->SetTextureParameterValue(FName("Texture"), MapTileTexture);
	DynamicWorldTileMaterial->SetScalarParameterValue(FName("Alpha"), WorldMapEditorSubsystem->GetSectorAlpha());
	WorldTileDecalComponent->SetDecalMaterial(DynamicWorldTileMaterial.Get());
}

void AWorldMapTileActor::WriteGridTexture()
{
	FlushPersistentDebugLines(GetWorld());
	if (GridDecalMaterial == nullptr || GridDecalComponent == nullptr)
	{
		return;
	}
	UKMWorldMapEditorSubsystem* WorldMapEditorSubsystem = UKMWorldMapEditorSubsystem::Get();
	if (WorldMapEditorSubsystem == nullptr)
	{
		return;
	}
	int32 TileSize = AKMWorldGameMode::TileSize;
	int32 WorldSize = WorldMapEditorSubsystem->GetWorldSize();
	float WorldHalfSize = WorldSize / 2;
	int32 TextureSize = WorldHalfSize * TextureScale;//10080
	// 새로운 UTexture2D 객체를 생성합니다
	if (MapTileTexture == nullptr)
	{
		MapTileTexture = UTexture2D::CreateTransient(TextureSize, TextureSize, PF_B8G8R8A8);
	}
	// 텍스처를 수정하기 전에 Modify()를 호출해야 합니다.
	MapTileTexture->Modify();

	// 텍스처 데이터에 접근합니다.
	uint8* MipData = static_cast<uint8*>(MapTileTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
	int32 TileCount = WorldSize / TileSize;     //TextureSize * TextureScale;
	int32 TileTextureSize = TextureSize / TileCount;//10
	//int32 TileSize = TileTextureSize / TextureScale * 2;//TileSize는 200
	FVector TileStartLocation = FVector(-WorldHalfSize, -WorldHalfSize, 0);
	FCollisionQueryParams QueryParams;
	QueryParams.bFindInitialOverlaps = true;
	QueryParams.AddIgnoredActor(this); // 이 액터를 무시합니다.
	UE_LOG(LogTemp, Log, TEXT("TileStartLocation : %s"), *TileStartLocation.ToString());
	TArray<FVector> TileCorners;
	TileCorners.SetNum(4);
	for (int32 TileY = 0; TileY < TileCount; ++TileY)
	{
		for (int32 TileX = 0; TileX < TileCount; ++TileX)
		{
			FColor Color = FColor::Transparent;
			FVector TileLocation = FVector(TileStartLocation.X + TileX * TileSize, TileStartLocation.Y + TileY * TileSize, 0);
			FVector TileCenterLocation = TileLocation + FVector(TileSize / 2, TileSize / 2, 0);

			bool bIsHit = GetWorld()->OverlapAnyTestByChannel(
				TileCenterLocation,
				FQuat::Identity,
				ECollisionChannel::ECC_GameTraceChannel1, FCollisionShape::MakeBox(FVector(TileSize / 2, TileSize / 2, 500000)));


			if (bIsHit)
			{
				//실제 지형과 겹치는 부분만 다시한번 체크한다.
				//타일의 4점 위치를 계산합니다
				TileCorners.Empty(4);
				TileCorners.Add(TileLocation);//bottom left
				TileCorners.Add(TileLocation + FVector(TileSize, 0, 0));//bottom right
				TileCorners.Add(TileLocation + FVector(TileSize, TileSize, 0));//top right
				TileCorners.Add(TileLocation + FVector(0, TileSize, 0));//top left


				float ZTestMin = 99999999.f;
				float ZTestMax = -99999999.f;
				for (int i = 0; i < TileCorners.Num(); ++i)
				{
					FVector TileCorner = TileCorners[i];
					FHitResult HitResult;
					if (GetWorld()->LineTraceSingleByChannel(
						HitResult,
						TileCorners[i] + FVector(0, 0, 99999),
						TileCorners[i] + FVector(0, 0, -99999),
						ECollisionChannel::ECC_WorldStatic, QueryParams))//ECC_WorldStatic 은 테스트를 위해 임시로 사용한것임 (추후 변경 필요)
					{
						ZTestMin = FMath::Min(ZTestMin, HitResult.ImpactPoint.Z);
						ZTestMax = FMath::Max(ZTestMax, HitResult.ImpactPoint.Z);
						TileCorner.Z = HitResult.ImpactPoint.Z;
						TileCorners[i] = TileCorner;
					}
					else
					{
						UE_LOG(LogTemp, Log, TEXT("LineTraceSingleByChannel Fail. Location : %s"), *TileCenterLocation.ToString());
					}
				}

				float ZPosition = (ZTestMin + ZTestMax) / 2;
				float ZLength = ZTestMax - ZTestMin;
				FVector TestColliderPosition = TileCenterLocation + FVector(0, 0, ZPosition);

				//4점 위치에 충돌하는 오브젝트가 있는지 확인합니다
				bIsHit = GetWorld()->OverlapAnyTestByChannel(
					TestColliderPosition,
					FQuat::Identity,
					ECollisionChannel::ECC_GameTraceChannel1,
					FCollisionShape::MakeBox(FVector(TileSize / 2, TileSize / 2, FMath::Max(ZLength / 2, 50))),
					QueryParams);


				if (bIsHit == false)
				{
					//if (bShowDebugGrid)
					{
						DrawDebugBox(GetWorld(), TestColliderPosition, FVector(TileSize / 2, TileSize / 2, FMath::Max(ZLength / 2, 50)), FColor::Green, false, 999999.f);
					}
					UE_LOG(LogTemp, Log, TEXT("Location : %s, ZTestMin : %f, ZTestMax : %f, ZTest : %f, ZTestLength : %f"), *TileCenterLocation.ToString(), ZTestMin, ZTestMax, ZPosition, ZLength);
				}

			}
			if (bIsHit)
			{
				Color = FColor::Red;
			}
			//타일 당 픽셀1 로 치환하면 좋을거 같은데 쉐이더에서 어떻게 해야 할지 모르겠다
			for (int PixelY = 0; PixelY < TileTextureSize; ++PixelY)
			{
				for (int PixelX = 0; PixelX < TileTextureSize; ++PixelX)
				{
					int32 Index = ((TileX * TileTextureSize + PixelX) + (TileY * TileTextureSize + PixelY) * TextureSize);
					FMemory::Memcpy(MipData + Index * sizeof(FColor), &Color, sizeof(FColor));
				}
			}
		}
	}

	// 텍스처 데이터의 잠금을 해제합니다.
	MapTileTexture->GetPlatformData()->Mips[0].BulkData.Unlock();

	// 텍스처를 업데이트합니다.
	MapTileTexture->UpdateResource();
	DynamicWorldTileMaterial->SetTextureParameterValue(FName("Texture"), MapTileTexture);
}
void AWorldMapTileActor::MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 x, int32 y)
{
	if (Viewport == nullptr || ViewportClient == nullptr)
	{
		return;
	}
	UKMWorldMapEditorSubsystem* WorldMapEditorSubsystem = UKMWorldMapEditorSubsystem::Get();
	if (WorldMapEditorSubsystem == nullptr)
	{
		return;
	}
	int32 WorldSize = WorldMapEditorSubsystem->GetWorldSize();
	FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
		Viewport,
		ViewportClient->GetScene(),
		ViewportClient->EngineShowFlags)
		.SetRealtimeUpdate(ViewportClient->IsRealtime()));
	FSceneView* SceneView = ViewportClient->CalcSceneView(&ViewFamily);
	//LineTrace를 통해 마우스 위치에 대한 정보를 얻어온다.
	FVector WorldOrigin;
	FVector WorldDirection;
	SceneView->DeprojectFVector2D(FVector2D(x, y), WorldOrigin, WorldDirection);
	FHitResult HitResult;
	FCollisionQueryParams CollisionQueryParams;
	if (GetWorld()->LineTraceSingleByChannel(HitResult, WorldOrigin, WorldOrigin + WorldDirection * 9999999, ECollisionChannel::ECC_Visibility, CollisionQueryParams))
	{
		int32 WorldHalfSize = WorldSize / 2;
		FVector StartLocation = FVector(-WorldHalfSize, -WorldHalfSize, 0);
		FIntPoint TileIndex = GetTileIndex(WorldHalfSize, AKMWorldGameMode::TileSize, StartLocation, HitResult.Location);

		FVector TileLocation = FVector(StartLocation.X + TileIndex.X * AKMWorldGameMode::TileSize, StartLocation.Y + TileIndex.Y * AKMWorldGameMode::TileSize, 0);
		FVector TileCenterLocation = TileLocation + FVector(AKMWorldGameMode::TileSize/2, AKMWorldGameMode::TileSize/2, 0);
		
		BrushDecalComponent->SetHiddenInGame(false);
		BrushDecalComponent->SetWorldLocation(TileCenterLocation);

	}
	else
	{
		BrushDecalComponent->SetHiddenInGame(true);
	}
	if (ViewportClient != nullptr)
	{
		ViewportClient->Invalidate(false, false);
	}
}

void AWorldMapTileActor::UpdateMapTileTexture()
{
	UKMWorldMapEditorSubsystem* WorldMapEditorSubsystem = UKMWorldMapEditorSubsystem::Get();
	MapTileTexture->Modify();
	// 텍스처 데이터에 접근합니다.
	uint8* MipData = static_cast<uint8*>(MapTileTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE));

	int32 TextureSize = MapTileTexture->GetSizeX();
	int32 TileCount = WorldMapEditorSubsystem->GetWorldSize() / AKMWorldGameMode::TileSize;     //TextureSize * TextureScale;
	int32 PixelSize = TextureSize / TileCount;//10
	TArray<TTuple<FIntPoint, FColor>> TempUndoData;
	for (auto& Elem : MouseClickCacheData)
	{
		FIntPoint TileIndex = Elem.Key;
		FColor Color = Elem.Value;
		FColor PrevColor = Elem.Value;
		int32 PreColorIndex = ((TileIndex.X * PixelSize) + (TileIndex.Y * PixelSize) * TextureSize);
		FMemory::Memcpy(&PrevColor, MipData + PreColorIndex * sizeof(FColor), sizeof(FColor));
		for (int32 PixelY = 0; PixelY < PixelSize; ++PixelY)
		{
			for (int32 PixelX = 0; PixelX < PixelSize; ++PixelX)
			{
				int32 Index = ((TileIndex.X * PixelSize + PixelX) + (TileIndex.Y * PixelSize + PixelY) * TextureSize);
				FMemory::Memcpy(MipData + Index * sizeof(FColor), &Color, sizeof(FColor));
			}
		}
		TempUndoData.Add(TTuple<FIntPoint, FColor>(TileIndex, PrevColor));
	}
	UndoDatas.Add(TempUndoData);
	if (UndoDatas.Num() > MaxUndoCount)
	{
		UndoDatas.RemoveAt(0);
	}
	MouseClickCacheData.Empty();
	MapTileTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
	MapTileTexture->UpdateResource();
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
}
void AWorldMapTileActor::MouseClick(FEditorViewportClient* InViewportClient, HHitProxy* HitProxy, const FViewportClick& Click)
{
	MouseClick(false, false);
}
void AWorldMapTileActor::DragLine(bool bIsRemove)
{
	UKMWorldMapEditorSubsystem* WorldMapEditorSubsystem = UKMWorldMapEditorSubsystem::Get();

	//일단 BrushDecalComponent의 위치에 그리는걸로 하자
	if (BrushDecalComponent == nullptr ||
		MapTileTexture == nullptr ||
		BrushDecalComponent->bHiddenInGame ||
		WorldMapEditorSubsystem == nullptr ||
		WorldMapEditorSubsystem->GetCurrentMapInfo().IsValid() == false)
	{
		return;
	}
	int32 WorldSize = WorldMapEditorSubsystem->GetWorldSize();
	int32 WorldHalfSize = WorldSize / 2;
	FVector StartLocation = FVector(-WorldHalfSize, -WorldHalfSize, 0);
	FVector BrushLocation = BrushDecalComponent->GetComponentLocation();
	FIntPoint TileIndex = GetTileIndex(WorldHalfSize, AKMWorldGameMode::TileSize, StartLocation, BrushLocation);
	if (TileIndex == DragLastTileIndex)
	{
		return;
	}
	if(DragLineDirection == FVector2D::ZeroVector)
	{
		DragLineDirection = FVector2D(TileIndex - DragLastTileIndex).GetSafeNormal();
		float XLength = FMath::Abs(DragLineDirection.X);
		float YLength = FMath::Abs(DragLineDirection.Y);
		//Direction 을 4방향으로 보정한다
		//만약 대각선이면 다음 DragLine이 호출될때 방향을 정한다
		
		if (FMath::IsNearlyEqual(XLength, YLength, 0.01f))
		{
			DragLineDirection = FVector2D::ZeroVector;
			return;
		}
		else
		{
			if (XLength > YLength)
			{
				DragLineDirection.Y = 0;
			}
			else
			{
				DragLineDirection.X = 0;
			}
		}
	}
	
	//타일 인덱스를 Direction 방향으로 이동시킨다
	TileIndex = FIntPoint(DragLineDirection.X != 0 ? TileIndex.X : DragLastTileIndex.X, DragLineDirection.Y != 0 ? TileIndex.Y : DragLastTileIndex.Y);
	TArray<FIntPoint> TileArray = GetTileIndexArray(TileIndex);
	FColor BrushColor = WorldMapEditorSubsystem->GetBrushColor().ToFColor(true);
	for (int i = 0; i < TileArray.Num(); ++i)
	{
		MouseClickCacheData.Remove(TileArray[i]);
		MouseClickCacheData.Add(TileArray[i], bIsRemove ? FColor::Transparent : WorldMapEditorSubsystem->GetBrushColor().ToFColor(true));
	}
	WorldMapEditorSubsystem->UpdateTileColor(TileArray, BrushColor, bIsRemove);
	DragLastTileIndex = TileIndex;

	if (GetWorld()->GetTimerManager().IsTimerActive(TimerHandle))
	{
		UE_LOG(LogTemp, Log, TEXT("Timer is already active"));
		return;
	}
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AWorldMapTileActor::UpdateMapTileTexture, 0.1f, false);
}

void AWorldMapTileActor::MouseClick(bool bIsDrag, bool bIsEraser)
{
	DragLineDirection = FVector2D::ZeroVector;
	UKMWorldMapEditorSubsystem* WorldMapEditorSubsystem = UKMWorldMapEditorSubsystem::Get();
	
	//일단 BrushDecalComponent의 위치에 그리는걸로 하자
	if (BrushDecalComponent == nullptr || 
		MapTileTexture == nullptr || 
		BrushDecalComponent->bHiddenInGame || 
		WorldMapEditorSubsystem == nullptr || 
		WorldMapEditorSubsystem->GetCurrentMapInfo().IsValid() == false)
	{
		return;
	}
	int32 WorldSize = WorldMapEditorSubsystem->GetWorldSize();
	int32 WorldHalfSize = WorldSize / 2;
	FVector StartLocation = FVector(-WorldHalfSize, -WorldHalfSize, 0);
	FVector BrushLocation = BrushDecalComponent->GetComponentLocation();
	FIntPoint TileIndex = GetTileIndex(WorldHalfSize, AKMWorldGameMode::TileSize, StartLocation, BrushLocation);
	if (bIsDrag && TileIndex == DragLastTileIndex)
	{
		return;
	}
	
	DragLastTileIndex = TileIndex;
	TArray<FIntPoint> TileArray = GetTileIndexArray(WorldHalfSize, AKMWorldGameMode::TileSize, StartLocation, BrushLocation);
	FColor BrushColor = WorldMapEditorSubsystem->GetBrushColor().ToFColor(true);
	for (int i = 0; i < TileArray.Num(); ++i)
	{
		MouseClickCacheData.Remove(TileArray[i]);
		MouseClickCacheData.Add(TileArray[i], bIsEraser ? FColor::Transparent : WorldMapEditorSubsystem->GetBrushColor().ToFColor(true));
	}
	WorldMapEditorSubsystem->UpdateTileColor(TileArray, BrushColor, bIsEraser);
	

	if (GetWorld()->GetTimerManager().IsTimerActive(TimerHandle))
	{
		return;
	}
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AWorldMapTileActor::UpdateMapTileTexture, 0.1f, false);
}


const FIntPoint AWorldMapTileActor::GetTileIndex(int32 WorldHalfSize, int32 TileSize, FVector StartLocation, const FVector& HitLocation)
{
	int32 TileX = (HitLocation.X + WorldHalfSize) / TileSize;
	int32 TileY = (HitLocation.Y + WorldHalfSize) / TileSize;
	return FIntPoint(TileX, TileY);
}
const TArray<FIntPoint> AWorldMapTileActor::GetTileIndexArray(int32 WorldHalfSize, int32 TileSize, FVector StartLocation, const FVector& HitLocation)
{
	TArray<FIntPoint> TileIndexArray;
	UKMWorldMapEditorSubsystem* WorldMapEditorSubsystem = UKMWorldMapEditorSubsystem::Get();
	if (WorldMapEditorSubsystem == nullptr)
	{
		return TileIndexArray;
	}
	int32 TileX = (HitLocation.X + WorldHalfSize) / TileSize;
	int32 TileY = (HitLocation.Y + WorldHalfSize) / TileSize;
	int32 BrushSize = WorldMapEditorSubsystem->GetBrushSize();
	if (BrushSize == 1)
	{
		TileIndexArray.Add(FIntPoint(TileX, TileY));
		return TileIndexArray;
	}
	int32 WorldSize = WorldMapEditorSubsystem->GetWorldSize();

	int32 BrushHalfSize = BrushSize / 2;
	int32 TileCount = WorldSize / TileSize;

	for (int32 i = -BrushHalfSize; i <= BrushHalfSize; ++i)
	{
		for (int32 j = -BrushHalfSize; j <= BrushHalfSize; ++j)
		{
			int32 NewTileX = TileX + i;
			int32 NewTileY = TileY + j;
			if (NewTileX < 0 || NewTileY < 0)
			{
				continue;
			}
			if (NewTileX >= TileCount || NewTileY >= TileCount)
			{
				continue;
			}
			TileIndexArray.Add(FIntPoint(NewTileX, NewTileY));
		}
	}
	return TileIndexArray;
}
const TArray<FIntPoint> AWorldMapTileActor::GetTileIndexArray(const FIntPoint& TileIndex)
{
	TArray<FIntPoint> TileIndexArray;
	UKMWorldMapEditorSubsystem* WorldMapEditorSubsystem = UKMWorldMapEditorSubsystem::Get();
	if (WorldMapEditorSubsystem == nullptr)
	{
		return TileIndexArray;
	}
	int32 BrushSize = WorldMapEditorSubsystem->GetBrushSize();
	if (BrushSize == 1)
	{
		TileIndexArray.Add(TileIndex);
		return TileIndexArray;
	}
	int32 WorldSize = WorldMapEditorSubsystem->GetWorldSize();

	int32 BrushHalfSize = BrushSize / 2;
	int32 TileCount = WorldSize / AKMWorldGameMode::TileSize;

	for (int32 i = -BrushHalfSize; i <= BrushHalfSize; ++i)
	{
		for (int32 j = -BrushHalfSize; j <= BrushHalfSize; ++j)
		{
			int32 NewTileX = TileIndex.X + i;
			int32 NewTileY = TileIndex.Y + j;
			if (NewTileX < 0 || NewTileY < 0)
			{
				continue;
			}
			if (NewTileX >= TileCount || NewTileY >= TileCount)
			{
				continue;
			}
			TileIndexArray.Add(FIntPoint(NewTileX, NewTileY));
		}
	}
	return TileIndexArray;
}
void AWorldMapTileActor::ProcessUndo()
{
	UKMWorldMapEditorSubsystem* WorldMapEditorSubsystem = UKMWorldMapEditorSubsystem::Get();
	if (MapTileTexture == nullptr || 
		WorldMapEditorSubsystem == nullptr ||
		UndoDatas.Num() == 0 ||
		WorldMapEditorSubsystem->GetCurrentMapInfo().IsValid() == false)
	{
		return;
	}
	int32 WorldSize = WorldMapEditorSubsystem->GetWorldSize();
	MapTileTexture->Modify();
	// 텍스처 데이터에 접근합니다.
	uint8* MipData = static_cast<uint8*>(MapTileTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
	int32 TextureSize = MapTileTexture->GetSizeX();
	TArray<TTuple<FIntPoint, FColor>> UndoData = UndoDatas[UndoDatas.Num() - 1];
	TArray<FIntPoint> TileArray;
	TileArray.Reserve(UndoData.Num());
	Algo::Transform(UndoData, TileArray, [](const TTuple<FIntPoint, FColor>& Elem) 
	{
		return Elem.Key;
	});
	WorldMapEditorSubsystem->UpdateTileColor(TileArray, UndoData[0].Value, UndoData[0].Value == FColor::Transparent);
	for (auto& Elem : UndoData)
	{
		FIntPoint TileIndex = Elem.Key;
		FColor Color = Elem.Value;
		
		int32 PixelSize = TextureSize / (WorldSize / AKMWorldGameMode::TileSize);
		for (int32 PixelY = 0; PixelY < PixelSize; ++PixelY)
		{
			for (int32 PixelX = 0; PixelX < PixelSize; ++PixelX)
			{
				int32 Index = ((TileIndex.X * PixelSize + PixelX) + (TileIndex.Y * PixelSize + PixelY) * TextureSize);
				FMemory::Memcpy(MipData + Index * sizeof(FColor), &Color, sizeof(FColor));
			}
		}
	}
	UndoDatas.RemoveAt(UndoDatas.Num() - 1);
	MapTileTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
	MapTileTexture->UpdateResource();
}

int32 AWorldMapTileActor::GetWorldSizeFromLandscape()
{
	ALandscape* Landscape_ = nullptr;
	for (TActorIterator<ALandscape> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		Landscape_ = *ActorItr;
		break;
	}
	if (Landscape_ == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Landscape is not found"));
		return 0;
	}
	FVector LandsLocation_;
	LandsLocation_ = Landscape_->GetActorTransform().GetLocation();
	return FMath::Abs(LandsLocation_.X * 2);
}

void AWorldMapTileActor::OnBrushSizeChanged()
{
	UKMWorldMapEditorSubsystem* WorldMapEditorSubsystem = UKMWorldMapEditorSubsystem::Get();
	if (WorldMapEditorSubsystem == nullptr)
	{
		return;
	}
	if (BrushDecalComponent == nullptr)
	{
		return;
	}
	int32 DecalSize = AKMWorldGameMode::TileSize / 2 * WorldMapEditorSubsystem->GetBrushSize();
	BrushDecalComponent->DecalSize = FVector(50000, DecalSize, DecalSize);
}
void AWorldMapTileActor::OnBrushColorChanged()
{
	UKMWorldMapEditorSubsystem* WorldMapEditorSubsystem = UKMWorldMapEditorSubsystem::Get();
	if (WorldMapEditorSubsystem == nullptr)
	{
		return;
	}
	if (BrushDecalComponent == nullptr)
	{
		return;
	}
	if (DynamicBrushMaterial.IsValid() == false)
	{
		DynamicBrushMaterial = UMaterialInstanceDynamic::Create(BrushDecalMaterial, this);
	}
	DynamicBrushMaterial->SetVectorParameterValue(FName("Color"), WorldMapEditorSubsystem->GetBrushColor());
	BrushDecalComponent->SetDecalMaterial(DynamicBrushMaterial.Get());
}
void AWorldMapTileActor::OnMapInfoChanged()
{
	UKMWorldMapEditorSubsystem* WorldMapEditorSubsystem = UKMWorldMapEditorSubsystem::Get();
	if (WorldMapEditorSubsystem == nullptr)
	{
		return;
	}
	
	UndoDatas.Empty(MaxUndoCount);
	if (WorldMapEditorSubsystem->GetCurrentMapInfo().IsValid() == false)
	{
		//초기화 시켜주고 끝냄
		InitWorldTileTexture();
		return;
	}
	int32 WorldSize = WorldMapEditorSubsystem->GetWorldSize();
	MapTileTexture->Modify();
	// 텍스처 데이터에 접근합니다.
	uint8* MipData = static_cast<uint8*>(MapTileTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
	int32 TextureSize = MapTileTexture->GetSizeX();
	//텍스쳐를 초기화 한뒤에 다시 그려야한다
	int32 TileCount = WorldSize / AKMWorldGameMode::TileSize;     //TextureSize * TextureScale;
	for (int32 TileY = 0; TileY < TileCount; ++TileY)
	{
		for (int32 TileX = 0; TileX < TileCount; ++TileX)
		{
			FIntPoint TileIndex = FIntPoint(TileX, TileY);
			FColor TileColor = FColor::Transparent;
			auto FindTile = WorldMapEditorSubsystem->GetCurrentMapInfo()->TileInfoMap.Find(TileIndex);
			if (FindTile != nullptr)
			{
				TileColor = FindTile->SectorColor;
			}
			int32 PixelSize = TextureSize / (WorldSize / AKMWorldGameMode::TileSize);
			for (int32 PixelY = 0; PixelY < PixelSize; ++PixelY)
			{
				for (int32 PixelX = 0; PixelX < PixelSize; ++PixelX)
				{
					int32 Index = ((TileIndex.X * PixelSize + PixelX) + (TileIndex.Y * PixelSize + PixelY) * TextureSize);
					FMemory::Memcpy(MipData + Index * sizeof(FColor), &TileColor, sizeof(FColor));
				}
			}
		}
	}

	MapTileTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
	MapTileTexture->UpdateResource();
}
void AWorldMapTileActor::OnControlTypeChanged()
{
	UKMWorldMapEditorSubsystem* WorldMapEditorSubsystem = UKMWorldMapEditorSubsystem::Get();
	if (WorldMapEditorSubsystem == nullptr)
	{
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("[AWorldMapTileActor::OnControlTypeChanged] CurrenControlType : %s"), *WorldMapEditorSubsystem->GetControlTypeString());
	if (WorldMapEditorSubsystem->GetControlType() == EWorldMapEditorControlType::Brush)
	{
		BrushDecalComponent->SetVisibility(true);
	}
	else
	{
		BrushDecalComponent->SetVisibility(false);
	}
}

void AWorldMapTileActor::OnSectorAlphaChanged()
{
	UKMWorldMapEditorSubsystem* WorldMapEditorSubsystem = UKMWorldMapEditorSubsystem::Get();
	if (WorldMapEditorSubsystem == nullptr)
	{
		return;
	}
	if (DynamicWorldTileMaterial.IsValid() == false)
	{
		return;
	}
	DynamicWorldTileMaterial->SetScalarParameterValue(FName("Alpha"), WorldMapEditorSubsystem->GetSectorAlpha());
}
