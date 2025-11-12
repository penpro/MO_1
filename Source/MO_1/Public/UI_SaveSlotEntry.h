#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MOPersistenceSubsystem.h"
#include "UI_SaveSlotEntry.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class MO_1_API UUI_SaveSlotEntry : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetupFromInfo(const FMOShortSaveInfo& InInfo);

protected:
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> LabelText;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> DetailText;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton>    LoadButton;

private:
	UFUNCTION() void OnLoadClicked();

	static FString FormatPlayTime(float Seconds);
	static FString FormatTimestamp(const FDateTime& TS);

	FMOShortSaveInfo Info;
};
