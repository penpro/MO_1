#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MOWorldSaveGame.generated.h"

USTRUCT()
struct FMOPersistedActor
{
	GENERATED_BODY()
	UPROPERTY() FGuid Guid;
	UPROPERTY() FName ClassPath;     // Soft class path if you need to respawn
	UPROPERTY() FTransform Transform;
};

UCLASS()
class MO_1_API UMOWorldSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	UPROPERTY() int32 SaveVersion = 1;

	UPROPERTY() FGuid WorldGuid;
	UPROPERTY() FName LevelName;

	// Actors that should exist with these transforms
	UPROPERTY() TArray<FMOPersistedActor> Actors;

	// Actors that were destroyed in this world
	UPROPERTY() TSet<FGuid> DestroyedActorGuids;
};
