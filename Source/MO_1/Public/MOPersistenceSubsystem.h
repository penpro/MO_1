#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MOPersistenceSubsystem.generated.h"

class UMOSettingsSaveGame;
class UMOWorldSaveGame;
class UMOPawnSaveGame;
class UMOTerrainSaveGame;
class UMOIdentityComponent;

USTRUCT(BlueprintType)
struct FMOShortSaveInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FString SlotName;
	UPROPERTY(BlueprintReadOnly) FGuid   WorldGuid;
	UPROPERTY(BlueprintReadOnly) FName   LevelName;
	UPROPERTY(BlueprintReadOnly) FDateTime Timestamp;
	UPROPERTY(BlueprintReadOnly) float   PlayTimeSeconds = 0.f;
	UPROPERTY(BlueprintReadOnly) bool    bAutosave = false;
	UPROPERTY(BlueprintReadOnly) FString UserLabel;   // optional note/title
};

USTRUCT(BlueprintType)
struct FMOWorldSaves
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FGuid WorldGuid;
	UPROPERTY(BlueprintReadOnly) FName DisplayLevelName;     // from newest entry
	UPROPERTY(BlueprintReadOnly) TArray<FMOShortSaveInfo> Saves; // newest first
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMOOnSavesChanged);

UCLASS(BlueprintType)
class MO_1_API UMOPersistenceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	// high level API
	UFUNCTION(BlueprintCallable) bool SaveAll(const FGuid& WorldGuid);
	UFUNCTION(BlueprintCallable) bool LoadAll(const FGuid& WorldGuid);

	UPROPERTY(BlueprintAssignable, Category="MO|Save")
	FMOOnSavesChanged OnSavesChanged;

	// granular
	UFUNCTION(BlueprintCallable) bool SaveSettings();
	UFUNCTION(BlueprintCallable) bool LoadSettings();

	UFUNCTION(BlueprintCallable) bool SaveWorld(const FGuid& WorldGuid);
	UFUNCTION(BlueprintCallable) bool LoadWorld(const FGuid& WorldGuid);

	UFUNCTION(BlueprintCallable) bool SavePawnByGuid(const FGuid& PawnGuid);
	UFUNCTION(BlueprintCallable) bool LoadPawnByGuid(const FGuid& PawnGuid);

	UFUNCTION(BlueprintCallable) bool SaveTerrain(const FGuid& WorldGuid);
	UFUNCTION(BlueprintCallable) bool LoadTerrain(const FGuid& WorldGuid);

	UFUNCTION(BlueprintCallable, Category="MO|Save")
	bool DeleteSaveBySlot(const FString& SlotName);

	// Make a unique slot name for a world save like: World_<GUID>_20251111T171234_Auto
	UFUNCTION(BlueprintCallable, Category="MO|Save")
	static FString MakeWorldSlotName(const FGuid& WorldGuid, bool bAutosave);

	// Save to a new slot with metadata (manual or autosave). Label is optional.
	UFUNCTION(BlueprintCallable, Category="MO|Save")
	bool SaveWorldNewSlot(const FGuid& WorldGuid, bool bAutosave, const FString& Label);

	// helpers
	static FString SettingsSlot() { return TEXT("Settings"); }
	static FString WorldSlot(const FGuid& W) { return FString::Printf(TEXT("World_%s"), *W.ToString(EGuidFormats::Digits)); }
	static FString PawnSlot(const FGuid& P)  { return FString::Printf(TEXT("Pawn_%s"),  *P.ToString(EGuidFormats::Digits)); }
	static FString TerrainSlot(const FGuid& W){ return FString::Printf(TEXT("Terrain_%s"), *W.ToString(EGuidFormats::Digits)); }

	// Build grouped save list: Worlds -> Saves (sorted newest first)
	UFUNCTION(BlueprintCallable, Category="MO|Save")
	void QueryAllWorldSaves(TArray<FMOWorldSaves>& Out) const;

	// Request load of a specific world-save slot: opens the saved level then applies on next map
	UFUNCTION(BlueprintCallable, Category="MO|Save")
	bool RequestLoadGameBySlot(const FString& SlotName);

	// Call this from GameMode BeginPlay after a map opens to apply pending load (if any)
	UFUNCTION(BlueprintCallable, Category="MO|Save")
	bool ApplyPendingLoadIfAny();

	static FString SaveDir(); // Saved/SaveGames

	// Create or fetch the session's WorldGuid (simple in-memory, good enough for now)
	UFUNCTION(BlueprintCallable, Category="MO|Save")
	FGuid GetOrCreateWorldGuid();

	// Internal capture/apply helpers
	void CapturePawns(UMOWorldSaveGame* SG);
	void ApplyPawns(const UMOWorldSaveGame* SG);

private:
	// capture and apply
	void CaptureWorld(UMOWorldSaveGame* Out);
	void ApplyWorld(const UMOWorldSaveGame* In);

	bool CapturePawn(APawn* Pawn, UMOPawnSaveGame* Out);
	bool ApplyPawn(const UMOPawnSaveGame* In);

	// lookup
	APawn* FindPawnByGuid(const FGuid& Guid) const;
	UMOIdentityComponent* FindIdentity(UObject* Obj) const;

	// Helpers
	bool TryLoadWorldSaveHeader(const FString& SlotName, UMOWorldSaveGame*& OutSG) const;
	

	UPROPERTY() FString PendingLoadSlot;

	mutable FGuid CurrentWorldGuid;

	
};
