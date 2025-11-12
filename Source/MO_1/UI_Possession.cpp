#include "UI_Possession.h"
#include "Components/ScrollBox.h"
#include "Components/CheckBox.h"
#include "UI_PossessionEntry.h"
#include "MOPlayerController.h"
#include "MOIdentityComponent.h"
#include "Components/Button.h"          // for RefreshButton
#include "MOPosessionSubsystem.h"  
#include "MOPossessionTypes.h"

void UUI_Possession::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // Optional: bind the ShowTaken checkbox (you already have this)
    if (ShowTakenCheck)
    {
        ShowTakenCheck->OnCheckStateChanged.Clear();
        ShowTakenCheck->OnCheckStateChanged.AddDynamic(this, &UUI_Possession::OnShowTakenChanged);
        bShowTaken = ShowTakenCheck->IsChecked();
    }

    // Optional: bind the Refresh button
    if (RefreshButton)
    {
        RefreshButton->OnClicked.Clear();
        RefreshButton->OnClicked.AddDynamic(this, &UUI_Possession::OnRefreshClicked);
    }

    // Cache subsystem and bind to its change event
    if (UWorld* World = GetWorld())
    {
        CachedSubsystem = World->GetSubsystem<UMOPosessionSubsystem>();
        if (CachedSubsystem)
        {
            CachedSubsystem->OnCandidatesChanged.AddDynamic(this, &UUI_Possession::OnCandidatesChanged);
        }
    }

    RefreshList();
}

void UUI_Possession::OnShowTakenChanged(bool bChecked)
{
    bShowTaken = bChecked;
    RefreshList();
}

void UUI_Possession::RefreshList()
{
    if (!List || !EntryClass) return;
    List->ClearChildren();

    AMOPlayerController* PC = Cast<AMOPlayerController>(GetOwningPlayer());
    if (!PC) return;

    // "You" row
    FGuid SelfGuid;
    FText SelfName;
    if (APawn* SelfPawn = PC->GetPawn())
    {
        if (auto* Id = SelfPawn->FindComponentByClass<UMOIdentityComponent>())
        {
            if (Id->HasValidGuid())
            {
                SelfGuid = Id->GetGuid();
                SelfName = Id->GetDisplayName();
                if (UUI_PossessionEntry* SelfRow = CreateWidget<UUI_PossessionEntry>(this, EntryClass))
                {
                    SelfRow->SetupEntry(SelfGuid, SelfName, /*bIsYou=*/true, /*bIsTaken=*/false, FText());
                    List->AddChild(SelfRow);
                }
            }
        }
    }

    // Server snapshot
    TArray<FMOGuidName> Snapshot;
    PC->GetCachedCandidates(Snapshot);

    for (const FMOGuidName& E : Snapshot)
    {
        // Skip our own pawn if it happens to appear in the snapshot
        if (SelfGuid.IsValid() && E.Guid == SelfGuid) continue;

        // Filter: if not showing taken, skip taken ones
        if (!bShowTaken && E.bTaken) continue;

        if (UUI_PossessionEntry* Row = CreateWidget<UUI_PossessionEntry>(this, EntryClass))
        {
            Row->SetupEntry(E.Guid, E.DisplayName, /*bIsYou=*/false, /*bIsTaken=*/E.bTaken, /*TakenBy=*/E.TakenBy);
            List->AddChild(Row);
        }
    }
}
void UUI_Possession::RequestRefresh()
{
    // Ask the server for an authoritative rescan, then rebuild locally
    if (AMOPlayerController* PC = Cast<AMOPlayerController>(GetOwningPlayer()))
    {
        PC->Server_RequestRescanPossession(); // server will push snapshot to everyone
    }
    RefreshList();
}

void UUI_Possession::OnCandidatesChanged()
{
    // Subsystem told us the candidate set changed — rebuild the list
    RefreshList();
}

void UUI_Possession::OnRefreshClicked()
{
    // Refresh button forwards to the same logic
    RequestRefresh();
}

void UUI_Possession::NativeDestruct()
{
    // Unbind delegates and clear timers before destruction
    if (CachedSubsystem)
    {
        CachedSubsystem->OnCandidatesChanged.RemoveDynamic(this, &UUI_Possession::OnCandidatesChanged);
        CachedSubsystem = nullptr;
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RefreshRetryHandle);
    }

    Super::NativeDestruct();
}

