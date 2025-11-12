#include "UI_SettingsPanel.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Components/Button.h"
#include "Types/SlateEnums.h"   
#include "Components/ComboBoxString.h" // NEW
#include "Kismet/GameplayStatics.h"
#include "MOPersistenceSubsystem.h"
#include "MOSettingsSaveGame.h"

void UUI_SettingsPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (SaveButton)
	{
		SaveButton->OnClicked.Clear();
		SaveButton->OnClicked.AddDynamic(this, &UUI_SettingsPanel::HandleSaveClicked);
	}
	if (ResetButton)
	{
		ResetButton->OnClicked.Clear();
		ResetButton->OnClicked.AddDynamic(this, &UUI_SettingsPanel::HandleResetClicked);
	}

	PopulateResolutionOptions(); // build default list

	if (ResolutionCombo)
	{
		ResolutionCombo->OnSelectionChanged.Clear();
		ResolutionCombo->OnSelectionChanged.AddDynamic(this, &UUI_SettingsPanel::HandleResolutionChanged);
	}

	LoadSettingsIntoUI();
}

void UUI_SettingsPanel::PopulateResolutionOptions()
{
	if (!ResolutionCombo) return;

	ResolutionCombo->ClearOptions();

	// Common 16:9 and a common ultrawide
	const TCHAR* Presets[] = {
		TEXT("1280 x 720"),
		TEXT("1600 x 900"),
		TEXT("1920 x 1080"),
		TEXT("2560 x 1440"),
		TEXT("3440 x 1440"),
		TEXT("3840 x 2160")
	};
	for (const TCHAR* Opt : Presets)
	{
		ResolutionCombo->AddOption(Opt);
	}

	// Default selection
	ResolutionCombo->SetSelectedOption(TEXT("1920 x 1080"));
}

FString UUI_SettingsPanel::FormatRes(int32 X, int32 Y)
{
	return FString::Printf(TEXT("%d x %d"), X, Y);
}

bool UUI_SettingsPanel::ParseRes(const FString& S, int32& OutX, int32& OutY)
{
	OutX = OutY = 0;
	FString L = S;
	L.ReplaceInline(TEXT(" "), TEXT(""));
	TArray<FString> Parts;
	L.ParseIntoArray(Parts, TEXT("x"), true);
	if (Parts.Num() != 2) return false;
	return LexTryParseString<int32>(OutX, *Parts[0]) && LexTryParseString<int32>(OutY, *Parts[1]);
}

void UUI_SettingsPanel::HandleResolutionChanged(FString SelectedItem, ESelectInfo::Type /*SelectionType*/)
{
	// No live apply here, but you could hook GameUserSettings if bApplyOnSave is true
}

void UUI_SettingsPanel::LoadSettingsIntoUI()
{
	const FString SaveSlot = UMOPersistenceSubsystem::SettingsSlot();

	UMOSettingsSaveGame* SG = nullptr;
	if (UGameplayStatics::DoesSaveGameExist(SaveSlot, 0))
	{
		SG = Cast<UMOSettingsSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlot, 0));
	}
	if (!SG)
	{
		// No file yet: use class defaults
		SG = Cast<UMOSettingsSaveGame>(UGameplayStatics::CreateSaveGameObject(UMOSettingsSaveGame::StaticClass()));
	}
	if (!SG) return;

	if (MasterVolumeSlider) MasterVolumeSlider->SetValue(FMath::Clamp(SG->MasterVolume, 0.f, 1.f));
	if (FullscreenCheck)    FullscreenCheck->SetIsChecked(SG->bFullscreen);

	// Select matching resolution or add a custom one if missing
	if (ResolutionCombo)
	{
		const FString Target = FormatRes(SG->ResolutionX, SG->ResolutionY);

		auto FindIndex = [](UComboBoxString* Combo, const FString& Opt)->int32
		{
			const int32 Count = Combo->GetOptionCount();
			for (int32 i = 0; i < Count; ++i)
			{
				if (Combo->GetOptionAtIndex(i).Equals(Opt, ESearchCase::IgnoreCase))
				{
					return i;
				}
			}
			return INDEX_NONE;
		};

		const int32 Index = FindIndex(ResolutionCombo, Target);
		if (Index == INDEX_NONE)
		{
			ResolutionCombo->AddOption(Target);
			ResolutionCombo->RefreshOptions(); // ensure UI updates immediately
		}

		ResolutionCombo->SetSelectedOption(Target);
	}
}

bool UUI_SettingsPanel::SaveSettingsFromUI()
{
	const FString SaveSlot = UMOPersistenceSubsystem::SettingsSlot();

	UMOSettingsSaveGame* SG = Cast<UMOSettingsSaveGame>(UGameplayStatics::CreateSaveGameObject(UMOSettingsSaveGame::StaticClass()));
	if (!SG) return false;

	// Read UI
	const float Vol = MasterVolumeSlider ? MasterVolumeSlider->GetValue() : 1.0f;

	int32 RX = 1920, RY = 1080;
	if (ResolutionCombo)
	{
		const FString Sel = ResolutionCombo->GetSelectedOption();
		ParseRes(Sel, RX, RY);
	}
	const bool bFS = FullscreenCheck ? FullscreenCheck->IsChecked() : true;

	// Fill save object
	SG->MasterVolume = FMath::Clamp(Vol, 0.f, 1.f);
	SG->ResolutionX  = FMath::Max(320, RX);
	SG->ResolutionY  = FMath::Max(240, RY);
	SG->bFullscreen  = bFS;

	const bool bOk = UGameplayStatics::SaveGameToSlot(SG, SaveSlot, 0);

	if (bOk && bApplyOnSave)
	{
		// Hook GameUserSettings here if you want instant apply later
	}

	return bOk;
}

void UUI_SettingsPanel::ResetDefaultsIntoUI()
{
	if (MasterVolumeSlider) MasterVolumeSlider->SetValue(1.0f);
	if (ResolutionCombo)    ResolutionCombo->SetSelectedOption(TEXT("1920 x 1080"));
	if (FullscreenCheck)    FullscreenCheck->SetIsChecked(true);
}

void UUI_SettingsPanel::HandleSaveClicked()
{
	const bool bOk = SaveSettingsFromUI();
	UE_LOG(LogTemp, Display, TEXT("Settings save %s"), bOk ? TEXT("OK") : TEXT("FAILED"));
}

void UUI_SettingsPanel::HandleResetClicked()
{
	ResetDefaultsIntoUI();
}
