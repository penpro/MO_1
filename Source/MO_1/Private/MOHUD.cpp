#include "MOHUD.h"
#include "UI_MasterMenu.h"
#include "MOUIManagerSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

void AMOHUD::BeginPlay()
{
	Super::BeginPlay();

	if (MasterMenuClass && PlayerOwner && PlayerOwner->IsLocalController())
	{
		MasterMenu = CreateWidget<UUI_MasterMenu>(PlayerOwner, MasterMenuClass);
		if (MasterMenu)
		{
			MasterMenu->AddToViewport(50);
			MasterMenu->SetVisibility(ESlateVisibility::Collapsed);
		}

		// Tell the UI Manager that this HUD is ready. It decides whether to open a menu now.
		if (UGameInstance* GI = GetGameInstance())
		{
			if (auto* UIM = GI->GetSubsystem<UMOUIManagerSubsystem>())
			{
				UIM->RegisterLocalHUD(this);
			}
		}
	}
}

void AMOHUD::ToggleMasterMenu()
{
	if (!MasterMenu || !PlayerOwner) return;

	const bool bOpen = MasterMenu->GetVisibility() != ESlateVisibility::Collapsed;
	if (bOpen)
	{
		MasterMenu->HideMenu();
		SetGameOnlyInput();
	}
	else
	{
		MasterMenu->ShowMenuLast();
		SetUIOnlyInput(MasterMenu);
	}
}

void AMOHUD::ShowMasterMenuTab(EMOMenuTab Tab)
{
	if (!MasterMenu || !PlayerOwner) return;

	MasterMenu->ShowMenu(Tab);
	SetUIOnlyInput(MasterMenu);
}

void AMOHUD::HideMasterMenu()
{
	if (!MasterMenu || !PlayerOwner) return;

	MasterMenu->HideMenu();
	SetGameOnlyInput();
}

void AMOHUD::SetUIOnlyInput(UUserWidget* FocusWidget)
{
	if (!PlayerOwner) return;
	PlayerOwner->SetShowMouseCursor(true);
	FInputModeGameAndUI Mode;
	Mode.SetHideCursorDuringCapture(false);
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	if (FocusWidget) Mode.SetWidgetToFocus(FocusWidget->TakeWidget());
	PlayerOwner->SetInputMode(Mode);
}

void AMOHUD::SetGameOnlyInput()
{
	if (!PlayerOwner) return;
	PlayerOwner->SetShowMouseCursor(false);
	FInputModeGameOnly Mode;
	PlayerOwner->SetInputMode(Mode);
}
