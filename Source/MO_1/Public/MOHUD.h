#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MO_1/MOUITypes.h"
#include "MOHUD.generated.h"

class UUI_MasterMenu;

UCLASS(BlueprintType, Blueprintable)
class MO_1_API AMOHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	// Toggle menu with last focused tab
	UFUNCTION(BlueprintCallable, Category="MO|HUD")
	void ToggleMasterMenu();

	// Show specific tab (e.g., IA_Possess)
	UFUNCTION(BlueprintCallable, Category="MO|HUD")
	void ShowMasterMenuTab(EMOMenuTab Tab);

	// Hide menu
	UFUNCTION(BlueprintCallable, Category="MO|HUD")
	void HideMasterMenu();

	// Expose pointer for debugging
	UFUNCTION(BlueprintCallable, Category="MO|HUD")
	UUI_MasterMenu* GetMasterMenu() const { return MasterMenu; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MO|HUD")
	TSubclassOf<UUI_MasterMenu> MasterMenuClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MO|HUD")
	bool bOpenMasterMenuAtBeginPlay = true;

private:
	void SetUIOnlyInput(UUserWidget* FocusWidget);
	void SetGameOnlyInput();

private:
	UPROPERTY() TObjectPtr<UUI_MasterMenu> MasterMenu = nullptr;
};
