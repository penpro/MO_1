// Fill out your copyright notice in the Description page of Project Settings.

#include "MOPlayerController.h"
#include "MOCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"

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
