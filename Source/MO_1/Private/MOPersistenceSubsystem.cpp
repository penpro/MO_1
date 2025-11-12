#include "MOPersistenceSubsystem.h"
#include "EngineUtils.h"
#include "MO_1/MOIdentityComponent.h"
#include "MOPawnSaveGame.h"
#include "MOSettingsSaveGame.h"
#include "GameFramework/GameStateBase.h"
#include "MOTerrainSaveGame.h"
#include "HAL/FileManagerGeneric.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "MOWorldSaveGame.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

class UMOIdentityComponent;

static FString GuidDigits(const FGuid& G)
{
    return G.ToString(EGuidFormats::Digits);
}

static bool IsWorldSlotName(const FString& Slot)
{
    return Slot.StartsWith(TEXT("World_"));
}

FString UMOPersistenceSubsystem::SaveDir()
{
    return FPaths::ProjectSavedDir() / TEXT("SaveGames");
}

// MOPersistenceSubsystem.cpp (top-level static helper)
static FDateTime CoalesceTimestamp(const FDateTime& SavedAt, const FString& SlotName)
{
    if (SavedAt.GetTicks() > 0) return SavedAt;
    const FString Path = UMOPersistenceSubsystem::SaveDir() / (SlotName + TEXT(".sav"));
    const FDateTime Stamp = IFileManager::Get().GetTimeStamp(*Path);
    return (Stamp.GetTicks() > 0) ? Stamp : FDateTime::UtcNow();
}

bool UMOPersistenceSubsystem::SaveAll(const FGuid& WorldGuid)
{
    return false;
}

bool UMOPersistenceSubsystem::LoadAll(const FGuid& WorldGuid)
{
    return false;
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

    SG->WorldGuid   = WorldGuid;
    SG->LevelName   = FName(*UGameplayStatics::GetCurrentLevelName(this, true));
    SG->SavedAt     = FDateTime::UtcNow();
    SG->bIsAutosave = false;
    SG->Label       = TEXT("Quick Save");

    // Optional: carry forward playtime
    float PreviousPlay = 0.f;
    {
        TArray<FMOWorldSaves> Worlds;
        QueryAllWorldSaves(Worlds);
        for (const FMOWorldSaves& W : Worlds)
        {
            if (W.WorldGuid == WorldGuid && W.Saves.Num() > 0)
            {
                PreviousPlay = FMath::Max(PreviousPlay, W.Saves[0].PlayTimeSeconds);
                break;
            }
        }
    }
    float SessionSeconds = 0.f;
    if (AGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState<AGameStateBase>() : nullptr)
    {
        SessionSeconds = GS->GetServerWorldTimeSeconds();
    }
    SG->PlaySeconds = PreviousPlay + SessionSeconds;

    // CAPTURE — same idea here
    CaptureWorld(SG);   // existing
    CapturePawns(SG);   // NEW

    const bool bOK = UGameplayStatics::SaveGameToSlot(SG, WorldSlot(WorldGuid), 0);
    UE_LOG(LogTemp, Display, TEXT("[Save-Quick] %s"), bOK ? TEXT("OK") : TEXT("FAILED"));
    if (bOK) { OnSavesChanged.Broadcast(); }
    return bOK;
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

bool UMOPersistenceSubsystem::DeleteSaveBySlot(const FString& SlotName)
{
    const FString Path = SaveDir() / (SlotName + TEXT(".sav"));
    const bool bDeleted = IFileManager::Get().Delete(*Path, false, true, true);
    if (bDeleted) { OnSavesChanged.Broadcast(); }
    return bDeleted;
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

UMOIdentityComponent* UMOPersistenceSubsystem::FindIdentity(UObject* Obj) const
{
    if (AActor* A = Cast<AActor>(Obj))
    {
        return A->FindComponentByClass<UMOIdentityComponent>();
    }
    return nullptr;
}

bool UMOPersistenceSubsystem::TryLoadWorldSaveHeader(const FString& SlotName, UMOWorldSaveGame*& OutSG) const
{
    OutSG = nullptr;
    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0)) return false;

    if (USaveGame* Raw = UGameplayStatics::LoadGameFromSlot(SlotName, 0))
    {
        if (UMOWorldSaveGame* WS = Cast<UMOWorldSaveGame>(Raw))
        {
            OutSG = WS;
            return true;
        }
    }
    return false;
}

void UMOPersistenceSubsystem::QueryAllWorldSaves(TArray<FMOWorldSaves>& Out) const
{
    Out.Reset();

    // 1) find *.sav
    TArray<FString> Files;
    IFileManager::Get().FindFiles(Files, *(SaveDir() / TEXT("*.sav")), true, false);

    // 2) group by world guid
    TMap<FGuid, FMOWorldSaves> Worlds;

    for (const FString& File : Files)
    {
        const FString SlotName = FPaths::GetBaseFilename(File); // strip .sav
        if (!IsWorldSlotName(SlotName)) continue;

        UMOWorldSaveGame* WS = nullptr;
        if (!TryLoadWorldSaveHeader(SlotName, WS))
        {
            continue;
        }

        FMOShortSaveInfo Info;
        Info.SlotName       = SlotName;
        Info.WorldGuid      = WS->WorldGuid;
        Info.LevelName      = WS->LevelName;
        Info.Timestamp      = CoalesceTimestamp(WS->SavedAt, SlotName);
        Info.PlayTimeSeconds= WS->PlaySeconds;
        Info.bAutosave      = WS->bIsAutosave;
        Info.UserLabel      = WS->Label; // ok if empty

        FMOWorldSaves& World = Worlds.FindOrAdd(WS->WorldGuid);
        World.WorldGuid = WS->WorldGuid;
        World.Saves.Add(MoveTemp(Info));
    }

    // 3) sort each world's saves (newest first) and pick display level name
    for (auto& KVP : Worlds)
    {
        FMOWorldSaves& W = KVP.Value;
        W.Saves.Sort([](const FMOShortSaveInfo& A, const FMOShortSaveInfo& B)
        {
            return A.Timestamp > B.Timestamp;
        });
        if (W.Saves.Num() > 0)
        {
            W.DisplayLevelName = W.Saves[0].LevelName;
        }
        Out.Add(W);
    }

    // 4) optional: sort worlds by latest activity
    Out.Sort([](const FMOWorldSaves& A, const FMOWorldSaves& B)
    {
        const FDateTime TA = A.Saves.Num() ? A.Saves[0].Timestamp : FDateTime(0);
        const FDateTime TB = B.Saves.Num() ? B.Saves[0].Timestamp : FDateTime(0);
        return TA > TB;
    });
}

bool UMOPersistenceSubsystem::RequestLoadGameBySlot(const FString& SlotName)
{
    UMOWorldSaveGame* WS = nullptr;
    if (!TryLoadWorldSaveHeader(SlotName, WS)) return false;

    UWorld* W = GetWorld();
    if (!W) return false;

    PendingLoadSlot = SlotName;

    const FString LevelStr = WS->LevelName.ToString();

    switch (W->GetNetMode())
    {
    case NM_Standalone:
        {
            UGameplayStatics::OpenLevel(this, WS->LevelName);
            return true;
        }
    case NM_ListenServer:
        {
            // Server travel so clients follow
            W->ServerTravel(LevelStr, true /*bAbsolute*/);
            return true;
        }
    case NM_Client:
    default:
        {
            UE_LOG(LogTemp, Warning, TEXT("RequestLoadGameBySlot called on client. Route this through the server."));
            // TODO: add a server RPC path via a PlayerController or GameMode if you want client-initiated loads
            PendingLoadSlot.Reset();
            return false;
        }
    }
}

bool UMOPersistenceSubsystem::ApplyPendingLoadIfAny()
{
    if (PendingLoadSlot.IsEmpty()) return false;

    // Load the full world save and apply it
    UMOWorldSaveGame* WS = nullptr;
    if (!TryLoadWorldSaveHeader(PendingLoadSlot, WS))
    {
        PendingLoadSlot.Reset();
        return false;
    }

    // First: world state (your existing call)
    ApplyWorld(WS);

    // Then: pawns (NEW — restores GUID-addressed pawn transforms and velocities)
    ApplyPawns(WS);

    PendingLoadSlot.Reset();
    return true;
}

FString UMOPersistenceSubsystem::MakeWorldSlotName(const FGuid& WorldGuid, bool bAutosave)
{
    const FDateTime Now = FDateTime::UtcNow();
    const FString Stamp = Now.ToString(TEXT("%Y%m%dT%H%M%S")); // sortable
    return FString::Printf(TEXT("World_%s_%s_%s"),
        *GuidDigits(WorldGuid),
        *Stamp,
        bAutosave ? TEXT("Auto") : TEXT("Manual"));
}

bool UMOPersistenceSubsystem::SaveWorldNewSlot(const FGuid& WorldGuid, bool bAutosave, const FString& Label)
{
    // 1) Create save object
    auto* SG = Cast<UMOWorldSaveGame>(UGameplayStatics::CreateSaveGameObject(UMOWorldSaveGame::StaticClass()));
    if (!SG) return false;

    // 2) Fill metadata
    SG->WorldGuid   = WorldGuid;
    SG->LevelName   = FName(*UGameplayStatics::GetCurrentLevelName(this, /*bRemovePrefix*/ true));
    SG->SavedAt     = FDateTime::UtcNow();
    SG->bIsAutosave = bAutosave;
    SG->Label       = Label;

    // Accumulate total playtime (optional but nice for UI)
    float PreviousPlay = 0.f;
    {
        TArray<FMOWorldSaves> Worlds;
        QueryAllWorldSaves(Worlds);
        for (const FMOWorldSaves& W : Worlds)
        {
            if (W.WorldGuid == WorldGuid && W.Saves.Num() > 0)
            {
                PreviousPlay = FMath::Max(PreviousPlay, W.Saves[0].PlayTimeSeconds);
                break;
            }
        }
    }
    float SessionSeconds = 0.f;
    if (AGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState<AGameStateBase>() : nullptr)
    {
        SessionSeconds = GS->GetServerWorldTimeSeconds();
    }
    SG->PlaySeconds = PreviousPlay + SessionSeconds;

    // 3) CAPTURE — this is where your world + pawns snapshot is taken
    CaptureWorld(SG);   // <- you already had this
    CapturePawns(SG);   // <- NEW: add this call

    // 4) Save to a unique slot
    // Ensure directory exists
    IFileManager::Get().MakeDirectory(*SaveDir(), /*Tree*/ true);

    // Unique slot name
    const FString Slot = MakeWorldSlotName(WorldGuid, bAutosave);

    // Log the absolute path we expect to appear
    const FString FullPath = SaveDir() / (Slot + TEXT(".sav"));
    UE_LOG(LogTemp, Display, TEXT("[Save] Writing slot '%s' at '%s'"), *Slot, *FullPath);

    const bool bOK = UGameplayStatics::SaveGameToSlot(SG, Slot, 0);
    UE_LOG(LogTemp, Display, TEXT("[Save] %s -> %s"), bOK ? TEXT("OK") : TEXT("FAILED"), *Slot);

    if (bOK) { OnSavesChanged.Broadcast(); }

    
}

FGuid UMOPersistenceSubsystem::GetOrCreateWorldGuid()
{
    if (!CurrentWorldGuid.IsValid())
    {
        // Try to reuse an existing world's guid for this level (optional heuristic)
        const FName ThisLevel(*UGameplayStatics::GetCurrentLevelName(this, true));
        TArray<FMOWorldSaves> Worlds;
        QueryAllWorldSaves(Worlds);
        for (const auto& W : Worlds)
        {
            if (W.DisplayLevelName == ThisLevel && W.WorldGuid.IsValid())
            {
                CurrentWorldGuid = W.WorldGuid;
                break;
            }
        }
        if (!CurrentWorldGuid.IsValid())
        {
            CurrentWorldGuid = FGuid::NewGuid();
        }
    }
    return CurrentWorldGuid;
}

APawn* UMOPersistenceSubsystem::FindPawnByGuid(const FGuid& Guid) const
{
    UWorld* W = GetWorld();
    if (!W || !Guid.IsValid()) return nullptr;

    TArray<AActor*> Actors;
    UGameplayStatics::GetAllActorsOfClass(W, APawn::StaticClass(), Actors);

    for (AActor* A : Actors)
    {
        APawn* P = Cast<APawn>(A);
        if (!P) continue;

        // CHANGE THIS to your actual identity component type and guid accessor
        if (UMOIdentityComponent* IC = P->FindComponentByClass<UMOIdentityComponent>())
        {
            // e.g., FGuid Cid = IC->GetGuid(); or IC->Guid;
            const FGuid Cid = IC->GetGuid();         // <- same getter here
            if (Cid == Guid)
            {
                return P;
            }
        }
    }
    return nullptr;
}

void UMOPersistenceSubsystem::CapturePawns(UMOWorldSaveGame* SG)
{
    if (!SG) return;
    SG->PawnStates.Reset();

    UWorld* W = GetWorld();
    if (!W) return;

    TArray<AActor*> Actors;
    UGameplayStatics::GetAllActorsOfClass(W, APawn::StaticClass(), Actors);

    for (AActor* A : Actors)
    {
        APawn* P = Cast<APawn>(A);
        if (!P) continue;

        // CHANGE THIS to your identity component type/getter
        if (UMOIdentityComponent* IC = P->FindComponentByClass<UMOIdentityComponent>())
        {
            const FGuid Cid = IC->GetGuid();
            if (!Cid.IsValid()) continue;

            FMOPawnState S;
            S.PawnGuid  = Cid;
            S.Transform = P->GetActorTransform();
            S.Velocity  = P->GetVelocity();

            SG->PawnStates.Add(MoveTemp(S));
        }
    }
}

void UMOPersistenceSubsystem::ApplyPawns(const UMOWorldSaveGame* SG)
{
    if (!SG) return;

    for (const FMOPawnState& S : SG->PawnStates)
    {
        if (!S.PawnGuid.IsValid()) continue;

        if (APawn* P = FindPawnByGuid(S.PawnGuid))
        {
            // Stop movement first for clean teleporting
            if (ACharacter* C = Cast<ACharacter>(P))
            {
                if (UCharacterMovementComponent* Move = C->GetCharacterMovement())
                {
                    Move->StopMovementImmediately();
                }
            }

            P->SetActorTransform(S.Transform, false, nullptr, ETeleportType::TeleportPhysics);

            if (ACharacter* C = Cast<ACharacter>(P))
            {
                if (UCharacterMovementComponent* Move = C->GetCharacterMovement())
                {
                    Move->Velocity = S.Velocity;
                }
            }
        }
        // else: optional spawn-by-class if missing — leave out for now to keep test simple
    }
}