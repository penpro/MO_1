#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MOIdentityComponent.generated.h"

/**
 * Identity component that gives any actor a human-readable DisplayName
 * and a stable GUID for possession, save/load, and UI.
 */
UCLASS(ClassGroup=(MO), meta=(BlueprintSpawnableComponent))
class MO_1_API UMOIdentityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMOIdentityComponent();

	/** Human-readable name for menus. Set per instance in editor or at spawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category="MO|Identity", meta=(ExposeOnSpawn="true"))
	FText DisplayName;

	/** Stable GUID used as the canonical identity key. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, SaveGame, Category="MO|Identity")
	FGuid StableGuid;

	/** Returns true if StableGuid is valid. */
	UFUNCTION(BlueprintPure, Category="MO|Identity")
	bool HasValidGuid() const { return StableGuid.IsValid(); }

	/** Getters for BP and C++. */
	UFUNCTION(BlueprintPure, Category="MO|Identity")
	FText GetDisplayName() const { return DisplayName; }

	UFUNCTION(BlueprintPure, Category="MO|Identity")
	FGuid GetGuid() const { return StableGuid; }

	/** Call in editor to force a new GUID if you duplicated an actor on purpose. */
	UFUNCTION(CallInEditor, BlueprintCallable, Category="MO|Identity")
	void RegenerateGuid();

protected:
	virtual void BeginPlay() override;
	virtual void OnComponentCreated() override;
	virtual void PostLoad() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#if WITH_EDITOR
	/** When duplicating in editor (not for PIE), generate a fresh GUID to avoid collisions. */
	virtual void PostDuplicate(bool bDuplicateForPIE) override;
#endif

private:
	void EnsureGuidGenerated(bool bEditorContext, bool bForPIE);
};
