#include "MOGameMode.h"

#include "MOPersistenceSubsystem.h"
#include "MOPlayerController.h"
#include "MOPosessionSubsystem.h"

#include "TimerManager.h"
#include "Engine/World.h"

void AMOGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Defer one tick so level-placed actors finish BeginPlay
	FTimerHandle H;
	GetWorldTimerManager().SetTimer(H, [this]()
	{
		if (UWorld* World = GetWorld())
		{
			if (UMOPosessionSubsystem* Subsys = World->GetSubsystem<UMOPosessionSubsystem>())
			{
				Subsys->DiscoverLevelPawns();
				UE_LOG(LogTemp, Display, TEXT("Possession subsystem initialized in game mode"));
			}
		}
	}, 0.0f, false);

	if (auto* PS = GetGameInstance()->GetSubsystem<UMOPersistenceSubsystem>())
	{
		PS->ApplyPendingLoadIfAny();
	}
}

void AMOGameMode::PostLogin(APlayerController* NewPC)
{
	Super::PostLogin(NewPC);

	if (UWorld* W = GetWorld())
	{
		if (auto* Subsys = W->GetSubsystem<UMOPosessionSubsystem>())
		{
			TArray<FMOGuidName> Snap;
			Subsys->BuildFreeSnapshot(Snap);

			if (auto* MOPC = Cast<AMOPlayerController>(NewPC))
			{
				MOPC->Client_SetCandidateSnapshot(Snap);
			}
		}
	}
}
