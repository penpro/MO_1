// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "MOPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UUserWidget;               
class UUI_Possession;

USTRUCT(BlueprintType)
struct FMOGuidName
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly) FGuid Guid;
	UPROPERTY(BlueprintReadOnly) FText  DisplayName;
};

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MO|UI")
	TSubclassOf<UUserWidget> PossessionMenuClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MO|UI")
	int32 PossessionMenuZOrder = 1000;

private:
	// Enhanced Input handlers
	void OnMove(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void OnInteract(const FInputActionValue& Value);
	void OnPossessAction(const FInputActionValue& Value);

	UPROPERTY() TObjectPtr<UUserWidget> PossessionMenu;

	void RefreshPossessionMenuUI();

public:

	UFUNCTION(BlueprintImplementableEvent, Category="MO|UI")
	void OnPossessionMenuToggled(bool bIsOpen);

	UFUNCTION(BlueprintCallable, Category="MO|UI")
	void TogglePossessionMenu();

	UFUNCTION(BlueprintCallable, Category="MO|UI")
	void SetGameOnlyInput();

	// CLIENT → SERVER: request to possess a GUID
	UFUNCTION(Server, Reliable)
	void Server_RequestPossessByGuid(const FGuid& Guid);

	// SERVER → CLIENT: refresh your possession UI/list
	UFUNCTION(Client, Reliable)
	void Client_RefreshPossessionList();

	UFUNCTION(Server, Reliable)
	void Server_RequestRescanPossession();
};
