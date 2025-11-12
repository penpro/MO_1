#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MOPersistenceSubsystem.h" // for FMOWorldSaves
#include "UI_WorldSaveEntry.generated.h"

class UButton;
class UTextBlock;
class UScrollBox;

UCLASS()
class MO_1_API UUI_WorldSaveEntry : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeOnInitialized() override;

	void SetupFromData(const FMOWorldSaves& InData);

protected:
	// Header
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton>   HeaderButton;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock>WorldTitle;

	// Collapsible list of saves
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UScrollBox> SavesList;

	// Entry widget class for each save under this world
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MO|SaveBrowser")
	TSubclassOf<UUserWidget> SaveEntryClass; // set to BP_UI_SaveSlotEntry

private:
	UFUNCTION() void OnHeaderClicked();

	void RebuildChildren();

	FMOWorldSaves Data;
	bool bExpanded = false;
};
