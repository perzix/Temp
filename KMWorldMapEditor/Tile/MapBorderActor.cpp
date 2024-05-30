// Fill out your copyright notice in the Description page of Project Settings.


#include "Tile/MapBorderActor.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Subsystem/KMWorldMapEditorSubsystem.h"

// Sets default values
AMapBorderActor::AMapBorderActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
	SplineMeshComponent = CreateDefaultSubobject<USplineMeshComponent>(TEXT("SplineMeshComponent"));
}

// Called when the game starts or when spawned
void AMapBorderActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMapBorderActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
void AMapBorderActor::CreateBoundarySpline(const TArray<FIntPoint>& InBoundaryTiles)
{
	// Add the boundary tiles to the spline
	TArray<FVector> Positions;
	FCollisionQueryParams QueryParams;
	QueryParams.bFindInitialOverlaps = true;
	UKMWorldMapEditorSubsystem* WorldMapSubsystem = GEditor->GetEditorSubsystem<UKMWorldMapEditorSubsystem>();
	if (WorldMapSubsystem == nullptr)
	{
		return;
	}
	float PrevHeight = -999999.f;
	for (int32 i = 0; i < InBoundaryTiles.Num(); ++i)
	{
		const FIntPoint& TileIndex = InBoundaryTiles[i];
		const FIntPoint& PreviousTileIndex = i == 0 ? InBoundaryTiles.Last() : InBoundaryTiles[i - 1];
		const FIntPoint& NextTileIndex = i == InBoundaryTiles.Num() - 1 ? InBoundaryTiles[0] : InBoundaryTiles[i + 1];


		FIntPoint DeltaTileIndex = TileIndex - PreviousTileIndex;
		FIntPoint DeltaNextTileIndex = NextTileIndex - TileIndex;

		FVector WorldPosition = WorldMapSubsystem->TileIndexToWorldPosition(TileIndex);
		WorldPosition.Z = WorldMapSubsystem->GetLandscapeHeight(WorldPosition);

		if(DeltaTileIndex == DeltaNextTileIndex && FMath::Abs(PrevHeight - WorldPosition.Z) < 500.f)
		{
			continue;
		}
		PrevHeight = WorldPosition.Z;

		//지형높이에 맞게 z값을 조절해준다

		//이전 월드 포지션과 현재 월드 포지션의 방향을 구한다
		//FHitResult HitResult;
		//if (GetWorld()->LineTraceSingleByChannel(
		//	HitResult,
		//	WorldPosition + FVector(0, 0, 99999),
		//	WorldPosition + FVector(0, 0, -99999),
		//	ECollisionChannel::ECC_WorldStatic, QueryParams))//ECC_WorldStatic 은 테스트를 위해 임시로 사용한것임 (추후 변경 필요)
		//{
		//	WorldPosition.Z = HitResult.ImpactPoint.Z;
		//}
		Positions.Add(WorldPosition);
	}
	CreateBoundarySpline(Positions);
}
void AMapBorderActor::CreateBoundarySpline(const TArray<FVector>& Positions)
{
	// Clear the spline points
	SplineComponent->ClearSplinePoints(true);

	// Add the boundary tiles to the spline
	int SplinePointIndex = 0;
	for (const FVector& WorldPosition : Positions)
	{
		SplineComponent->AddSplinePoint(WorldPosition, ESplineCoordinateSpace::World, true);
		SplineComponent->SetSplinePointType(SplinePointIndex, ESplinePointType::Linear);
		SplinePointIndex++;
	}

	// Close the spline loop
	SplineComponent->SetClosedLoop(true, true);
	

	// Apply the spline mesh to the spline
	for (int32 i = 0; i < SplineComponent->GetNumberOfSplineSegments(); i++)
	{
		USplineMeshComponent* splineMesh = NewObject<USplineMeshComponent>(SplineComponent);
		splineMesh->SetReceivesDecals(false);
		splineMesh->SetCastShadow(false);

		FVector StartLocation = SplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
		FVector EndLocation = SplineComponent->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::World);

		splineMesh->SetStaticMesh(SplineMeshComponent->GetStaticMesh());
		splineMesh->SetStartOffset(SplineMeshComponent->GetStartOffset());
		splineMesh->SetEndOffset(SplineMeshComponent->GetEndOffset());

		splineMesh->SetStartScale(SplineMeshComponent->GetStartScale());
		splineMesh->SetEndScale(SplineMeshComponent->GetEndScale());
		splineMesh->SetMaterial(0, SplineMeshComponent->GetMaterial(0));
		splineMesh->SetStartAndEnd(SplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World),
			SplineComponent->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::World),
			SplineComponent->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::World),
			SplineComponent->GetTangentAtSplinePoint(i + 1, ESplineCoordinateSpace::World));
		splineMesh->AttachToComponent(SplineComponent, FAttachmentTransformRules::KeepWorldTransform);
		splineMesh->RegisterComponent();
	}
}


