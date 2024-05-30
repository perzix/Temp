// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Protobuf/PMapInfo.pb.h"

//using namespace google::protobuf;
class FTileInfo
{
public:
	FTileInfo();
	int32 SectorId;
	FColor SectorColor;//컬러는 데이터에 저장되지 않는다.
};
class FWorldBuildingObjectInfo
{
public:
	FWorldBuildingObjectInfo();
	int32 BuildingObjectId;
	int32 SectorId;
	TArray<FIntPoint> TileIndices;
	FVector* Position;
};

class FMapInfo
{
public:
	FMapInfo();
	int32 WorldMapSize;
	TMap<FIntPoint, FTileInfo> TileInfoMap;
	TArray<FWorldBuildingObjectInfo> WorldBuildingObjectInfos;
	TSharedPtr<FJsonObject> ToJsonObject();
	void FromJsonObject(TSharedPtr<FJsonObject> JsonObject);
	void FromProto(TSharedPtr<google::protobuf::PMapInfo> MapInfoProto);
	TSharedPtr<google::protobuf::PMapInfo> ToProto();
	void DoDFS(int32 SectorId, FIntPoint InStart, FTileInfo InTileInfo, TMap<FIntPoint, FTileInfo>& InTileInfoMap, TMap<FIntPoint, bool>& OutVisited);
	void DoBFS(int32 SectorId, FIntPoint InStart, FTileInfo InTileInfo, TMap<FIntPoint, FTileInfo>& InTileInfoMap, TMap<FIntPoint, bool>& OutVisited);
	void UpdateSecotorId();
};
