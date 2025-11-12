#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MO_1/MOUITypes.h"
#include "UI_MasterMenu.generated.h"

class USizeBox;
class UButton;
class UUserWidget;

UCLASS()
class MO_1_API UUI_MasterMenu : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeOnInitialized() override;

    // Show menu and focus a specific tab
    UFUNCTION(BlueprintCallable, Category="MO|Menu")
    void ShowMenu(EMOMenuTab TabToFocus);

    // Show menu and use the last tab we showed
    UFUNCTION(BlueprintCallable, Category="MO|Menu")
    void ShowMenuLast();

    // Hide menu
    UFUNCTION(BlueprintCallable, Category="MO|Menu")
    void HideMenu();

    // Expose a direct call for IA_Possess etc.
    UFUNCTION(BlueprintCallable, Category="MO|Menu")
    void OpenPossessionTab();

    // Returns currently focused tab
    UFUNCTION(BlueprintCallable, Category="MO|Menu")
    EMOMenuTab GetCurrentTab() const { return CurrentTab; }

protected:
    // Focus container for sub-widgets (bind this to your SizeBox "FocusPanel")
    UPROPERTY(meta=(BindWidget))
    TObjectPtr<USizeBox> FocusPanel;

    // Sidebar buttons (bind if you place them; optional is fine)
    UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> OptionsButton;
    UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> PossessionButton;
    UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> CraftingButton;
    UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> SkillsButton;
    UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> WikiButton;

    // Bottom action buttons
    UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> SaveGameButton;
    UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> SaveAndExitButton;
    UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> ExitToMainMenuButton;
    UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> ExitGameButton;
    UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> LoadGameButton;

    // Panel classes to spawn in FocusPanel
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MO|Menu")
    TSubclassOf<UUserWidget> OptionsPanelClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MO|Menu")
    TSubclassOf<UUserWidget> PossessionPanelClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MO|Menu")
    TSubclassOf<UUserWidget> CraftingPanelClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MO|Menu")
    TSubclassOf<UUserWidget> SkillsPanelClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MO|Menu")
    TSubclassOf<UUserWidget> WikiPanelClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MO|Menu")
    TSubclassOf<UUserWidget> LoadGamePanelClass;

    // Map name of your front end menu (LoadingScreen)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MO|Menu")
    FName MainMenuMapName = TEXT("LoadingScreen");

private:
    // Button handlers
    UFUNCTION() void OnOptionsClicked();
    UFUNCTION() void OnPossessionClicked();
    UFUNCTION() void OnCraftingClicked();
    UFUNCTION() void OnSkillsClicked();
    UFUNCTION() void OnWikiClicked();
    UFUNCTION() void OnLoadGameClicked();

    UFUNCTION() void OnSaveClicked();
    UFUNCTION() void OnSaveAndExitClicked();
    UFUNCTION() void OnExitToMainMenuClicked();
    UFUNCTION() void OnExitGameClicked();

    void SetFocusPanelWidget(UUserWidget* NewContent);
    void FocusTab(EMOMenuTab Tab);

private:
    UPROPERTY() TObjectPtr<UUserWidget> CurrentPanel;
    UPROPERTY() EMOMenuTab CurrentTab = EMOMenuTab::None;
    UPROPERTY() EMOMenuTab LastTab    = EMOMenuTab::Options;
};
