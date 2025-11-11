#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI_Possession.generated.h"

class UScrollBox;
class UUI_PossessionEntry;
class UButton;

UCLASS()
class MO_1_API UUI_Possession : public UUserWidget
{
	GENERATED_BODY()

public:
	// Rebuild the list from the subsystem scan
	UFUNCTION(BlueprintCallable, Category="MO|Possession")
	void RefreshList();

	UFUNCTION(BlueprintCallable, Category="MO|Possession")
	void RequestRefresh();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;
	
	// Designer widgets: create a ScrollBox named "List"
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UScrollBox> List;

	// Set this to your BP widget class that extends UUI_PossessionEntry
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MO|Possession")
	TSubclassOf<UUI_PossessionEntry> EntryClass;

	// NEW: optional button in the widget (name it exactly "RefreshButton" in UMG)
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> RefreshButton;

private:
	UFUNCTION() void OnCandidatesChanged();
	UFUNCTION() void OnRefreshClicked();
	
	UPROPERTY() TObjectPtr<class UMOPosessionSubsystem> CachedSubsystem;

	int32 RefreshRetryCount = 0;
	FTimerHandle RefreshRetryHandle;
};
