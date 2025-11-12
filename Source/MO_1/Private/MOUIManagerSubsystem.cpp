#include "MOUIManagerSubsystem.h"
#include "MOHUD.h"
#include "UI_MasterMenu.h"

void UMOUIManagerSubsystem::RegisterLocalHUD(AMOHUD* InHUD)
{
	if (!InHUD) return;
	LocalHUD = InHUD;

	if (bOpenMenuOnBeginPlay)
	{
		ShowTab(InitialTab);
	}
}

void UMOUIManagerSubsystem::ToggleMenu()
{
	if (!LocalHUD.IsValid()) return;
	LocalHUD->ToggleMasterMenu();
}

void UMOUIManagerSubsystem::ShowTab(EMOMenuTab Tab)
{
	if (!LocalHUD.IsValid()) return;
	LocalHUD->ShowMasterMenuTab(Tab);
	LastTab = Tab;
}

void UMOUIManagerSubsystem::HideMenu()
{
	if (!LocalHUD.IsValid()) return;
	LocalHUD->HideMasterMenu();
}
