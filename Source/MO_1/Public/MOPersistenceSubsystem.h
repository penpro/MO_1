#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MOPersistenceSubsystem.generated.h"

class UMOSettingsSaveGame;
class UMOWorldSaveGame;
class UMOPawnSaveGame;
class UMOTerrainSaveGame;
class UMOIdentityComponent;

UCLASS(BlueprintType)
class MO_1_API UMOPersistenceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	// high level API
	UFUNCTION(BlueprintCallable) bool SaveAll(const FGuid& WorldGuid);
	UFUNCTION(BlueprintCallable) bool LoadAll(const FGuid& WorldGuid);

	// granular
	UFUNCTION(BlueprintCallable) bool SaveSettings();
	UFUNCTION(BlueprintCallable) bool LoadSettings();

	UFUNCTION(BlueprintCallable) bool SaveWorld(const FGuid& WorldGuid);
	UFUNCTION(BlueprintCallable) bool LoadWorld(const FGuid& WorldGuid);

	UFUNCTION(BlueprintCallable) bool SavePawnByGuid(const FGuid& PawnGuid);
	UFUNCTION(BlueprintCallable) bool LoadPawnByGuid(const FGuid& PawnGuid);

	UFUNCTION(BlueprintCallable) bool SaveTerrain(const FGuid& WorldGuid);
	UFUNCTION(BlueprintCallable) bool LoadTerrain(const FGuid& WorldGuid);

	// helpers
	static FString SettingsSlot() { return TEXT("Settings"); }
	static FString WorldSlot(const FGuid& W) { return FString::Printf(TEXT("World_%s"), *W.ToString(EGuidFormats::Digits)); }
	static FString PawnSlot(const FGuid& P)  { return FString::Printf(TEXT("Pawn_%s"),  *P.ToString(EGuidFormats::Digits)); }
	static FString TerrainSlot(const FGuid& W){ return FString::Printf(TEXT("Terrain_%s"), *W.ToString(EGuidFormats::Digits)); }

private:
	// capture and apply
	void CaptureWorld(UMOWorldSaveGame* Out);
	void ApplyWorld(const UMOWorldSaveGame* In);

	bool CapturePawn(APawn* Pawn, UMOPawnSaveGame* Out);
	bool ApplyPawn(const UMOPawnSaveGame* In);

	// lookup
	APawn* FindPawnByGuid(const FGuid& Guid) const;
	UMOIdentityComponent* FindIdentity(UObject* Obj) const;
};
