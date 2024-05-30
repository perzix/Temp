#include "SKMWorldMapEdModeWidget.h"
#include "KMWorldMapEditor/KMWorldMapEditor.h"
#include "KMWorldMapEdMode.h"
#include "Widgets/Input/SButton.h"
#include "IDesktopPlatform.h"
#include "DesktopPlatformModule.h"
#include "Widgets/Colors/SColorPicker.h"
#include "MapInfo.h"
#include "Subsystem/KMWorldMapEditorSubsystem.h"

void SKMWorldMapEdModeWidget::Construct(const FArguments& InArgs)
{
	WorldMapEditorSubsystem = UKMWorldMapEditorSubsystem::Get();
	WorldMapEditorSubsystem->GetOnControlTypeChanged().AddSP(this, &SKMWorldMapEdModeWidget::UpdateDynamicUIContainer);
	WorldMapEditorSubsystem->GetOnTileSelected().AddSP(this, &SKMWorldMapEdModeWidget::OnTileSelected);

	for (int32 i = 0; i < 5; ++i)
	{
		BrushSizes.Add(MakeShared<int32>(1 + i * 4));
	}
	for (int32 i = 5; i < 10; ++i)
	{
		BrushSizes.Add(MakeShared<int32>(1 + i * 8));
	}
	OpenFileNameTextBlock = SNew(STextBlock).Text(FText::FromString(*WorldMapEditorSubsystem->GetOpenFileName()));
	ChildSlot
	[
		SNew(SScrollBox)
		+ SScrollBox::Slot()
		.VAlign(VAlign_Top)
		.Padding(5.f)
		[
			SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				.Padding(0.f, 0.f, 0.f, 0.f)
				[
					SNew(STextBlock)
						.Text(FText::FromString(TEXT("WorldMap Editor")))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 15))
				]
				+ SVerticalBox::Slot()
				.HAlign(HAlign_Right)
				[
					OpenFileNameTextBlock.ToSharedRef()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 5.f, 0.f, 0.f)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(2)
						[
							SNew(SButton)
								.Text(FText::FromString("New"))
								.OnClicked(this, &SKMWorldMapEdModeWidget::OnMakeNewFile)
						]
						+ SHorizontalBox::Slot()
						//.FillWidth(20.f)
						.AutoWidth()
						.Padding(2)
						.VAlign(VAlign_Center)
						[
							SNew(SButton)
								.Text(FText::FromString("Load"))
								.OnClicked(this, &SKMWorldMapEdModeWidget::OnLoadProtoFile)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(2)
						[
							SNew(SButton)
								.Text(FText::FromString("Save"))
								.OnClicked(this, &SKMWorldMapEdModeWidget::OnSaveProtoFile)
								.IsEnabled(this, &SKMWorldMapEdModeWidget::CanSaveFile)
						]

				]
				+SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 10.f, 0.f, 0.f)
				[
					SNew(SBorder)
						//.BorderImage(FAppStyle::GetNoBrush())  // 실선의 색상을 검은색으로 설정
						//.ColorAndOpacity(FLinearColor::Green)
						.BorderBackgroundColor(FLinearColor::Gray)  // 배경색을 투명으로 설정
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						.Padding(3.0f)  // 실선의 두께를 1로 설정
						[
							SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								.HAlign(HAlign_Center)
								.Padding(0, 0, 2, 0)
								[
									SNew(SButton)
										.Text(FText::FromString("Select"))
										.OnClicked_Lambda([this] { WorldMapEditorSubsystem->SetControlType(EWorldMapEditorControlType::Select); return FReply::Handled(); })
								]
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								.Padding(0, 0, 2, 0)
								[
									SNew(SButton)
										.Text(FText::FromString("Brush"))
										.OnClicked_Lambda([this] { WorldMapEditorSubsystem->SetControlType(EWorldMapEditorControlType::Brush); return FReply::Handled(); })
								]
						]
				]
		]
		+ SScrollBox::Slot()
		.Padding(5.f)
		[
			SAssignNew(DynamicUIContainer, SBox)
		]
	];
	UpdateDynamicUIContainer();
}

void SKMWorldMapEdModeWidget::UpdateDynamicUIContainer()
{
	if (WorldMapEditorSubsystem->GetControlType() == EWorldMapEditorControlType::Brush)
	{
		DynamicUIContainer->SetContent(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 5.f, 0.f, 0.f)
			[
				SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0, 0, 2, 0)
					[
						SNew(STextBlock)
							.Text(FText::FromString(TEXT("Sector Alpha")))
					]
					+ SHorizontalBox::Slot()
					[
						SNew(SSlider)
							.Value(this, &SKMWorldMapEdModeWidget::GetSliderValue)
							.OnValueChanged(this, &SKMWorldMapEdModeWidget::SetSliderValue)
					]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 5.f, 0.f, 0.f)
			[
				SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(2, 0, 0, 0)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
							.Text(FText::FromString("Brush Size"))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(2, 0, 0, 0)
					.VAlign(VAlign_Center)
					[
						// Create the combo box
						SNew(SComboBox<TSharedPtr<int32>>)
							.OptionsSource(&BrushSizes)
							.OnGenerateWidget(this, &SKMWorldMapEdModeWidget::GenerateWidgetForBrushSize)
							.OnSelectionChanged(this, &SKMWorldMapEdModeWidget::OnBrushSizeSelected)
							.InitiallySelectedItem(MakeShared<int32>(WorldMapEditorSubsystem->GetBrushSize()))
							.Content()
							[
								SNew(STextBlock).Text_Lambda([this] { return FText::AsNumber(WorldMapEditorSubsystem->GetBrushSize()); })
							]
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(5, 0, 0, 0)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
							.Text(FText::FromString("LineMode"))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(2, 0, 0, 0)
					.VAlign(VAlign_Center)
					[
						SNew(SCheckBox)
							.IsChecked_Lambda([this]()->ECheckBoxState
								{
									if (WorldMapEditorSubsystem->IsBrushLineMode())
									{
										return ECheckBoxState::Checked;
									}
									return ECheckBoxState::Unchecked;
								})
							.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) { WorldMapEditorSubsystem->SetBrushLineMode(NewState == ECheckBoxState::Checked); })
					]

			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 5.f, 0.f, 0.f)
			[
				SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(2, 0, 2, 0)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
							.Text(FText::FromString("Current Brush Color"))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(2, 0, 0, 0)
					.VAlign(VAlign_Center)
					[
						// 현재 선택된 색상을 표시하는 SColorBlock 위젯을 추가합니다.
						SNew(SColorBlock)
							.Color(this, &SKMWorldMapEdModeWidget::GetCurrentColor)
							.Size(FVector2D(32, 16))
							//.OnMouseButtonDown(this, &SKMWorldMapEdModeWidget::OnColorBlockClicked)

					]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 5.f, 0.f, 0.f)
			[
				SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(2, 0, 2, 0)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
							.Text(FText::FromString("Brush Color"))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(2, 0, 0, 0)
					.VAlign(VAlign_Center)
					[
						// 현재 선택된 색상을 표시하는 SColorBlock 위젯을 추가합니다.
						CreateColorGridPanel()
					]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 5.f, 0.f, 0.f)
			[
				SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1)
					.VAlign(VAlign_Center)
					.Padding(0, 0, 2, 0)
					[
						SNew(SButton)
							.Text(FText::FromString("RefreshSectorId"))
							.OnClicked_Lambda([this] { WorldMapEditorSubsystem->RefreshSectorId(); return FReply::Handled(); })
							.HAlign(HAlign_Center)
					]
			]
		);

	}
	else
	{
		DynamicUIContainer->SetContent(DefaultSelectContents());
	}

}
TSharedRef<SVerticalBox> SKMWorldMapEdModeWidget::DefaultSelectContents()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Left)
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(3, 0, 0, 0)
				[
					SNew(STextBlock)
						.Text(FText::FromString(TEXT("Outliner SelectionAllow")))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(3, 0, 0, 0)
				[
					SNew(SCheckBox)
						.IsChecked_Lambda([this]()->ECheckBoxState
							{
								if (GetEdMode()->IsSelectionAllowed())
								{
									return ECheckBoxState::Checked;
								}
								return ECheckBoxState::Unchecked;
							})
						.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) { GetEdMode()->SetSelectionAllowed(NewState == ECheckBoxState::Checked); })
				]
		];

}
void SKMWorldMapEdModeWidget::OnTileSelected(FIntPoint TileIndex)
{
	TSharedRef<SVerticalBox> DefaultContent = DefaultSelectContents();
	if (WorldMapEditorSubsystem == nullptr || WorldMapEditorSubsystem->GetCurrentMapInfo().IsValid() == false)
	{
		DynamicUIContainer->SetContent(DefaultContent);
		return;
	}
	FTileInfo* FindTileInfo = WorldMapEditorSubsystem->GetCurrentMapInfo()->TileInfoMap.Find(TileIndex);
	if (FindTileInfo == nullptr)
	{
		DynamicUIContainer->SetContent(DefaultContent);
		return;
	}
	DefaultContent->AddSlot()
		.AutoHeight()
		.Padding(0)
		[
			SNew(SBorder)
				.BorderBackgroundColor(FLinearColor::Gray)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.Padding(3.0f) 
				[
					SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Left)
						.Padding(0.f, 0.f, 0.f, 0.f)
						[
							SNew(STextBlock)
								.Text(FText::FromString(FString::Printf(TEXT("TileIndex : %s"), *TileIndex.ToString())))
								.Font(FCoreStyle::GetDefaultFontStyle("NormalFont", 12))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Left)
						.Padding(0.f, 0.f, 0.f, 0.f)
						[
							SNew(STextBlock)
								.Text(FText::FromString(FString::Printf(TEXT("SectorId : %d"), FindTileInfo->SectorId)))
								.Font(FCoreStyle::GetDefaultFontStyle("NormalFont", 12))

						]
				]
		];

	//TODO : FTileInfo를 자동으로 만들어주는게 있으면 좋을거 같다
	DynamicUIContainer->SetContent(DefaultContent);
}


FKMWorldMapEdMode* SKMWorldMapEdModeWidget::GetEdMode() const
{
	return (FKMWorldMapEdMode*)GLevelEditorModeTools().GetActiveMode(FKMWorldMapEdMode::EM_WorldMap);
}

FReply SKMWorldMapEdModeWidget::OnMakeNewFile()
{
	WorldMapEditorSubsystem->InitMapInfo();
	OpenFileNameTextBlock->SetText(FText());
	//다이얼로그로 진짜 만들건지 물어보고 만들게 하기
	return FReply::Handled();
}

FReply SKMWorldMapEdModeWidget::OnLoadJsonFile()
{
	const void* ParentWindowPtr = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(TSharedPtr<SWindow>());
	TArray<FString> OutFiles;
	FString DefaultPath = FPaths::ProjectDir();
	FString DefaultFile = "";
	FString FileTypes = "Json Files (*.json)|*.json";
	if (FDesktopPlatformModule::Get()->OpenFileDialog(ParentWindowPtr, "Select File", DefaultPath, DefaultFile, FileTypes, EFileDialogFlags::None, OutFiles))
	{
		if (OutFiles.Num() > 0)
		{
			// 파일의 내용을 읽습니다.
			FString FileContents;
			if (FFileHelper::LoadFileToString(FileContents, *OutFiles[0]))
			{
				UE_LOG(LogTemp, Warning, TEXT("FileContents : %s"), *FileContents);
				// 파일의 내용을 JSON으로 파싱합니다.
				TSharedPtr<FJsonObject> JsonObject;
				TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FileContents);
				if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
				{
					TSharedPtr<FMapInfo> NewMapInfo = MakeShareable(new FMapInfo);
					NewMapInfo->FromJsonObject(JsonObject);
					WorldMapEditorSubsystem->SetCurrentMapInfo(NewMapInfo);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Failed to parse JSON"));
				}
			}
		}
	}
	return FReply::Handled();
}
FReply SKMWorldMapEdModeWidget::OnLoadProtoFile()
{
	const void* ParentWindowPtr = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(TSharedPtr<SWindow>());
	TArray<FString> OutFiles;
	FString DefaultPath = FPaths::ProjectDir();
	FString DefaultFile = "";
	FString FileTypes = "Proto Files (*.bytes)|*.bytes";
	if (FDesktopPlatformModule::Get()->OpenFileDialog(ParentWindowPtr, "Select File", DefaultPath, DefaultFile, FileTypes, EFileDialogFlags::None, OutFiles))
	{
		if (OutFiles.Num() > 0)
		{
			
			// 파일의 내용을 읽습니다.
			TArray<uint8> FileContents;
			if (FFileHelper::LoadFileToArray(FileContents, *OutFiles[0]))
			{
				TSharedPtr< google::protobuf::PMapInfo> ProtoMapInfo = MakeShareable(new google::protobuf::PMapInfo);
				ProtoMapInfo->ParseFromArray(FileContents.GetData(), FileContents.Num());
				TSharedPtr<FMapInfo> NewMapInfo = MakeShareable(new FMapInfo);
				NewMapInfo->FromProto(ProtoMapInfo);
				WorldMapEditorSubsystem->SetCurrentMapInfo(NewMapInfo);
				WorldMapEditorSubsystem->SetOpenFileName(OutFiles[0]);
				OpenFileNameTextBlock->SetText(FText::FromString(OutFiles[0]));
			}
		}
	}
	return FReply::Handled();
}
bool SKMWorldMapEdModeWidget::CanLoadFile() const
{
	return true;
}

FReply SKMWorldMapEdModeWidget::OnSaveJsonFile()
{
	const void* ParentWindowPtr = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(TSharedPtr<SWindow>());
	TArray<FString> OutFiles;
	FString DefaultPath = FPaths::ProjectDir();
	FString DefaultFile = "";
	FString FileTypes = "Json Files (*.json)|*.json";
	if (FDesktopPlatformModule::Get()->SaveFileDialog(ParentWindowPtr, "Select File", DefaultPath, DefaultFile, FileTypes, EFileDialogFlags::None, OutFiles))
	{
		if (OutFiles.Num() > 0)
		{
			
			// 파일의 내용을 쓴다.
			FString JsonString;
			TSharedPtr<FMapInfo> Info = WorldMapEditorSubsystem->GetCurrentMapInfo();
			if(Info != nullptr)
			{
				TSharedPtr<FJsonObject> JsonObject = Info->ToJsonObject();

				TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
				FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Info is nullptr"));
			}
			FFileHelper::SaveStringToFile(JsonString, *OutFiles[0]);
			WorldMapEditorSubsystem->SetOpenFileName(OutFiles[0]);
			OpenFileNameTextBlock->SetText(FText::FromString(OutFiles[0]));
		}
	}
	return FReply::Handled();
}

FReply SKMWorldMapEdModeWidget::OnSaveProtoFile()
{
	const void* ParentWindowPtr = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(TSharedPtr<SWindow>());
	TArray<FString> OutFiles;
	FString DefaultPath = FPaths::ProjectDir();
	FString DefaultFile = "";
	FString FileTypes = "Proto Files (*.bytes)|*.bytes";
	if (FDesktopPlatformModule::Get()->SaveFileDialog(ParentWindowPtr, "Select File", DefaultPath, DefaultFile, FileTypes, EFileDialogFlags::None, OutFiles))
	{
		if (OutFiles.Num() > 0)
		{
			
			// 파일의 내용을 쓴다.
			FString JsonString;
			TSharedPtr<FMapInfo> Info = WorldMapEditorSubsystem->GetCurrentMapInfo();
			if (Info != nullptr)
			{
				auto Proto = Info->ToProto();
				Proto->ByteSizeLong();
				std::string SerializedData;
				if (Proto->SerializeToString(&SerializedData) == false)
				{
					return FReply::Handled();
				}
				TArray<uint8> Data;
				Data.Append((uint8*)SerializedData.c_str(), SerializedData.length());
				if (!FFileHelper::SaveArrayToFile(Data, *OutFiles[0]))
				{
					UE_LOG(LogTemp, Error, TEXT("Failed to save data to file."));
				}
				WorldMapEditorSubsystem->SetOpenFileName(OutFiles[0]);
				OpenFileNameTextBlock->SetText(FText::FromString(OutFiles[0]));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Info is nullptr"));
			}
		}
	}
	return FReply::Handled();
}


bool SKMWorldMapEdModeWidget::CanSaveFile() const
{
	return WorldMapEditorSubsystem->GetCurrentMapInfo().IsValid();
}
FLinearColor SKMWorldMapEdModeWidget::GetCurrentColor() const
{
	// 현재 선택된 색상을 반환합니다.
	// 이 함수는 SColorBlock 위젯에서 사용됩니다.
	return WorldMapEditorSubsystem->GetBrushColor();
}
FReply SKMWorldMapEdModeWidget::OnColorBlockClicked(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	// 컬러 팔레트를 띄웁니다.
	FColorPickerArgs PickerArgs;
	PickerArgs.bUseAlpha = true;
	PickerArgs.OnColorCommitted = FOnLinearColorValueChanged::CreateSP(this, &SKMWorldMapEdModeWidget::OnColorCommitted);
	PickerArgs.InitialColor = GetCurrentColor();
	OpenColorPicker(PickerArgs);

	return FReply::Handled();
}


void SKMWorldMapEdModeWidget::OnColorCommitted(FLinearColor NewColor)
{
	WorldMapEditorSubsystem->SetBrushColor(NewColor);
}
TSharedRef<SWidget> SKMWorldMapEdModeWidget::GenerateWidgetForBrushSize(TSharedPtr<int32> InBrushSize)
{
	return SNew(STextBlock).Text(FText::AsNumber(*InBrushSize));
}
void SKMWorldMapEdModeWidget::OnBrushSizeSelected(TSharedPtr<int32> SelectedBrushSize, ESelectInfo::Type SelectInfo)
{
	if (SelectedBrushSize.IsValid())
	{
		WorldMapEditorSubsystem->SetBrushSize(*SelectedBrushSize);
	}
}


float SKMWorldMapEdModeWidget::GetSliderValue() const
{
	return WorldMapEditorSubsystem->GetSectorAlpha();
}
void SKMWorldMapEdModeWidget::SetSliderValue(float NewValue)
{
	WorldMapEditorSubsystem->SetSectorAlpha(NewValue);
}

TSharedRef<SGridPanel> SKMWorldMapEdModeWidget::CreateColorGridPanel()
{
	TSharedRef<SGridPanel> ColorGridPanel = SNew(SGridPanel);
	// Add the color blocks to the grid
	const auto& ColorBlocks_ = WorldMapEditorSubsystem->GetColorBlocks();
	for (int32 i = 0; i < ColorBlocks_.Num(); ++i)
	{
		// Calculate the row and column for this color block
		int32 Row = i / 4; // Change this to the number of columns you want
		int32 Column = i % 4; // Change this to the number of columns you want

		// Create a new SColorBlock for this color
		TSharedRef<SColorBlock> ColorBlock = SNew(SColorBlock)
			.Color(ColorBlocks_[i])
			.Size(FVector2D(32, 16))
			.OnMouseButtonDown_Lambda([this, i](const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) { WorldMapEditorSubsystem->SetBrushColor((FLinearColor(WorldMapEditorSubsystem->GetColorBlocks()[i]))); return FReply::Handled(); });

		ColorGridPanel->AddSlot(Column, Row).Padding(1, 1, 1, 1)
			[ColorBlock];
	}
	return ColorGridPanel;
}


