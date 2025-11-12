#pragma once
#include "CoreMinimal.h"
#include "MOUITypes.generated.h"

UENUM(BlueprintType)
enum class EMOMenuTab : uint8
{
	None       UMETA(DisplayName="None"),
	Options    UMETA(DisplayName="Options"),
	Possession UMETA(DisplayName="Possession"),
	Crafting   UMETA(DisplayName="Crafting"),
	Skills     UMETA(DisplayName="Skills"),
	Wiki       UMETA(DisplayName="Wiki"),
};
