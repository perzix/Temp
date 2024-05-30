#include "KMWorldMapEdMode.h"
#include "KMWorldMapEditor/KMWorldMapEditor.h"
#include "KMWorldMapEditor/MapInfo.h"
#include "KMWorldMapEditor/Tile/WorldMapTileActor.h"
#include "KMWorldMapEdModeToolkit.h"
#include "Toolkits/ToolkitManager.h"
#include "WorldMode/KMWorldGameMode.h"
#include "Subsystem/KMWorldMapEditorSubsystem.h"
class KMWorldMapEditorCommands : public TCommands<KMWorldMapEditorCommands>
{
public:
	KMWorldMapEditorCommands() : TCommands<KMWorldMapEditorCommands>(
								  "KMWorldMapEditor",							 // Context name for fast lookup
								  FText::FromString(TEXT("KMWorldMap Editor")), // context name for displaying
								  NAME_None,								 // Parent
								  FAppStyle::GetAppStyleSetName())
	{
	}

#define LOCTEXT_NAMESPACE ""
	virtual void RegisterCommands() override
	{
		UI_COMMAND(DeletePoint, "Delete Point", "Delete the currently selected point.", EUserInterfaceActionType::Button, FInputGesture(EKeys::Delete));
	}
#undef LOCTEXT_NAMESPACE

public:
	TSharedPtr<FUICommandInfo> DeletePoint;
};

const FEditorModeID FKMWorldMapEdMode::EM_WorldMap(TEXT("EM_WorldMap"));

FKMWorldMapEdMode::FKMWorldMapEdMode()
{
	KMWorldMapEditorCommands::Register();
	ExampleEdModeActions = MakeShareable(new FUICommandList);
}

FKMWorldMapEdMode::~FKMWorldMapEdMode()
{
	KMWorldMapEditorCommands::Unregister();
}

void FKMWorldMapEdMode::MapCommands()
{
}

UKMWorldMapEditorSubsystem* FKMWorldMapEdMode::GetWorldMapEditorSubsystem()
{
	if(GEditor == nullptr)
	{
		return nullptr;
	}
	return GEditor->GetEditorSubsystem<UKMWorldMapEditorSubsystem>();
}

void FKMWorldMapEdMode::Enter()
{
	
	FEdMode::Enter();
	UKMWorldMapEditorSubsystem* WorldMapEditorSubsystem = UKMWorldMapEditorSubsystem::Get();
	MouseButtonHoldType = EKeys::AnyKey;
	if (!Toolkit.IsValid())
	{
		Toolkit = MakeShareable(new FKMWorldMapEdModeToolkit);
		Toolkit->Init(Owner->GetToolkitHost());
	}
	// reset
	UE_LOG(LogTemp, Warning, TEXT("Enter WorldMapEdMode %s"), LexToString(GetWorld()->WorldType));
	//if (GEditor->PlayWorld != nullptr)
	{
		bPlayInEditor = true;


		if (WorldMapTileActor == nullptr || WorldMapTileActor.IsValid() == false)
		{
			UBlueprint* LoadObject = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), NULL, TEXT("/Game/kds218/Blueprints/BP_WorldMapTile.BP_WorldMapTile")));
			if (LoadObject != nullptr)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.ObjectFlags = RF_Transient;
				WorldMapTileActor = GetWorld()->SpawnActor<AWorldMapTileActor>(LoadObject->GeneratedClass, SpawnParams);
			}
			if (WorldMapTileActor != nullptr)
			{
				WorldMapTileActor->Init();
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("WorldMapTileActor is not found"));
			}
		}
		WorldMapTileActor->SetIsTemporarilyHiddenInEditor(false);
	}
	GEditor->SelectNone(true, true);

	MapCommands();
}



void FKMWorldMapEdMode::Exit()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	GEditor->SelectNone(true, true);
	if (Toolkit.IsValid())
	{
		FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
		Toolkit.Reset();
	}

	/*if (WorldMapTileActor.IsValid())
	{
		WorldMapTileActor->Destroy();
		WorldMapTileActor.Reset();
	}*/
	WorldMapTileActor->SetIsTemporarilyHiddenInEditor(true);
	bPlayInEditor = false;
	FEdMode::Exit();
}

void FKMWorldMapEdMode::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{
	FEdMode::Render(View, Viewport, PDI);
}

void FKMWorldMapEdMode::Tick(FEditorViewportClient* ViewportClient, float DeltaTime)
{

	/*bool bNewbPlayInEditor = GEditor->PlayWorld != nullptr;
	if (bNewbPlayInEditor != bPlayInEditor)
	{
		bPlayInEditor = bNewbPlayInEditor;
		if (bPlayInEditor)
		{
			Enter();
		}
		else
		{
			Exit();
		}
	}*/
}

bool FKMWorldMapEdMode::HandleClick(FEditorViewportClient* InViewportClient, HHitProxy* HitProxy, const FViewportClick& Click)
{
	UKMWorldMapEditorSubsystem* WorldMapEditorSubsystem = GetWorldMapEditorSubsystem();
	if(WorldMapEditorSubsystem == nullptr)
	{
		return false;
	}
	UE_LOG(LogTemp, Warning, TEXT("[HandleClick] CurrenControlType : %s"), *WorldMapEditorSubsystem->GetControlTypeString());
	//클릭한 위치의 포지션을 구해서 타일인덱스를 로그로 출력해보자
	if (WorldMapEditorSubsystem->GetControlType() == EWorldMapEditorControlType::Brush)
	{
		if (Click.GetKey() == EKeys::LeftMouseButton)
		{
			WorldMapTileActor->MouseClick(InViewportClient, HitProxy, Click);
			return true;
		}
		else if (Click.GetKey() == EKeys::RightMouseButton)
		{
			WorldMapTileActor->MouseClick(false, true);
			return true;
		}
	}
	else
	{
		if (Click.GetKey() == EKeys::LeftMouseButton)
		{
			//HitPro
			//선택한 타일의 정보를 로그로 출력하고 아웃라인을 그려주자
			FVector WorldPosition = WorldMapEditorSubsystem->GetMousePositionToWorldPosition(InViewportClient, InViewportClient->Viewport, Click.GetClickPos());
			WorldMapEditorSubsystem->PrintTileInfo(WorldPosition);
		}
	}
	
	return false;
}

bool FKMWorldMapEdMode::InputDelta(FEditorViewportClient *InViewportClient, FViewport *InViewport, FVector &InDrag, FRotator &InRot, FVector &InScale)
{

	return false;
}

bool FKMWorldMapEdMode::InputKey(FEditorViewportClient *ViewportClient, FViewport *Viewport, FKey Key, EInputEvent Event)
{
	bool bIsHandled = false;

	if (!bIsHandled && Event == IE_Pressed)
	{
		bIsHandled = ExampleEdModeActions->ProcessCommandBindings(Key, FSlateApplication::Get().GetModifierKeys(), false);
	}
	UKMWorldMapEditorSubsystem* WorldMapEditorSubsystem = GetWorldMapEditorSubsystem();
	if (WorldMapEditorSubsystem == nullptr)
	{
		return false;
	}
	if (WorldMapEditorSubsystem->GetControlType() == EWorldMapEditorControlType::Brush)
	{
		bool bIsControlDown = FSlateApplication::Get().GetModifierKeys().IsControlDown();
		if (bIsControlDown && Key == EKeys::Z && Event == IE_Pressed)
		{
			//언두 기능을 실행한다
			WorldMapTileActor->ProcessUndo();
			return true;
		}

		if (Key == EKeys::LeftMouseButton || (bIsControlDown && Key == EKeys::RightMouseButton))
		{
			if (Event == IE_Pressed)
			{
				MouseButtonHoldType = Key;
				WorldMapTileActor->MouseClick(false, Key == EKeys::RightMouseButton);
				// 카메라 움직임을 비활성화합니다.
				ViewportClient->SetAllowCinematicControl(false);
			}
			else if (Event == IE_Released)
			{
				MouseButtonHoldType = EKeys::AnyKey;
				// 카메라 움직임을 다시 활성화합니다.
				ViewportClient->SetAllowCinematicControl(true);
			}
			return true;
		}
	}
	
	
	return bIsHandled;
}

bool FKMWorldMapEdMode::MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 X, int32 Y)
{
	if (Viewport == nullptr || ViewportClient == nullptr || WorldMapTileActor.IsValid() == false)
	{
		return false;
	}
	WorldMapTileActor->MouseMove(ViewportClient, Viewport, X, Y);
	return true;
}

bool FKMWorldMapEdMode::CapturedMouseMove(FEditorViewportClient* InViewportClient, FViewport* InViewport, int32 InMouseX, int32 InMouseY)
{
	if (InViewportClient == nullptr || InViewport == nullptr || WorldMapTileActor.IsValid() == false)
	{
		return false;
	}
	UKMWorldMapEditorSubsystem* WorldMapEditorSubsystem = GetWorldMapEditorSubsystem();
	WorldMapTileActor->MouseMove(InViewportClient, InViewport, InMouseX, InMouseY);
	if (MouseButtonHoldType == EKeys::LeftMouseButton)
	{
		if (WorldMapEditorSubsystem->IsBrushLineMode())
		{
			WorldMapTileActor->DragLine(false);
		}
		else
		{
			WorldMapTileActor->MouseClick(true, false);
		}

	}
	else if (MouseButtonHoldType == EKeys::RightMouseButton)
	{
		if (WorldMapEditorSubsystem->IsBrushLineMode())
		{
			WorldMapTileActor->DragLine(false);
		}
		else
		{
			WorldMapTileActor->MouseClick(true, true);
		}
	}
	return false;
}

bool FKMWorldMapEdMode::ShowModeWidgets() const
{
	return true;
}

bool FKMWorldMapEdMode::ShouldDrawWidget() const
{
	return true;
}

bool FKMWorldMapEdMode::UsesTransformWidget() const
{
	return false;
}

bool FKMWorldMapEdMode::IsSelectionAllowed(AActor* InActor, bool bInSelection) const
{
	return IsSelectionAllowed();
}



bool FKMWorldMapEdMode::IsSelectionAllowed() const
{
	UKMWorldMapEditorSubsystem* WorldMapEditorSubSystem = UKMWorldMapEditorSubsystem::Get();
	/*if (WorldMapEditorSubSystem != nullptr)
	{
		return bIsSelectionAllowed && WorldMapEditorSubSystem->GetControlType() != EWorldMapEditorControlType::Select;
	}*/
	return bIsSelectionAllowed;
}

void FKMWorldMapEdMode::SetSelectionAllowed(bool bInSelectionAllowed)
{
	bIsSelectionAllowed = bInSelectionAllowed;
}
