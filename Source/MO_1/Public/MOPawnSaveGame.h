#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MOPawnSaveGame.generated.h"

USTRUCT()
struct FMOItemStack
{
	GENERATED_BODY()
	UPROPERTY() FName ItemId;
	UPROPERTY() int32 Count = 0;
};

UCLASS()
class MO_1_API UMOPawnSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	UPROPERTY() int32 SaveVersion = 1;
	UPROPERTY() FGuid PawnGuid;

	UPROPERTY() FTransform Transform;

	// Example inventory payload
	UPROPERTY() TArray<FMOItemStack> Inventory;

	// Add stats, health, equipment, skills, etc.
};
