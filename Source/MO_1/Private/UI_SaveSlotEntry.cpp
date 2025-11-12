#include "UI_SaveSlotEntry.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "MOPersistenceSubsystem.h"

void UUI_SaveSlotEntry::SetupFromInfo(const FMOShortSaveInfo& InInfo)
{
	Info = InInfo;

	const FString Label = Info.UserLabel.IsEmpty()
		? FString::Printf(TEXT("%s Save"), Info.bAutosave ? TEXT("Auto") : TEXT("Manual"))
		: Info.UserLabel;

	const FString Detail = FString::Printf(TEXT("%s    •    %s"),
		*FormatPlayTime(Info.PlayTimeSeconds),
		*FormatTimestamp(Info.Timestamp));

	if (LabelText)  LabelText->SetText(FText::FromString(Label));
	if (DetailText) DetailText->SetText(FText::FromString(Detail));

	if (LoadButton)
	{
		LoadButton->OnClicked.Clear();
		LoadButton->OnClicked.AddDynamic(this, &UUI_SaveSlotEntry::OnLoadClicked);
	}
}

void UUI_SaveSlotEntry::OnLoadClicked()
{
	if (UWorld* W = GetWorld())
	{
		if (auto* PS = W->GetGameInstance()->GetSubsystem<UMOPersistenceSubsystem>())
		{
			// Debounce to avoid double clicks
			if (LoadButton) LoadButton->SetIsEnabled(false);
			PS->RequestLoadGameBySlot(Info.SlotName);
		}
	}
}

FString UUI_SaveSlotEntry::FormatPlayTime(float Seconds)
{
	const int32 S = FMath::FloorToInt(Seconds);
	const int32 H = S / 3600;
	const int32 M = (S % 3600) / 60;
	const int32 SS= S % 60;
	return FString::Printf(TEXT("%02d:%02d:%02d"), H, M, SS);
}

FString UUI_SaveSlotEntry::FormatTimestamp(const FDateTime& TS)
{
	return TS.ToString(TEXT("%Y-%m-%d  %H:%M"));
}
