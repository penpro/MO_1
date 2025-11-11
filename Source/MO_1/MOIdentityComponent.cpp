#include "MOIdentityComponent.h"
#include "Net/UnrealNetwork.h"

UMOIdentityComponent::UMOIdentityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UMOIdentityComponent::BeginPlay()
{
	Super::BeginPlay();
	// At runtime on authority, ensure a GUID exists.
	if (GetOwner() && GetOwner()->HasAuthority() && !StableGuid.IsValid())
	{
		StableGuid = FGuid::NewGuid();
	}
}

void UMOIdentityComponent::OnComponentCreated()
{
	Super::OnComponentCreated();
#if WITH_EDITOR
	// In the editor, assign a GUID if missing so the instance has a stable key before play.
	if (!HasAnyFlags(RF_ClassDefaultObject) && !StableGuid.IsValid())
	{
		StableGuid = FGuid::NewGuid();
	}
#endif
}

void UMOIdentityComponent::PostLoad()
{
	Super::PostLoad();
	// Make sure cooked content keeps a GUID
	if (!StableGuid.IsValid())
	{
		StableGuid = FGuid::NewGuid();
	}
}

#if WITH_EDITOR
void UMOIdentityComponent::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);
	// If the user duplicated the actor in the editor (not PIE), give the copy a new GUID.
	if (!bDuplicateForPIE)
	{
		StableGuid = FGuid::NewGuid();
	}
}
#endif

void UMOIdentityComponent::RegenerateGuid()
{
	StableGuid = FGuid::NewGuid();
#if WITH_EDITOR
	// Mark the package dirty so it saves
	if (AActor* Owner = GetOwner())
	{
		Owner->Modify();
	}
	Modify();
#endif
}

void UMOIdentityComponent::EnsureGuidGenerated(bool bEditorContext, bool bForPIE)
{
	// Reserved for future policy tweaks
	if (!StableGuid.IsValid())
	{
		StableGuid = FGuid::NewGuid();
	}
}

void UMOIdentityComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UMOIdentityComponent, DisplayName);
	DOREPLIFETIME(UMOIdentityComponent, StableGuid);
}
