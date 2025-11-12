// Fill out your copyright notice in the Description page of Project Settings.

#include "MOCharacter.h"
#include "MOIdentityComponent.h"
#include "MOPosessionSubsystem.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AMOCharacter::AMOCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;             // <— make sure the pawn exists on clients
	SetReplicateMovement(true);
	bNetUseOwnerRelevancy = true;

	//Create Identity with IdentityComponent
	Identity = CreateDefaultSubobject<UMOIdentityComponent>(TEXT("Identity"));
	

	// Create camera boom
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 400.f;
	SpringArm->bUsePawnControlRotation = true;

	// Create follow camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	// Character rotation settings for third-person
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw   = false;
	bUseControllerRotationRoll  = false;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->bOrientRotationToMovement = true; // face direction of travel
		Move->RotationRate = FRotator(0.f, 540.f, 0.f);
	}
}

void AMOCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AMOCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMOCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Intentionally empty. Enhanced Input is bound in the PlayerController.
}

void AMOCharacter::HandleMove(const FVector2D Axis)
{
	// Axis.X = Right/Left, Axis.Y = Forward/Back (from IA_Move Axis2D)
	if (!Controller) return;

	const FRotator ControlRot = Controller->GetControlRotation();
	const FRotator YawOnly(0.f, ControlRot.Yaw, 0.f);

	const FVector Forward = FRotationMatrix(YawOnly).GetUnitAxis(EAxis::X);
	const FVector Right   = FRotationMatrix(YawOnly).GetUnitAxis(EAxis::Y);

	AddMovementInput(Forward, Axis.Y);
	AddMovementInput(Right,   Axis.X);
}

void AMOCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (auto* S = GetWorld()->GetSubsystem<UMOPosessionSubsystem>())
		S->DiscoverLevelPawns();
}
void AMOCharacter::UnPossessed()
{
	Super::UnPossessed();
	if (auto* S = GetWorld()->GetSubsystem<UMOPosessionSubsystem>())
		S->DiscoverLevelPawns();
}
