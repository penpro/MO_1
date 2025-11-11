#include "UI_Possession.h"
#include "Components/ScrollBox.h"
#include "Blueprint/WidgetTree.h"
#include "UI_PossessionEntry.h"
#include "MOPosessionSubsystem.h"

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

void UUI_Possession::RefreshList()
{
	if (!List || !EntryClass) return;

	List->ClearChildren();

	if (!CachedSubsystem)
	{
		if (UWorld* World = GetWorld())
		{
			CachedSubsystem = World->GetSubsystem<UMOPosessionSubsystem>();
		}
		if (!CachedSubsystem) return;
	}

	TArray<FMOPossessionCandidate> Candidates;
	CachedSubsystem->GetCandidates(Candidates);

	for (const FMOPossessionCandidate& C : Candidates)
	{
		if (!C.Pawn.IsValid()) continue;

		UUI_PossessionEntry* Row = CreateWidget<UUI_PossessionEntry>(this, EntryClass);
		if (!Row) continue;

		Row->SetupEntry(C.Guid, C.DisplayName);
		List->AddChild(Row);
	}
}
