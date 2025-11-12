#pragma once

#include "CoreMinimal.h"
#include "MOPossessionTypes.generated.h"

USTRUCT(BlueprintType)
struct FMOGuidName
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FGuid Guid;

	UPROPERTY(BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly)
	bool  bTaken = false;
	
	UPROPERTY(BlueprintReadOnly)
	FText TakenBy;
};
