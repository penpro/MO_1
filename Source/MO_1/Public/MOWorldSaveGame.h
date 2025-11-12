#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MOWorldSaveGame.generated.h"

USTRUCT()
struct FMOPersistedActor
{
	GENERATED_BODY()

	UPROPERTY() FGuid      Guid;
	UPROPERTY() FName      ClassPath;     // Optional: soft class path if you need to respawn
	UPROPERTY() FTransform Transform;
};

USTRUCT()
struct FMOPawnState
{
	GENERATED_BODY()

	UPROPERTY() FGuid      PawnGuid;
	UPROPERTY() FTransform Transform;
	UPROPERTY() FVector    Velocity = FVector::ZeroVector;
};

UCLASS()
class MO_1_API UMOWorldSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	UPROPERTY() int32 SaveVersion = 1;

	// Identity of the world this save belongs to
	UPROPERTY() FGuid WorldGuid;

	// Which level to load before applying this save
	UPROPERTY() FName LevelName;

	// -------- Metadata read by the Save Browser UI --------
	// UTC timestamp when the save was written
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Meta")
	FDateTime SavedAt;

	// Total accumulated play time up to this save (hh:mm:ss in UI)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Meta")
	float PlaySeconds = 0.f;

	// True if this save was created by the autosave flow
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Meta")
	bool bIsAutosave = false;

	// Optional user label (e.g., "After town quest")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Meta")
	FString Label;

	// -------- World state payload --------
	UPROPERTY() TArray<FMOPersistedActor> Actors;        // actors with transforms
	UPROPERTY() TSet<FGuid>               DestroyedActorGuids; // actors removed by player
	UPROPERTY() TArray<FMOPawnState> PawnStates;
};
