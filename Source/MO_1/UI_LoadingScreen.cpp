#include "UI_LoadingScreen.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UUI_LoadingScreen::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (StartNewButton)
    {
        StartNewButton->OnClicked.Clear();
        StartNewButton->OnClicked.AddDynamic(this, &UUI_LoadingScreen::HandleStartNewClicked);
    }
    if (LoadGameButton)
    {
        LoadGameButton->OnClicked.Clear();
        LoadGameButton->OnClicked.AddDynamic(this, &UUI_LoadingScreen::HandleLoadClicked);
    }
    if (OptionsButton)
    {
        OptionsButton->OnClicked.Clear();
        OptionsButton->OnClicked.AddDynamic(this, &UUI_LoadingScreen::HandleOptionsClicked);
    }
    if (ExitButton)
    {
        ExitButton->OnClicked.Clear();
        ExitButton->OnClicked.AddDynamic(this, &UUI_LoadingScreen::HandleExitClicked);
    }
}

void UUI_LoadingScreen::HandleStartNewClicked() { ShowStartNewGame(); }
void UUI_LoadingScreen::HandleLoadClicked()     { ShowLoadGame();     }
void UUI_LoadingScreen::HandleOptionsClicked()  { ShowOptions();      }
void UUI_LoadingScreen::HandleExitClicked()     { ExitGame();         }

void UUI_LoadingScreen::ShowStartNewGame()
{
    if (TestLevelName.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("TestLevelName not set on LoadingScreen widget"));
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("Starting new game"));
    UGameplayStatics::OpenLevel(this, TestLevelName);
}

void UUI_LoadingScreen::ShowLoadGame()
{
    if (!SaveGamePanelClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("SaveGamePanelClass not set on LoadingScreen widget"));
        return;
    }
    UUserWidget* Panel = CreateWidget<UUserWidget>(GetWorld(), SaveGamePanelClass);
    SetFocusPanelContent(Panel);
}

void UUI_LoadingScreen::ShowOptions()
{
    if (!OptionsPanelClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("OptionsPanelClass not set on LoadingScreen widget"));
        return;
    }
    UUserWidget* Panel = CreateWidget<UUserWidget>(GetWorld(), OptionsPanelClass);
    SetFocusPanelContent(Panel);
}

void UUI_LoadingScreen::ExitGame()
{
    APlayerController* PC = GetOwningPlayer();
    UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, false);
}

void UUI_LoadingScreen::SetFocusPanelContent(UUserWidget* NewContent)
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
