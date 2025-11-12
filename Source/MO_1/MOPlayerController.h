// MOPlayerController.h
// Primary player controller for MO_1.
// - Owns Enhanced Input bindings
// - Hosts possession menu fallback (until Master Menu/HUD is present)
// - Exposes menu/tab actions for IA_GameMenu, IA_Possess, IA_Craft, IA_Skills

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "MOPossessionTypes.h"
#include "GameFramework/PlayerController.h"
#include "MOUITypes.h"                // EMOMenuTab (Master Menu tabs)
#include "MOPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UUserWidget;
class UUI_Possession;

UCLASS()
class MO_1_API AMOPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMOPlayerController() = default;

protected:
	// -------- APlayerController --------
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	/** Assign in BP_MOPlayerController (your child BP) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MO|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** Movement / camera / interaction */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MO|Input")
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MO|Input")
	TObjectPtr<UInputAction> IA_Look;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MO|Input")
	TObjectPtr<UInputAction> IA_Interact;

	/** Possession key (still supported even when Master Menu exists) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MO|Input")
	TObjectPtr<UInputAction> IA_Possess;

	/** NEW — Master menu toggle (opens last tab) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MO|Input")
	TObjectPtr<UInputAction> IA_GameMenu;

	/** NEW — Jump straight to Crafting tab */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MO|Input")
	TObjectPtr<UInputAction> IA_Craft;

	/** NEW — Jump straight to Skills tab */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MO|Input")
	TObjectPtr<UInputAction> IA_Skills;

	// -------- Simple possession popup (fallback UI) --------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MO|UI")
	TSubclassOf<UUserWidget> PossessionMenuClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MO|UI")
	int32 PossessionMenuZOrder = 1000;

	/** Bound to IA_GameMenu — toggles Master Menu (or falls back to possession popup) */
	UFUNCTION(BlueprintCallable, Category="MO|Menu")
	void HandleGameMenuAction();

	/** Bound to IA_Possess — opens Possession tab (or falls back to possession popup) */
	UFUNCTION(BlueprintCallable, Category="MO|Menu")
	void HandlePossessionAction();

	/** NEW — Bound to IA_Craft — opens Crafting tab in Master Menu */
	UFUNCTION(BlueprintCallable, Category="MO|Menu")
	void HandleCraftAction();

	/** NEW — Bound to IA_Skills — opens Skills tab in Master Menu */
	UFUNCTION(BlueprintCallable, Category="MO|Menu")
	void HandleSkillsAction();

private:
	// -------- Enhanced Input raw handlers --------
	void OnMove(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void OnInteract(const FInputActionValue& Value);
	void OnPossessAction(const FInputActionValue& Value);

	// Possession popup state (fallback UI)
	UPROPERTY() TObjectPtr<UUserWidget> PossessionMenu = nullptr;

	void RefreshPossessionMenuUI();
	void ClosePossessionMenu();

	// Server helper to broadcast latest snapshot to all players (not a UFUNCTION on purpose)
	void Server_PushSnapshotToAll();

	// Cached candidate list supplied by the server (consumed by WBP_Possession)
	UPROPERTY() TArray<FMOGuidName> CachedCandidates;

public:
	// -------- Possession data flow --------

	/** UI reads this to populate entries from the latest server snapshot */
	UFUNCTION(BlueprintCallable, Category="MO|Possession")
	void GetCachedCandidates(TArray<FMOGuidName>& Out) const { Out = CachedCandidates; }

	/** Server → Client: set snapshot and nudge the open menu to refresh */
	UFUNCTION(Client, Reliable)
	void Client_SetCandidateSnapshot(const TArray<FMOGuidName>& InList);

	/** Server → Client: result of a possess attempt (so client knows when to close) */
	UFUNCTION(Client, Reliable)
	void Client_PossessionResult(bool bSuccess);

	/** BP hook for focus/animation when the possession popup opens/closes */
	UFUNCTION(BlueprintImplementableEvent, Category="MO|UI")
	void OnPossessionMenuToggled(bool bIsOpen);

	/** Fallback possession popup toggle */
	UFUNCTION(BlueprintCallable, Category="MO|UI")
	void TogglePossessionMenu();

	/** Return input to game-only after closing menus */
	UFUNCTION(BlueprintCallable, Category="MO|UI")
	void SetGameOnlyInput();

	/** Client → Server: ask to possess a pawn by GUID */
	UFUNCTION(Server, Reliable)
	void Server_RequestPossessByGuid(const FGuid& Guid);

	/** Server → Client: request a local UI refresh (used rarely) */
	UFUNCTION(Client, Reliable)
	void Client_RefreshPossessionList();

	/** Client → Server: ask server to rescan and push a fresh snapshot */
	UFUNCTION(Server, Reliable)
	void Server_RequestRescanPossession();

	UFUNCTION(Server, Reliable)
	void Server_SaveWorld(bool bAutosave, const FString& Label = TEXT(""));
};
