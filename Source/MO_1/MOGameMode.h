#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MOGameMode.generated.h"

UCLASS()
class MO_1_API AMOGameMode : public AGameMode
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
};
