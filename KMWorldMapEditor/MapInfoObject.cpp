// Fill out your copyright notice in the Description page of Project Settings.


#include "MapInfoObject.h"

TSharedPtr<FJsonObject> UMapInfoObject::ToJsonObject()
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetNumberField("WorldMapSize", WorldMapSize);
	TSharedPtr<FJsonObject> TileColorMapObject = MakeShareable(new FJsonObject);
	for (const auto& Pair : TileColorMap)
	{
		// 키를 문자열로 변환합니다.
		FString KeyString = FString::Printf(TEXT("%d,%d"), Pair.Key.X, Pair.Key.Y);

		// 값을 문자열로 변환합니다.
		FString ValueString = Pair.Value.ToString();

		// 키-값 쌍을 FJsonObject에 저장합니다.
		TileColorMapObject->SetStringField(KeyString, ValueString);
	}
	JsonObject->SetObjectField("TileColorMap", TileColorMapObject);
	return JsonObject;
}

void UMapInfoObject::FromJsonObject(TSharedPtr<FJsonObject> JsonObject)
{
	// WorldMapSize를 읽어옵니다.
	WorldMapSize = JsonObject->GetIntegerField("WorldMapSize");

	// TileColorMap을 읽어옵니다.
	TSharedPtr<FJsonObject> TileColorMapObject = JsonObject->GetObjectField("TileColorMap");
	for (const auto& Pair : TileColorMapObject->Values)
	{
		// 키를 FIntPoint로 변환합니다.
		FString KeyString = Pair.Key;
		FString ValueString = Pair.Value->AsString();
		FIntPoint Key;
		FString XString;
		FString YString;
		KeyString.Split(",", &XString, &YString);
		Key.X = FCString::Atoi(*XString);
		Key.Y = FCString::Atoi(*YString);

		// 값을 FColor로 변환합니다.
		FColor Value;
		Value.InitFromString(ValueString);
		// 키-값 쌍을 TileColorMap에 저장합니다.
		TileColorMap.Add(Key, Value);
	}
}
