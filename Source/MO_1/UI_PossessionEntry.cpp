#include "UI_PossessionEntry.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "MOPlayerController.h"

void UUI_PossessionEntry::SetupEntry(const FGuid& InGuid, const FText& InDisplayName,
									 bool bInIsYou, bool bInIsTaken, FText InTakenBy)
{
	Guid     = InGuid;
	bIsYou   = bInIsYou;
	bIsTaken = bInIsTaken;

	if (DisplayNameText)
		DisplayNameText->SetText(InDisplayName);

	if (YouTag)
	{
		YouTag->SetText(FText::FromString(TEXT("You")));
		YouTag->SetVisibility(bIsYou ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (TakenTag)
	{
		if (bIsTaken && !bIsYou)
		{
			if (!InTakenBy.IsEmpty())
				TakenTag->SetText(FText::FromString(FString::Printf(TEXT("Taken by %s"), *InTakenBy.ToString())));
			else
				TakenTag->SetText(FText::FromString(TEXT("Taken")));
			TakenTag->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			TakenTag->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (PossessButton)
	{
		const bool bEnable = !bIsYou && !bIsTaken;
		PossessButton->SetIsEnabled(bEnable);
		PossessButton->SetVisibility(bEnable ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UUI_PossessionEntry::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (PossessButton)
	{
		PossessButton->OnClicked.Clear();
		PossessButton->OnClicked.AddDynamic(this, &UUI_PossessionEntry::HandlePossessClicked);
	}
}

void UUI_PossessionEntry::HandlePossessClicked()
{
	if (!Guid.IsValid() || bIsYou || bIsTaken) return;

	if (AMOPlayerController* MOPC = Cast<AMOPlayerController>(GetOwningPlayer()))
	{
		MOPC->Server_RequestPossessByGuid(Guid);
	}
}
