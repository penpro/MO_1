// Fill out your copyright notice in the Description page of Project Settings.

#include "MOPlayerController.h"
#include "MOCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "MOPosessionSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "UI_Possession.h"    

void AMOPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Add mapping context at runtime
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsys =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP))
		{
			Subsys->ClearAllMappings();
			if (DefaultMappingContext)
			{
				Subsys->AddMappingContext(DefaultMappingContext, /*Priority*/0);
			}
		}
	}
}

void AMOPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (IA_Move)     { EIC->BindAction(IA_Move,     ETriggerEvent::Triggered, this, &AMOPlayerController::OnMove); }
		if (IA_Look)     { EIC->BindAction(IA_Look,     ETriggerEvent::Triggered, this, &AMOPlayerController::OnLook); }
		if (IA_Interact) { EIC->BindAction(IA_Interact, ETriggerEvent::Started,   this, &AMOPlayerController::OnInteract); }
		if (IA_Possess)  { EIC->BindAction(IA_Possess,  ETriggerEvent::Started,   this, &AMOPlayerController::OnPossessAction); }
	}
}

void AMOPlayerController::OnMove(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();

	if (APawn* P = GetPawn())
	{
		if (AMOCharacter* MOChar = Cast<AMOCharacter>(P))
		{
			MOChar->HandleMove(Axis);
		}
	}
}

void AMOPlayerController::OnLook(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	AddYawInput(Axis.X);
	AddPitchInput(Axis.Y);
}

void AMOPlayerController::OnInteract(const FInputActionValue& /*Value*/)
{
	// Placeholder. Implement as needed or handle in BP via an event.
	 UE_LOG(LogTemp, Log, TEXT("Interact pressed"));
}

void AMOPlayerController::OnPossessAction(const FInputActionValue& /*Value*/)
{
	// Default to a BP hook so you can pop a menu.
	TogglePossessionMenu();
}

void AMOPlayerController::TogglePossessionMenu()
{
	if (!PossessionMenu && PossessionMenuClass)
	{
		PossessionMenu = CreateWidget<UUserWidget>(this, PossessionMenuClass);
	}

	if (!PossessionMenu)
	{
		UE_LOG(LogTemp, Warning, TEXT("PossessionMenuClass is not set on BP_MOPlayerController"));
		return;
	}
	
	// Ask the server to rescan so everyone is in sync
	if (IsLocalController())
	{
		if (HasAuthority())
		{
			// listen server: rescan directly
			if (UWorld* World = GetWorld())
			{
				if (UMOPosessionSubsystem* Subsys = World->GetSubsystem<UMOPosessionSubsystem>())
				{
					Subsys->DiscoverLevelPawns();
				}
			}
		}
		else
		{
			// client: ask the server to rescan and push updates
			Server_RequestRescanPossession();
		}
	}

	if (PossessionMenu->IsInViewport())
	{
		PossessionMenu->RemoveFromParent();
		SetGameOnlyInput();
		OnPossessionMenuToggled(false);
	}
	else
	{
		PossessionMenu->AddToViewport(PossessionMenuZOrder);
		RefreshPossessionMenuUI();

		SetShowMouseCursor(true);
		FInputModeGameAndUI Mode;
		Mode.SetHideCursorDuringCapture(false);
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		Mode.SetWidgetToFocus(PossessionMenu->TakeWidget());
		SetInputMode(Mode);

		OnPossessionMenuToggled(true);
	}
}

void AMOPlayerController::RefreshPossessionMenuUI()
{
	if (UUI_Possession* Poss = Cast<UUI_Possession>(PossessionMenu))
	{
		Poss->RefreshList();
	}
}

void AMOPlayerController::SetGameOnlyInput()
{
	SetShowMouseCursor(false);
	FInputModeGameOnly Mode;
	SetInputMode(Mode);
}

// ===== RPCs =====

void AMOPlayerController::Server_RequestPossessByGuid_Implementation(const FGuid& Guid)
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (UMOPosessionSubsystem* Subsys = World->GetSubsystem<UMOPosessionSubsystem>())
	{
		const bool bOk = Subsys->PossessByGuid(this, Guid);
		if (!bOk) return;

		// Notify all players to refresh their UI
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (AMOPlayerController* MOPC = Cast<AMOPlayerController>(It->Get()))
			{
				MOPC->Client_RefreshPossessionList();
			}
		}
	}
}

void AMOPlayerController::Client_RefreshPossessionList_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		if (UMOPosessionSubsystem* Subsys = World->GetSubsystem<UMOPosessionSubsystem>())
		{
			Subsys->DiscoverLevelPawns(); // local scan
		}
	}
	RefreshPossessionMenuUI();
}

void AMOPlayerController::Server_RequestRescanPossession_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		if (UMOPosessionSubsystem* Subsys = World->GetSubsystem<UMOPosessionSubsystem>())
		{
			Subsys->DiscoverLevelPawns(); // server authoritative rescan

			// Tell every player to refresh their UI
			for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
			{
				if (AMOPlayerController* MOPC = Cast<AMOPlayerController>(It->Get()))
				{
					MOPC->Client_RefreshPossessionList();
				}
			}
		}
	}
}
