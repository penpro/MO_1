#include "UI_MasterMenu.h"
#include "Components/SizeBox.h"
#include "Components/Button.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MOPersistenceSubsystem.h" // for SaveSettings now; expand later

void UUI_MasterMenu::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (OptionsButton)       { OptionsButton->OnClicked.AddDynamic(this, &UUI_MasterMenu::OnOptionsClicked); }
    if (PossessionButton)    { PossessionButton->OnClicked.AddDynamic(this, &UUI_MasterMenu::OnPossessionClicked); }
    if (CraftingButton)      { CraftingButton->OnClicked.AddDynamic(this, &UUI_MasterMenu::OnCraftingClicked); }
    if (SkillsButton)        { SkillsButton->OnClicked.AddDynamic(this, &UUI_MasterMenu::OnSkillsClicked); }
    if (WikiButton)          { WikiButton->OnClicked.AddDynamic(this, &UUI_MasterMenu::OnWikiClicked); }

    if (SaveGameButton)      { SaveGameButton->OnClicked.AddDynamic(this, &UUI_MasterMenu::OnSaveClicked); }
    if (SaveAndExitButton)   { SaveAndExitButton->OnClicked.AddDynamic(this, &UUI_MasterMenu::OnSaveAndExitClicked); }
    if (ExitToMainMenuButton){ ExitToMainMenuButton->OnClicked.AddDynamic(this, &UUI_MasterMenu::OnExitToMainMenuClicked); }
    if (ExitGameButton)      { ExitGameButton->OnClicked.AddDynamic(this, &UUI_MasterMenu::OnExitGameClicked); }
}

void UUI_MasterMenu::ShowMenu(EMOMenuTab TabToFocus)
{
    SetVisibility(ESlateVisibility::Visible);
    FocusTab(TabToFocus == EMOMenuTab::None ? LastTab : TabToFocus);
}

void UUI_MasterMenu::ShowMenuLast()
{
    ShowMenu(LastTab);
}

void UUI_MasterMenu::HideMenu()
{
    SetVisibility(ESlateVisibility::Collapsed);
}

void UUI_MasterMenu::OpenPossessionTab()
{
    ShowMenu(EMOMenuTab::Possession);
}

void UUI_MasterMenu::OnOptionsClicked()    { FocusTab(EMOMenuTab::Options); }
void UUI_MasterMenu::OnPossessionClicked() { FocusTab(EMOMenuTab::Possession); }
void UUI_MasterMenu::OnCraftingClicked()   { FocusTab(EMOMenuTab::Crafting); }
void UUI_MasterMenu::OnSkillsClicked()     { FocusTab(EMOMenuTab::Skills); }
void UUI_MasterMenu::OnWikiClicked()       { FocusTab(EMOMenuTab::Wiki); }

void UUI_MasterMenu::OnSaveClicked()
{
    // Minimal: save settings for now. Later: call SaveAll(WorldGuid).
    if (UMOPersistenceSubsystem* P = GetGameInstance()->GetSubsystem<UMOPersistenceSubsystem>())
    {
        P->SaveSettings();
    }
    UE_LOG(LogTemp, Display, TEXT("Save requested"));
}

void UUI_MasterMenu::OnSaveAndExitClicked()
{
    OnSaveClicked();
    OnExitToMainMenuClicked();
}

void UUI_MasterMenu::OnExitToMainMenuClicked()
{
    if (MainMenuMapName.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("MainMenuMapName not set"));
        return;
    }
    UGameplayStatics::OpenLevel(this, MainMenuMapName);
}

void UUI_MasterMenu::OnExitGameClicked()
{
    APlayerController* PC = GetOwningPlayer();
    UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, false);
}

void UUI_MasterMenu::SetFocusPanelWidget(UUserWidget* NewContent)
{
    if (!FocusPanel) return;

    if (CurrentPanel && CurrentPanel->IsInViewport())
    {
        CurrentPanel->RemoveFromParent();
    }
    CurrentPanel = NewContent;

    if (CurrentPanel)
    {
        FocusPanel->SetContent(CurrentPanel);
    }
    else
    {
        FocusPanel->SetContent(nullptr);
    }
}

void UUI_MasterMenu::FocusTab(EMOMenuTab Tab)
{
    if (!FocusPanel)
    {
        UE_LOG(LogTemp, Warning, TEXT("FocusPanel is null on MasterMenu"));
        return;
    }

    TSubclassOf<UUserWidget> ClassToCreate = nullptr;
    switch (Tab)
    {
        case EMOMenuTab::Options:    ClassToCreate = OptionsPanelClass;    break;
        case EMOMenuTab::Possession: ClassToCreate = PossessionPanelClass; break;
        case EMOMenuTab::Crafting:   ClassToCreate = CraftingPanelClass;   break;
        case EMOMenuTab::Skills:     ClassToCreate = SkillsPanelClass;     break;
        case EMOMenuTab::Wiki:       ClassToCreate = WikiPanelClass;       break;
        default: break;
    }

    if (!ClassToCreate)
    {
        UE_LOG(LogTemp, Warning, TEXT("Tab %d has no panel class set"), (int32)Tab);
        return;
    }

    if (UUserWidget* Panel = CreateWidget<UUserWidget>(GetWorld(), ClassToCreate))
    {
        SetFocusPanelWidget(Panel);
        CurrentTab = Tab;
        LastTab    = Tab;
    }
}
