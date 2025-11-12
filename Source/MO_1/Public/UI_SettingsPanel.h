#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI_SettingsPanel.generated.h"

class USlider;
class UCheckBox;
class UButton;
class UComboBoxString; // NEW

UCLASS()
class MO_1_API UUI_SettingsPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

protected:
	// Bind these in your BP child
	UPROPERTY(meta=(BindWidget))          TObjectPtr<USlider>         MasterVolumeSlider;
	UPROPERTY(meta=(BindWidget))          TObjectPtr<UComboBoxString> ResolutionCombo; // NEW
	UPROPERTY(meta=(BindWidget))          TObjectPtr<UCheckBox>       FullscreenCheck;

	UPROPERTY(meta=(BindWidget))          TObjectPtr<UButton>         SaveButton;
	UPROPERTY(meta=(BindWidget))          TObjectPtr<UButton>         ResetButton;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MO|Settings")
	bool bApplyOnSave = false;

private:
	UFUNCTION() void HandleSaveClicked();
	UFUNCTION() void HandleResetClicked();
	UFUNCTION() void HandleResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType); // NEW

	void PopulateResolutionOptions(); // NEW
	void LoadSettingsIntoUI();
	bool SaveSettingsFromUI();
	void ResetDefaultsIntoUI();

	// Helpers
	static FString FormatRes(int32 X, int32 Y);           // NEW
	static bool ParseRes(const FString& S, int32& OutX, int32& OutY); // NEW
};
