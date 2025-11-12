#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI_SaveBrowser.generated.h"

class UScrollBox;

UCLASS()
class MO_1_API UUI_SaveBrowser : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintCallable, Category="MO|SaveBrowser")
	void RefreshList();

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UScrollBox> WorldsList;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MO|SaveBrowser")
	TSubclassOf<UUserWidget> WorldEntryClass; // set to BP_UI_WorldSaveEntry in BP
};
