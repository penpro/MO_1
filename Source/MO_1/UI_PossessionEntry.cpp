#include "UI_PossessionEntry.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "MOPlayerController.h"
#include "MOPosessionSubsystem.h"

void UUI_PossessionEntry::SetupEntry(const FGuid& InGuid, const FText& InDisplayName)
{
	Guid = InGuid;
	if (DisplayNameText)
	{
		DisplayNameText->SetText(InDisplayName);
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
	if (!Guid.IsValid()) return;

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (AMOPlayerController* MOPC = Cast<AMOPlayerController>(PC))
		{
			// Ask the server to possess
			MOPC->Server_RequestPossessByGuid(Guid);

			// Close menu locally and return to game-only input
			if (UUserWidget* Root = GetTypedOuter<UUserWidget>())
			{
				Root->RemoveFromParent();
			}
			MOPC->SetGameOnlyInput();
		}
	}
}
