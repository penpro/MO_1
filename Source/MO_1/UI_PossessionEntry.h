#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI_PossessionEntry.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class MO_1_API UUI_PossessionEntry : public UUserWidget
{
	GENERATED_BODY()

public:
	// Initialize entry with data
	UFUNCTION(BlueprintCallable, Category="MO|Possession")
	void SetupEntry(const FGuid& InGuid, const FText& InDisplayName);

protected:
	virtual void NativeOnInitialized() override;

	UFUNCTION()
	void HandlePossessClicked();

protected:
	// Bind these in the UMG designer by exact names
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> DisplayNameText;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> PossessButton;

private:
	FGuid Guid;
};
