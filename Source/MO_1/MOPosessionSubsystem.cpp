#include "MOPosessionSubsystem.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

// Your identity component header from earlier
#include "MOIdentityComponent.h"

void UMOPosessionSubsystem::DiscoverLevelPawns()
{
	GuidToPawn.Empty();
	GuidToName.Empty();

	UWorld* World = GetWorld();
	if (!World) return;

	for (TActorIterator<APawn> It(World); It; ++It)
	{
		APawn* Pawn = *It;
		if (!IsValid(Pawn)) continue;

		if (UMOIdentityComponent* Id = Pawn->FindComponentByClass<UMOIdentityComponent>())
		{
			const FGuid Guid = Id->GetGuid();
			if (!Guid.IsValid()) continue;

			RegisterPawn(Pawn, Guid, Id->GetDisplayName());
		}
	}

	OnCandidatesChanged.Broadcast();
}

void UMOPosessionSubsystem::RegisterPawn(APawn* Pawn, const FGuid& Guid, const FText& DisplayName)
{
	GuidToPawn.Add(Guid, Pawn);
	GuidToName.Add(Guid, DisplayName);
}

void UMOPosessionSubsystem::GetCandidates(TArray<FMOPossessionCandidate>& Out) const
{
	Out.Reset();
	for (const auto& KVP : GuidToPawn)
	{
		const FGuid& Guid = KVP.Key;
		const TWeakObjectPtr<APawn> Pawn = KVP.Value;
		const FText* NamePtr = GuidToName.Find(Guid);
		if (Pawn.IsValid() && NamePtr)
		{
			FMOPossessionCandidate Row;
			Row.Guid = Guid;
			Row.DisplayName = *NamePtr;
			Row.Pawn = Pawn;
			Out.Add(Row);
		}
	}
}

bool UMOPosessionSubsystem::PossessByGuid(APlayerController* PC, const FGuid& Guid)
{
	if (!PC) return false;

	if (TWeakObjectPtr<APawn>* Found = GuidToPawn.Find(Guid))
	{
		if (APawn* Pawn = Found->Get())
		{
			PC->Possess(Pawn);
			return true;
		}
	}
	return false;
}
