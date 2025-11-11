#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MOPosessionSubsystem.generated.h"

class APawn;

USTRUCT(BlueprintType)
struct FMOPossessionCandidate
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FGuid Guid;
	UPROPERTY(BlueprintReadOnly) FText DisplayName;
	UPROPERTY(BlueprintReadOnly) TWeakObjectPtr<APawn> Pawn;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPossessionCandidatesChanged);

UCLASS()
class MO_1_API UMOPosessionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Clear and rescan the current level for pawns that have an Identity component
	UFUNCTION(BlueprintCallable, Category="MO|Possession")
	void DiscoverLevelPawns();

	// Query current candidates for UI
	UFUNCTION(BlueprintCallable, Category="MO|Possession")
	void GetCandidates(UPARAM(ref) TArray<FMOPossessionCandidate>& Out) const;

	// Possess by GUID
	UFUNCTION(BlueprintCallable, Category="MO|Possession")
	bool PossessByGuid(class APlayerController* PC, const FGuid& Guid);

	// Event so UI can refresh if you ever re-scan dynamically
	UPROPERTY(BlueprintAssignable, Category="MO|Possession")
	FOnPossessionCandidatesChanged OnCandidatesChanged;

private:
	// Internal registration helper
	void RegisterPawn(APawn* Pawn, const FGuid& Guid, const FText& DisplayName);

private:
	UPROPERTY() TMap<FGuid, TWeakObjectPtr<APawn>> GuidToPawn;
	UPROPERTY() TMap<FGuid, FText> GuidToName;
};
