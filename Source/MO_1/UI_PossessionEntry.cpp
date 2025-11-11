#include "UI_PossessionEntry.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
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

	if (UWorld* World = GetWorld())
	{
		if (UMOPosessionSubsystem* Subsys = World->GetSubsystem<UMOPosessionSubsystem>())
		{
			APlayerController* PC = GetOwningPlayer();
			if (Subsys->PossessByGuid(PC, Guid))
			{
				// Optional: close menu after a successful possession
				if (UUserWidget* Root = GetTypedOuter<UUserWidget>())
				{
					Root->RemoveFromParent();
				}
			}
		}
	}
}
