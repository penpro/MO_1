#include "UI_Possession.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"    
#include "Blueprint/WidgetTree.h"
#include "UI_PossessionEntry.h"
#include "MOPosessionSubsystem.h"
#include "MOPlayerController.h"   

void UUI_Possession::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (UWorld* World = GetWorld())
	{
		CachedSubsystem = World->GetSubsystem<UMOPosessionSubsystem>();
		if (CachedSubsystem)
		{
			CachedSubsystem->OnCandidatesChanged.AddDynamic(this, &UUI_Possession::OnCandidatesChanged);
		}
	}

	// Bind the refresh button if present in the widget
	if (RefreshButton)
	{
		RefreshButton->OnClicked.Clear();
		RefreshButton->OnClicked.AddDynamic(this, &UUI_Possession::OnRefreshClicked);
	}

	RefreshList();
}

void UUI_Possession::NativeDestruct()
{
	if (CachedSubsystem)
	{
		CachedSubsystem->OnCandidatesChanged.RemoveDynamic(this, &UUI_Possession::OnCandidatesChanged);
	}
	Super::NativeDestruct();
}

void UUI_Possession::OnCandidatesChanged()
{
	RefreshList();
}

void UUI_Possession::OnRefreshClicked()
{
	RequestRefresh();
}

void UUI_Possession::RequestRefresh()
{
	// Ask the server to rescan, then locally refresh
	if (AMOPlayerController* PC = Cast<AMOPlayerController>(GetOwningPlayer()))
	{
		PC->Server_RequestRescanPossession();
	}
	RefreshList();
}

void UUI_Possession::RefreshList()
{
	if (!List || !EntryClass) return;
	List->ClearChildren();

	UWorld* World = GetWorld();
	if (!World) return;

	UMOPosessionSubsystem* Subsys = World->GetSubsystem<UMOPosessionSubsystem>();
	if (!Subsys) return;

	TArray<FMOPossessionCandidate> Candidates;
	Subsys->GetCandidatesFor(GetOwningPlayer(), Candidates);

	// Optional: brief retry if replication is still settling
	if (Candidates.Num() == 0 && RefreshRetryCount < 5)
	{
		++RefreshRetryCount;
		World->GetTimerManager().SetTimer(
			RefreshRetryHandle,
			FTimerDelegate::CreateUObject(this, &UUI_Possession::RefreshList),
			0.25f, false);
		return;
	}
	RefreshRetryCount = 0;

	for (const FMOPossessionCandidate& C : Candidates)
	{
		if (!C.Pawn.IsValid()) continue;

		UUI_PossessionEntry* Row = CreateWidget<UUI_PossessionEntry>(this, EntryClass);
		if (!Row) continue;

		Row->SetupEntry(C.Guid, C.DisplayName);
		List->AddChild(Row);
	}
}
