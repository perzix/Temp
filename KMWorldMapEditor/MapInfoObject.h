// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "MapInfoObject.generated.h"

/**
 * 
 */
UCLASS()
class KMWORLDMAPEDITOR_API UMapInfoObject : public UObject
{
	GENERATED_BODY()
public:
	int32 WorldMapSize;
	TMap<FIntPoint, FColor> TileColorMap;
	TSharedPtr<FJsonObject> ToJsonObject();
	void FromJsonObject(TSharedPtr<FJsonObject> JsonObject);
};
