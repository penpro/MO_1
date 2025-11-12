#include "UI_SaveBrowser.h"
#include "Components/ScrollBox.h"
#include "UI_WorldSaveEntry.h"
#include "MOPersistenceSubsystem.h"

void UUI_SaveBrowser::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	RefreshList();
	if (auto* PS = GetGameInstance()->GetSubsystem<UMOPersistenceSubsystem>())
	{
		PS->OnSavesChanged.AddDynamic(this, &UUI_SaveBrowser::RefreshList);
	}
}

void UUI_SaveBrowser::RefreshList()
{
	if (!WorldsList || !WorldEntryClass) return;

	WorldsList->ClearChildren();

	TArray<FMOWorldSaves> Worlds;
	if (auto* PS = GetGameInstance()->GetSubsystem<UMOPersistenceSubsystem>())
	{
		PS->QueryAllWorldSaves(Worlds);
	}

	for (const FMOWorldSaves& WS : Worlds)
	{
		if (UUI_WorldSaveEntry* Row = CreateWidget<UUI_WorldSaveEntry>(this, WorldEntryClass))
		{
			Row->SetupFromData(WS);     // populate header + nested saves
			WorldsList->AddChild(Row);
		}
	}
}
