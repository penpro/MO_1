#include "MOMenuPlayerController.h"
#include "Blueprint/UserWidget.h"

void AMOMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Display, TEXT("Player controller loaded"));

	if (!LoadingScreenClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadingScreenClass is NULL on BP_MOMenuPlayerController"));
	}


	bShowMouseCursor = true;
	FInputModeUIOnly Mode; Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(Mode);

	if (IsLocalController() && LoadingScreenClass)
	{
		LoadingScreenInstance = CreateWidget<UUserWidget>(this, LoadingScreenClass);
		UE_LOG(LogTemp, Display, TEXT("Loading Screen created"));
		if (LoadingScreenInstance)
		{
			LoadingScreenInstance->AddToViewport(100);
			UE_LOG(LogTemp, Display, TEXT("Loading screen added to viewport"));
		}
	}
}
