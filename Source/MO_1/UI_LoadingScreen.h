#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI_LoadingScreen.generated.h"

class UScrollBox;
class USizeBox;
class UButton;

UCLASS()
class MO_1_API UUI_LoadingScreen : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeOnInitialized() override;

    // Exposed so you can bind buttons in BP if you prefer
    UFUNCTION(BlueprintCallable, Category="MO|Menu") void ShowStartNewGame();
    UFUNCTION(BlueprintCallable, Category="MO|Menu") void ShowLoadGame();
    UFUNCTION(BlueprintCallable, Category="MO|Menu") void ShowOptions();
    UFUNCTION(BlueprintCallable, Category="MO|Menu") void ExitGame();

protected:
    // Bind these in your BP child
    UPROPERTY(meta=(BindWidget))          TObjectPtr<UScrollBox> MenuList;
    UPROPERTY(meta=(BindWidget))          TObjectPtr<USizeBox>   FocusPanel;

    // Optional direct button bindings if you place the buttons inside the ScrollBox
    UPROPERTY(meta=(BindWidgetOptional))  TObjectPtr<UButton> StartNewButton;
    UPROPERTY(meta=(BindWidgetOptional))  TObjectPtr<UButton> LoadGameButton;
    UPROPERTY(meta=(BindWidgetOptional))  TObjectPtr<UButton> OptionsButton;
    UPROPERTY(meta=(BindWidgetOptional))  TObjectPtr<UButton> ExitButton;

    // Panels you will assign in the BP child
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MO|Menu")
    TSubclassOf<UUserWidget> SaveGamePanelClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MO|Menu")
    TSubclassOf<UUserWidget> OptionsPanelClass;

    // Level to open for a new game
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MO|Menu")
    FName TestLevelName = TEXT("TestLevel");

private:
    // Native handlers hooked to buttons
    UFUNCTION() void HandleStartNewClicked();
    UFUNCTION() void HandleLoadClicked();
    UFUNCTION() void HandleOptionsClicked();
    UFUNCTION() void HandleExitClicked();

    // Helper to swap the FocusPanel content
    void SetFocusPanelContent(UUserWidget* NewContent);

    UPROPERTY() TObjectPtr<UUserWidget> CurrentPanel;
};
