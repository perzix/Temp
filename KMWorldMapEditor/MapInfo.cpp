// Fill out your copyright notice in the Description page of Project Settings.


#include "MapInfo.h"
#include "Subsystem/KMWorldMapEditorSubsystem.h"


FMapInfo::FMapInfo()
{
	WorldMapSize = 0;
	TileInfoMap.Empty();
}

TSharedPtr<FJsonObject> FMapInfo::ToJsonObject()
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	//JsonObject->SetNumberField("WorldMapSize", WorldMapSize);
	//TSharedPtr<FJsonObject> TileColorMapObject = MakeShareable(new FJsonObject);
	//for (const auto& Pair : TileInfoMap)
	//{
	//	// 키를 문자열로 변환합니다.
	//	FString KeyString = FString::Printf(TEXT("%d,%d"), Pair.Key.X, Pair.Key.Y);

	//	// 값을 문자열로 변환합니다.
	//	FString ValueString = Pair.Value.ToString();

	//	// 키-값 쌍을 FJsonObject에 저장합니다.
	//	TileColorMapObject->SetStringField(KeyString, ValueString);
	//}
	//JsonObject->SetObjectField("TileInfoMap", TileColorMapObject);
	return JsonObject;
}

void FMapInfo::FromJsonObject(TSharedPtr<FJsonObject> JsonObject)
{
	// WorldMapSize를 읽어옵니다.
	//WorldMapSize = JsonObject->GetIntegerField("WorldMapSize");

	//// TileColorMap을 읽어옵니다.
	//TSharedPtr<FJsonObject> TileInfoMapObject = JsonObject->GetObjectField("TileInfoMap");
	//for (const auto& Pair : TileInfoMapObject->Values)
	//{
	//	// 키를 FIntPoint로 변환합니다.
	//	FString KeyString = Pair.Key;
	//	FString ValueString = Pair.Value->AsString();
	//	FIntPoint Key;
	//	FString XString;
	//	FString YString;
	//	KeyString.Split(",", &XString, &YString);
	//	Key.X = FCString::Atoi(*XString);
	//	Key.Y = FCString::Atoi(*YString);

	//	// 값을 FColor로 변환합니다.
	//	FColor Value;
	//	Value.InitFromString(ValueString);
	//	// 키-값 쌍을 TileColorMap에 저장합니다.
	//	TileInfoMap.Add(Key, Value);
	//}
}

void FMapInfo::FromProto(TSharedPtr<google::protobuf::PMapInfo> MapInfoProto)
{
	if (MapInfoProto.IsValid() == false)
	{
		return;
	}
	WorldMapSize = MapInfoProto->worldmapsize();
	TileInfoMap.Empty();
	UKMWorldMapEditorSubsystem* WorldMapEditorSubsystem = GEditor->GetEditorSubsystem<UKMWorldMapEditorSubsystem>();
	const auto& ColorBlocks = WorldMapEditorSubsystem->GetColorBlocks();

	for(int i = 0; i < MapInfoProto->tileinfos_size(); ++i)
	{
		auto& TileInfo = MapInfoProto->tileinfos(i);
		FIntPoint Key(TileInfo.tileindex().x(), TileInfo.tileindex().y());
		FTileInfo Info;
		Info.SectorId = TileInfo.sectorid();
		Info.SectorColor = ColorBlocks[TileInfo.sectorid() % ColorBlocks.Num()];
		TileInfoMap.Add(Key, Info);
	}
}



TSharedPtr<google::protobuf::PMapInfo> FMapInfo::ToProto()
{
	TSharedPtr<google::protobuf::PMapInfo> MapInfoProto = MakeShared<google::protobuf::PMapInfo>();
	MapInfoProto->set_worldmapsize(WorldMapSize);
	for (const auto& Element : TileInfoMap)
	{
		auto TileInfo = MapInfoProto->add_tileinfos();
		google::protobuf::PTileIndex* TileIndex = new google::protobuf::PTileIndex();
		TileIndex->set_x(Element.Key.X);
		TileIndex->set_y(Element.Key.Y);
		TileInfo->set_allocated_tileindex(TileIndex);
		TileInfo->set_sectorid(Element.Value.SectorId);

	}
	return MapInfoProto;
}

static FIntPoint Directions[4] = { FIntPoint(1, 0), FIntPoint(-1, 0), FIntPoint(0, 1), FIntPoint(0, -1) };
void FMapInfo::DoDFS(int32 SectorId, FIntPoint InStart, FTileInfo InTileInfo, TMap<FIntPoint, FTileInfo>& InTileInfoMap, TMap<FIntPoint, bool>& OutVisited)
{
	TArray<FIntPoint> Stack_;

	Stack_.Push(InStart);

	while (Stack_.Num() > 0) 
	{
		FIntPoint CurrentIndex = Stack_.Top();
		Stack_.Pop();

		if (OutVisited.Contains(CurrentIndex)) 
		{
			continue;
		}

		OutVisited.Add(CurrentIndex, true);

		FTileInfo* CurrentTileInfo = InTileInfoMap.Find(CurrentIndex);
		if (CurrentTileInfo == nullptr) 
		{
			continue;
		}
		CurrentTileInfo->SectorId = SectorId;

		for (const FIntPoint& Dir : Directions) {
			FIntPoint NextIndex = CurrentIndex + Dir;

			const FTileInfo* NextTileInfo = InTileInfoMap.Find(NextIndex);
			bool* VisitedNext = OutVisited.Find(NextIndex);

			if ((NextTileInfo == nullptr || NextTileInfo->SectorColor == InTileInfo.SectorColor) && (VisitedNext == nullptr || (*VisitedNext) == false)) {
				Stack_.Push(NextIndex);
			}
		}
	}

}
void FMapInfo::DoBFS(int32 SectorId, FIntPoint InStart, FTileInfo InTileInfo, TMap<FIntPoint, FTileInfo>& InTileInfoMap, TMap<FIntPoint, bool>& OutVisited)
{
	TArray<FIntPoint> Queue_;

	Queue_.Push(InStart);

	while (Queue_.Num() > 0)
	{
		FIntPoint CurrentIndex = Queue_[0];
		Queue_.RemoveAt(0);

		if (OutVisited.Contains(CurrentIndex))
		{
			continue;
		}

		OutVisited.Add(CurrentIndex, true);

		FTileInfo* CurrentTileInfo = InTileInfoMap.Find(CurrentIndex);
		if (CurrentTileInfo == nullptr)
		{
			continue;
		}
		CurrentTileInfo->SectorId = SectorId;

		for (const FIntPoint& Dir : Directions) {
			FIntPoint NextIndex = CurrentIndex + Dir;

			const FTileInfo* NextTileInfo = InTileInfoMap.Find(NextIndex);
			bool* VisitedNext = OutVisited.Find(NextIndex);

			if ((NextTileInfo == nullptr || NextTileInfo->SectorColor == InTileInfo.SectorColor) && (VisitedNext == nullptr || (*VisitedNext) == false)) {
				Queue_.Push(NextIndex);
			}
		}
	}
}
void FMapInfo::UpdateSecotorId()
{
	UKMWorldMapEditorSubsystem* WorldMapEditorSubsystem = UKMWorldMapEditorSubsystem::Get();
	if(WorldMapEditorSubsystem == nullptr)
	{
		return;
	}
	const auto& ColorBlocks = WorldMapEditorSubsystem->GetColorBlocks();
	TMap<FIntPoint, bool> Visited;
	TSet<int32> SectorIds;
	for (const auto& Pair : TileInfoMap)
	{
		FTileInfo TileInfo = Pair.Value;
		if (TileInfo.SectorId == -1)
		{
			continue;
		}
		SectorIds.Add(TileInfo.SectorId);
	}
	int32 SectorId = 0;
	for (const auto& TileInfo : TileInfoMap)
	{
		FIntPoint TileIndex = TileInfo.Key;
		if (Visited.Contains(TileIndex))
		{
			continue;
		}
		// 해당 타일에서 시작하는 DFS 수행
		if (TileInfo.Value.SectorId != -1)
		{
			SectorId = TileInfo.Value.SectorId;
		}
		else
		{
			//SectorColor에 해당하는 SectorId를 찾는다.
			auto FindIndex = ColorBlocks.Find(TileInfo.Value.SectorColor);
			if (FindIndex != INDEX_NONE)
			{
				SectorId = FindIndex;
				int32 LoopCount = 1;
				while (SectorIds.Contains(SectorId))
				{
					SectorId = FindIndex + LoopCount * ColorBlocks.Num();
					++LoopCount;
				}
				SectorIds.Add(SectorId);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("SectorColor is not found. Color : %s"), *TileInfo.Value.SectorColor.ToString());
				return;
			}
		}

		DoBFS(SectorId, TileIndex, TileInfoMap[TileIndex], TileInfoMap, Visited);
	}

}

FTileInfo::FTileInfo()
{
	SectorId = -1;
	SectorColor = FColor::Transparent;
}

FWorldBuildingObjectInfo::FWorldBuildingObjectInfo()
{
	Position = nullptr;
}
