#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MO_1/MOUITypes.h"
#include "MOUIManagerSubsystem.generated.h"

class AMOHUD;
class UUI_MasterMenu;

UCLASS()
class MO_1_API UMOUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Called from HUD::BeginPlay when the local HUD is ready
	UFUNCTION(BlueprintCallable, Category="MO|UI")
	void RegisterLocalHUD(AMOHUD* InHUD);

	// Input-facing API
	UFUNCTION(BlueprintCallable, Category="MO|UI")
	void ToggleMenu();

	UFUNCTION(BlueprintCallable, Category="MO|UI")
	void ShowTab(EMOMenuTab Tab);

	UFUNCTION(BlueprintCallable, Category="MO|UI")
	void HideMenu();

	// Optional: open menu at begin play focused on a tab (default Possession)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MO|UI")
	bool bOpenMenuOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MO|UI")
	EMOMenuTab InitialTab = EMOMenuTab::Possession;

	// Remember last tab for IA_GameMenu toggle
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MO|UI")
	EMOMenuTab LastTab = EMOMenuTab::Options;

private:
	TWeakObjectPtr<AMOHUD> LocalHUD;
};
