#include "UI_WorldSaveEntry.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "UI_SaveSlotEntry.h"

void UUI_WorldSaveEntry::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (HeaderButton)
	{
		HeaderButton->OnClicked.AddDynamic(this, &UUI_WorldSaveEntry::OnHeaderClicked);
	}
	if (SavesList)
	{
		SavesList->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UUI_WorldSaveEntry::SetupFromData(const FMOWorldSaves& InData)
{
	Data = InData;
	const FString Title = FString::Printf(TEXT("World %s  (%s)"),
		*Data.WorldGuid.ToString(EGuidFormats::Digits),
		*Data.DisplayLevelName.ToString());

	if (WorldTitle) WorldTitle->SetText(FText::FromString(Title));

	// Start collapsed; don’t build children until user expands
	bExpanded = false;
	if (SavesList) SavesList->SetVisibility(ESlateVisibility::Collapsed);
}

void UUI_WorldSaveEntry::OnHeaderClicked()
{
	bExpanded = !bExpanded;

	if (!SavesList) return;
	SavesList->SetVisibility(bExpanded ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	if (bExpanded)
	{
		RebuildChildren();
	}
}

void UUI_WorldSaveEntry::RebuildChildren()
{
	if (!SavesList || !SaveEntryClass) return;

	SavesList->ClearChildren();
	for (const FMOShortSaveInfo& S : Data.Saves)
	{
		if (UUI_SaveSlotEntry* Entry = CreateWidget<UUI_SaveSlotEntry>(this, SaveEntryClass))
		{
			Entry->SetupFromInfo(S);
			SavesList->AddChild(Entry);
		}
	}
}
