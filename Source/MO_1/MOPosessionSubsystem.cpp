#include "MOPosessionSubsystem.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

// Your identity component header from earlier
#include "MOIdentityComponent.h"
#include "MOPossessionTypes.h"

void UMOPosessionSubsystem::DiscoverLevelPawns()
{
	if (bDiscoverInProgress) { return; }
	TGuardValue<bool> ScopeGuard(bDiscoverInProgress, true);

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
			UE_LOG(LogTemp, Display, TEXT("Registered a pawn in possession subsystem"));
		}
	}
	const int32 Num = GuidToPawn.Num();
	const ENetMode NM = GetWorld() ? GetWorld()->GetNetMode() : NM_Standalone;
	UE_LOG(LogTemp, Display, TEXT("[Possession] NM=%d scan -> %d candidates"),
		   static_cast<int32>(NM), Num);
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

void UMOPosessionSubsystem::GetCandidatesFor(APlayerController* Requestor, TArray<FMOPossessionCandidate>& Out) const
{
	Out.Reset();
	for (const auto& KVP : GuidToPawn)
	{
		const FGuid& Guid = KVP.Key;
		APawn* Pawn       = KVP.Value.Get();
		const FText* Nm   = GuidToName.Find(Guid);
		if (!Pawn || !Nm) continue;

		// If the pawn is possessed by ANY PlayerController, hide it (even if it's the Requestor).
		// AIController is OK: we still want to list it as a candidate.
		if (Cast<APlayerController>(Pawn->GetController()) != nullptr)
		{
			continue;
		}

		FMOPossessionCandidate Row;
		Row.Guid        = Guid;
		Row.DisplayName = *Nm;
		Row.Pawn        = Pawn;
		Out.Add(Row);
	}
}

bool UMOPosessionSubsystem::PossessByGuid(APlayerController* PC, const FGuid& Guid)
{
	if (!PC) return false;

	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_Client) // must be server
	{
		return false;
	}

	if (TWeakObjectPtr<APawn>* Found = GuidToPawn.Find(Guid))
	{
		if (APawn* Pawn = Found->Get())
		{
			// Already taken by someone else?
			if (APlayerController* ExistingPC = Cast<APlayerController>(Pawn->GetController()))
			{
				if (ExistingPC != PC) return false;
			}
			PC->Possess(Pawn);
			return true;
		}
	}
	return false;
}

void UMOPosessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Display, TEXT("UMOPosessionSubsystem::Initialize"));
}

void UMOPosessionSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// Defer one tick so placed actors finish BeginPlay
	InWorld.GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(
		this, &UMOPosessionSubsystem::DiscoverLevelPawns));

	UE_LOG(LogTemp, Display, TEXT("UMOPosessionSubsystem::OnWorldBeginPlay scheduled DiscoverLevelPawns"));
}

void UMOPosessionSubsystem::BuildFreeSnapshot(TArray<FMOGuidName>& Out) const
{
	Out.Reset();

	// Iterate the discovered pawns map
	for (const auto& KVP : GuidToPawn)
	{
		const FGuid& Guid = KVP.Key;
		APawn* Pawn       = KVP.Value.Get();
		if (!Pawn) continue;

		// Hide pawns already owned by any PlayerController. AI is allowed.
		if (Cast<APlayerController>(Pawn->GetController()) != nullptr)
		{
			continue;
		}

		const FText* Nm = GuidToName.Find(Guid);
		if (!Nm) continue;

		FMOGuidName Row;
		Row.Guid = Guid;
		Row.DisplayName = *Nm;
		Out.Add(Row);
	}
}

void UMOPosessionSubsystem::BuildFullSnapshot(TArray<FMOGuidName>& Out) const
{
	Out.Reset();

	for (const auto& KVP : GuidToPawn)
	{
		const FGuid& Guid = KVP.Key;
		APawn* Pawn       = KVP.Value.Get();
		if (!Pawn) continue;

		const FText* Nm = GuidToName.Find(Guid);
		if (!Nm) continue;

		FMOGuidName Row;
		Row.Guid        = Guid;
		Row.DisplayName = *Nm;

		if (AController* C = Pawn->GetController())
		{
			if (APlayerController* PC = Cast<APlayerController>(C))
			{
				Row.bTaken = true;
				if (APlayerState* PS = PC->PlayerState)
				{
					Row.TakenBy = FText::FromString(PS->GetPlayerName());  // <- here
				}
				else
				{
					Row.TakenBy = FText::FromString(TEXT("Player"));
				}
			}
			else
			{
				Row.bTaken  = false;
				Row.TakenBy = FText::GetEmpty();
			}
		}
		else
		{
			Row.bTaken  = false;
			Row.TakenBy = FText::GetEmpty();
		}

		Out.Add(Row);
	}
}