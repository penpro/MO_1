// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "MOPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS()
class MO_1_API AMOPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMOPlayerController() = default;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	/** Assign these in BP_MOPlayerController */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MO|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MO|Input")
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MO|Input")
	TObjectPtr<UInputAction> IA_Look;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MO|Input")
	TObjectPtr<UInputAction> IA_Interact;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MO|Input")
	TObjectPtr<UInputAction> IA_Possess;

private:
	// Enhanced Input handlers
	void OnMove(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void OnInteract(const FInputActionValue& Value);
	void OnPossessAction(const FInputActionValue& Value);

public:
	/** Optional BP hook for showing a possession menu */
	UFUNCTION(BlueprintImplementableEvent, Category="MO|UI")
	void TogglePossessionMenu();
};
