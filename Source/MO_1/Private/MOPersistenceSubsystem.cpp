#include "MOPersistenceSubsystem.h"
#include "EngineUtils.h"
#include "MO_1/MOIdentityComponent.h"
#include "MOPawnSaveGame.h"
#include "MOSettingsSaveGame.h"
#include "MOTerrainSaveGame.h"
#include "MOWorldSaveGame.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

bool UMOPersistenceSubsystem::SaveAll(const FGuid& WorldGuid)
{
    return true;
}

bool UMOPersistenceSubsystem::LoadAll(const FGuid& WorldGuid)
{
    return true;
}

bool UMOPersistenceSubsystem::SaveSettings()
{
    auto* SG = Cast<UMOSettingsSaveGame>(UGameplayStatics::CreateSaveGameObject(UMOSettingsSaveGame::StaticClass()));
    if (!SG) return false;

    // fill settings from current runtime

    return UGameplayStatics::SaveGameToSlot(SG, SettingsSlot(), 0);
}

bool UMOPersistenceSubsystem::LoadSettings()
{
    if (!UGameplayStatics::DoesSaveGameExist(SettingsSlot(), 0)) return false;
    auto* SG = Cast<UMOSettingsSaveGame>(UGameplayStatics::LoadGameFromSlot(SettingsSlot(), 0));
    if (!SG) return false;

    // apply settings to runtime

    return true;
}

bool UMOPersistenceSubsystem::SaveWorld(const FGuid& WorldGuid)
{
    auto* SG = Cast<UMOWorldSaveGame>(UGameplayStatics::CreateSaveGameObject(UMOWorldSaveGame::StaticClass()));
    if (!SG) return false;

    SG->WorldGuid = WorldGuid;
    SG->LevelName = FName(*UGameplayStatics::GetCurrentLevelName(this, true));

    CaptureWorld(SG);
    return UGameplayStatics::SaveGameToSlot(SG, WorldSlot(WorldGuid), 0);
}

bool UMOPersistenceSubsystem::LoadWorld(const FGuid& WorldGuid)
{
    const FString Slot = WorldSlot(WorldGuid);
    if (!UGameplayStatics::DoesSaveGameExist(Slot, 0)) return false;

    auto* SG = Cast<UMOWorldSaveGame>(UGameplayStatics::LoadGameFromSlot(Slot, 0));
    if (!SG) return false;

    ApplyWorld(SG);
    return true;
}

bool UMOPersistenceSubsystem::SavePawnByGuid(const FGuid& PawnGuid)
{
    APawn* Pawn = FindPawnByGuid(PawnGuid);
    if (!Pawn) return false;

    auto* SG = Cast<UMOPawnSaveGame>(UGameplayStatics::CreateSaveGameObject(UMOPawnSaveGame::StaticClass()));
    if (!SG) return false;

    SG->PawnGuid  = PawnGuid;
    CapturePawn(Pawn, SG);

    return UGameplayStatics::SaveGameToSlot(SG, PawnSlot(PawnGuid), 0);
}

bool UMOPersistenceSubsystem::LoadPawnByGuid(const FGuid& PawnGuid)
{
    const FString Slot = PawnSlot(PawnGuid);
    if (!UGameplayStatics::DoesSaveGameExist(Slot, 0)) return false;

    auto* SG = Cast<UMOPawnSaveGame>(UGameplayStatics::LoadGameFromSlot(Slot, 0));
    if (!SG) return false;

    return ApplyPawn(SG);
}

bool UMOPersistenceSubsystem::SaveTerrain(const FGuid& WorldGuid)
{
    return true;
}

bool UMOPersistenceSubsystem::LoadTerrain(const FGuid& WorldGuid)
{
    return true;
}

// -------- capture/apply basics --------

void UMOPersistenceSubsystem::CaptureWorld(UMOWorldSaveGame* Out)
{
    UWorld* W = GetWorld();
    if (!W) return;

    Out->Actors.Reset();
    for (TActorIterator<AActor> It(W); It; ++It)
    {
        AActor* A = *It;
        if (!IsValid(A)) continue;

        if (auto* Id = FindIdentity(A))
        {
            if (Id->HasValidGuid())
            {
                FMOPersistedActor Rec;
                Rec.Guid      = Id->GetGuid();
                Rec.Transform = A->GetActorTransform();
                // Rec.ClassPath = ... optional for respawn
                Out->Actors.Add(Rec);
            }
        }
    }

    // fill Out->DestroyedActorGuids if you track those during gameplay
}

void UMOPersistenceSubsystem::ApplyWorld(const UMOWorldSaveGame* In)
{
    UWorld* W = GetWorld();
    if (!W) return;

    // Destroy any actors whose GUIDs are marked destroyed
    for (TActorIterator<AActor> It(W); It; ++It)
    {
        if (auto* Id = FindIdentity(*It))
        {
            if (In->DestroyedActorGuids.Contains(Id->GetGuid()))
            {
                (*It)->Destroy();
            }
        }
    }

    // Move existing actors to saved transforms
    for (const FMOPersistedActor& Rec : In->Actors)
    {
        for (TActorIterator<AActor> It2(W); It2; ++It2)
        {
            if (auto* Id2 = FindIdentity(*It2))
            {
                if (Id2->GetGuid() == Rec.Guid)
                {
                    (*It2)->SetActorTransform(Rec.Transform);
                    break;
                }
            }
        }
    }

    // Optionally spawn missing actors by ClassPath here
}

bool UMOPersistenceSubsystem::CapturePawn(APawn* Pawn, UMOPawnSaveGame* Out)
{
    if (!Pawn) return false;
    Out->Transform = Pawn->GetActorTransform();

    // Inventory capture goes here

    return true;
}

bool UMOPersistenceSubsystem::ApplyPawn(const UMOPawnSaveGame* In)
{
    APawn* Pawn = FindPawnByGuid(In->PawnGuid);
    if (!Pawn) return false;

    Pawn->SetActorTransform(In->Transform);

    // Inventory apply goes here

    return true;
}

// -------- lookups --------

APawn* UMOPersistenceSubsystem::FindPawnByGuid(const FGuid& Guid) const
{
    UWorld* W = GetWorld();
    if (!W) return nullptr;

    for (TActorIterator<APawn> It(W); It; ++It)
    {
        if (auto* Id = FindIdentity(*It))
        {
            if (Id->GetGuid() == Guid) return *It;
        }
    }
    return nullptr;
}

UMOIdentityComponent* UMOPersistenceSubsystem::FindIdentity(UObject* Obj) const
{
    if (AActor* A = Cast<AActor>(Obj))
    {
        return A->FindComponentByClass<UMOIdentityComponent>();
    }
    return nullptr;
}
