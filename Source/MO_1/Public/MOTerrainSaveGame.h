#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MOTerrainSaveGame.generated.h"

USTRUCT()
struct FMOTerrainPatch
{
	GENERATED_BODY()
	UPROPERTY() FIntPoint Chunk;
	UPROPERTY() TArray<uint8> Payload; // your encoded edits
};

UCLASS()
class MO_1_API UMOTerrainSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	UPROPERTY() int32 SaveVersion = 1;
	UPROPERTY() FGuid WorldGuid;

	UPROPERTY() TArray<FMOTerrainPatch> Patches;
};
