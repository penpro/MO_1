// MOPlayerController.cpp

#include "MOPlayerController.h"
#include "MOCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "MOPersistenceSubsystem.h"

#include "MOPosessionSubsystem.h"  // server snapshot + possess by GUID
#include "MOHUD.h"                 // Master Menu HUD bridge
#include "MOUITypes.h"             // EMOMenuTab
#include "MOUIManagerSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "UI_Possession.h"         // fallback possession popup

// ============================================================================
// APlayerController
// ============================================================================

void AMOPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Install our Enhanced Input mapping context at runtime.
	// NOTE: If you plan to use multiple contexts (eg. vehicles/menus),
	// consider removing ClearAllMappings() and layering contexts by priority.
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

	// (Optional) Auto-open possession popup on first load in SP, useful for tests.
	/*if (IsLocalController())
	{
		FTimerHandle H;
		GetWorldTimerManager().SetTimer(H, this, &AMOPlayerController::TogglePossessionMenu, 0.05f, false);
	}*/
}

void AMOPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Bind Enhanced Input actions.
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// Movement / Camera / Interact
		if (IA_Move)     { EIC->BindAction(IA_Move,     ETriggerEvent::Triggered, this, &AMOPlayerController::OnMove); }
		if (IA_Look)     { EIC->BindAction(IA_Look,     ETriggerEvent::Triggered, this, &AMOPlayerController::OnLook); }
		if (IA_Interact) { EIC->BindAction(IA_Interact, ETriggerEvent::Started,   this, &AMOPlayerController::OnInteract); }

		// Possession (legacy popup or Master Menu Possession tab)
		if (IA_Possess)  { EIC->BindAction(IA_Possess,  ETriggerEvent::Started,   this, &AMOPlayerController::OnPossessAction); }

		// NEW — Master Menu and tab shortcuts
		if (IA_GameMenu) { EIC->BindAction(IA_GameMenu, ETriggerEvent::Started,   this, &AMOPlayerController::HandleGameMenuAction); }
		if (IA_Craft)    { EIC->BindAction(IA_Craft,    ETriggerEvent::Started,   this, &AMOPlayerController::HandleCraftAction); }
		if (IA_Skills)   { EIC->BindAction(IA_Skills,   ETriggerEvent::Started,   this, &AMOPlayerController::HandleSkillsAction); }
	}
}

// ============================================================================
// Movement / Camera / Interaction
// ============================================================================

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
	// Placeholder for interact. Promote to RPCs as needed for gameplay actions.
	UE_LOG(LogTemp, Verbose, TEXT("Interact pressed"));
}

// ============================================================================
// Master Menu / Possession actions
// ============================================================================

void AMOPlayerController::OnPossessAction(const FInputActionValue& /*Value*/)
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (auto* UIM = GI->GetSubsystem<UMOUIManagerSubsystem>())
		{
			UIM->ShowTab(EMOMenuTab::Possession);
			return;
		}
	}
	TogglePossessionMenu();
}

void AMOPlayerController::HandleGameMenuAction()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (auto* UIM = GI->GetSubsystem<UMOUIManagerSubsystem>())
		{
			// Toggle using the last remembered tab
			UIM->ToggleMenu();
			return;
		}
	}
	// Absolute fallback if subsystem missing
	TogglePossessionMenu();
}

void AMOPlayerController::HandlePossessionAction()
{
	// Kept for parity if bound elsewhere; prefer OnPossessAction for IA_Possess
	if (AMOHUD* H = GetHUD<AMOHUD>())
	{
		H->ShowMasterMenuTab(EMOMenuTab::Possession);
		return;
	}
	TogglePossessionMenu();
}

void AMOPlayerController::HandleCraftAction()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (auto* UIM = GI->GetSubsystem<UMOUIManagerSubsystem>())
		{
			UIM->ShowTab(EMOMenuTab::Crafting);
			return;
		}
	}
}

void AMOPlayerController::HandleSkillsAction()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (auto* UIM = GI->GetSubsystem<UMOUIManagerSubsystem>())
		{
			UIM->ShowTab(EMOMenuTab::Skills);
			return;
		}
	}
}

// ============================================================================
// Possession popup (fallback UI) — used when Master Menu/HUD is not in use
// ============================================================================

void AMOPlayerController::TogglePossessionMenu()
{
	// Never build UI for non-local controllers (e.g. server proxy of a client)
	if (!IsLocalController())
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("TogglePossessionMenu ignored: not local PC"));
		return;
	}

	// Lazily create the widget
	if (!PossessionMenu && PossessionMenuClass)
	{
		PossessionMenu = CreateWidget<UUserWidget>(this, PossessionMenuClass);
	}

	if (!PossessionMenu)
	{
		UE_LOG(LogTemp, Warning, TEXT("PossessionMenuClass is not set on BP_MOPlayerController"));
		return;
	}

	// Ask server for a fresh snapshot before showing
	if (!HasAuthority())   // client
	{
		Server_RequestRescanPossession();
	}
	else                   // listen server's local PC
	{
		if (UWorld* World = GetWorld())
		{
			if (auto* Subsys = World->GetSubsystem<UMOPosessionSubsystem>())
			{
				Subsys->DiscoverLevelPawns();
			}
		}
		Server_PushSnapshotToAll();
	}

	// Toggle visibility
	if (PossessionMenu->IsInViewport())
	{
		ClosePossessionMenu();
		return;
	}

	PossessionMenu->AddToViewport(PossessionMenuZOrder);
	RefreshPossessionMenuUI();

	// Switch to Game+UI input so mouse can interact with the widget
	SetShowMouseCursor(true);
	FInputModeGameAndUI Mode;
	Mode.SetHideCursorDuringCapture(false);
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	Mode.SetWidgetToFocus(PossessionMenu->TakeWidget());
	SetInputMode(Mode);

	OnPossessionMenuToggled(true);
}

void AMOPlayerController::RefreshPossessionMenuUI()
{
	if (UUI_Possession* Poss = Cast<UUI_Possession>(PossessionMenu))
	{
		Poss->RefreshList(); // reads from GetCachedCandidates()
	}
}

void AMOPlayerController::ClosePossessionMenu()
{
	if (!IsLocalController()) return;

	if (PossessionMenu && PossessionMenu->IsInViewport())
	{
		PossessionMenu->RemoveFromParent();
	}
	// Force fresh instance next open (avoids stale focus/state issues)
	PossessionMenu = nullptr;

	SetGameOnlyInput();
	OnPossessionMenuToggled(false);
}

void AMOPlayerController::SetGameOnlyInput()
{
	SetShowMouseCursor(false);
	FInputModeGameOnly Mode;
	SetInputMode(Mode);
}

// ============================================================================
// Possession replication & snapshot plumbing
// ============================================================================

void AMOPlayerController::Server_PushSnapshotToAll()
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (UMOPosessionSubsystem* Subsys = World->GetSubsystem<UMOPosessionSubsystem>())
	{
		TArray<FMOGuidName> Snapshot;
		Subsys->BuildFullSnapshot(Snapshot); // includes taken/free + owner names

		// Sort: free first, then alphabetically for stable UI
		Snapshot.Sort([](const FMOGuidName& A, const FMOGuidName& B)
		{
			if (A.bTaken != B.bTaken) return !A.bTaken && B.bTaken; // free first
			return A.DisplayName.ToString() < B.DisplayName.ToString();
		});

		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (AMOPlayerController* MOPC = Cast<AMOPlayerController>(It->Get()))
			{
				MOPC->Client_SetCandidateSnapshot(Snapshot);
			}
		}
	}
}

void AMOPlayerController::Client_SetCandidateSnapshot_Implementation(const TArray<FMOGuidName>& InList)
{
	CachedCandidates = InList;

	// If UI is open, refresh it to reflect the new snapshot
	if (PossessionMenu && PossessionMenu->IsInViewport())
	{
		if (UUI_Possession* Poss = Cast<UUI_Possession>(PossessionMenu))
		{
			Poss->RefreshList();
		}
	}
}

void AMOPlayerController::Client_PossessionResult_Implementation(bool bSuccess)
{
	if (!bSuccess)
	{
		// You can show a toast or sound here if you want
		return;
	}

	// Prefer the UI Manager which will tell the HUD to hide the menu and restore input
	if (UGameInstance* GI = GetGameInstance())
	{
		if (auto* UIM = GI->GetSubsystem<UMOUIManagerSubsystem>())
		{
			UIM->HideMenu();
			return;
		}
	}

	// Fallback to legacy popup if UI Manager is not present
	ClosePossessionMenu();
}

void AMOPlayerController::Client_RefreshPossessionList_Implementation()
{
	// Local convenience refresh (rarely used)
	if (UWorld* World = GetWorld())
	{
		if (UMOPosessionSubsystem* Subsys = World->GetSubsystem<UMOPosessionSubsystem>())
		{
			Subsys->DiscoverLevelPawns(); // local scan (mostly for single-player testing)
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
			Subsys->DiscoverLevelPawns(); // authoritative refresh
			Server_PushSnapshotToAll();   // then push to everyone
		}
	}
}

void AMOPlayerController::Server_RequestPossessByGuid_Implementation(const FGuid& Guid)
{
	UWorld* World = GetWorld();
	if (!World) { Client_PossessionResult(false); return; }

	if (UMOPosessionSubsystem* Subsys = World->GetSubsystem<UMOPosessionSubsystem>())
	{
		const bool bOk = Subsys->PossessByGuid(this, Guid);
		Client_PossessionResult(bOk);

		// If listen server's local PC, make the close explicit too
		if (bOk && HasAuthority() && IsLocalController())
		{
			if (UGameInstance* GI = GetGameInstance())
			{
				if (auto* UIM = GI->GetSubsystem<UMOUIManagerSubsystem>())
				{
					UIM->HideMenu();
				}
				else
				{
					ClosePossessionMenu(); // legacy fallback
				}
			}
		}

		// After any attempt, push updated snapshot to all
		Server_PushSnapshotToAll();
	}
}

void AMOPlayerController::Server_SaveWorld_Implementation(bool bAutosave, const FString& Label)
{
	if (!GetWorld() || GetWorld()->GetNetMode() == NM_Client) return;

	if (auto* PS = GetGameInstance()->GetSubsystem<UMOPersistenceSubsystem>())
	{
		const FGuid G = PS->GetOrCreateWorldGuid();
		const bool bOK = PS->SaveWorldNewSlot(G, bAutosave, Label);
		UE_LOG(LogTemp, Display, TEXT("[Server_SaveWorld] %s"), bOK ? TEXT("OK") : TEXT("FAILED"));
	}
}
