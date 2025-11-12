#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MOSettingsSaveGame.generated.h"

UCLASS()
class MO_1_API UMOSettingsSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	UPROPERTY() int32 SaveVersion = 1;

	UPROPERTY() float MasterVolume = 1.0f;
	UPROPERTY() int32 ResolutionX = 1920;
	UPROPERTY() int32 ResolutionY = 1080;
	UPROPERTY() bool bFullscreen = true;

	// add keybinds, etc.
};
