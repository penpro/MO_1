#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MOMenuPlayerController.generated.h"

class UUserWidget;

UCLASS()
class MO_1_API AMOMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MO|Menu")
	TSubclassOf<UUserWidget> LoadingScreenClass;

	UPROPERTY() TObjectPtr<UUserWidget> LoadingScreenInstance;
};
