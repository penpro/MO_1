// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MOCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UMOIdentityComponent;

UCLASS()
class MO_1_API AMOCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMOCharacter();

	/** Called by the PlayerController to move this pawn (X = Right, Y = Forward). */
	UFUNCTION(BlueprintCallable, Category="MO|Input")
	void HandleMove(const FVector2D Axis);
	void PossessedBy(AController* NewController);
	void UnPossessed();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MO|Identity")
	TObjectPtr<UMOIdentityComponent> Identity;
	
	UFUNCTION(BlueprintPure, Category="MO|Identity")
	UMOIdentityComponent* GetIdentity() const { return Identity; }
	

protected:
	virtual void BeginPlay() override;

	/** Third-person camera boom and camera. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MO|Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MO|Camera")
	TObjectPtr<UCameraComponent> Camera;

public:
	virtual void Tick(float DeltaTime) override;
	// We bind input in the PlayerController, so no bindings here.
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
