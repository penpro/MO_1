#include "MOGameMode.h"
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
			}
		}
	}, 0.0f, false);
}
