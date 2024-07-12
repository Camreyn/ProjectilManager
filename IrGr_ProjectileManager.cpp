#include "Game/IrGr_ProjectileManager.h"
#include "Actor/IrGr_Projectile.h"
#include "World/IrGr_ObjectPool.h"

void UIrGr_ProjectileManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    PreInitializeProjectilePool(AIrGr_Projectile::StaticClass(), InitialPoolSize);
}

void UIrGr_ProjectileManager::Tick(float DeltaTime)
{
    FScopeLock Lock(&ProjectileManagerLock);
    
    UE_LOG(LogTemp, Log, TEXT("ProjectileManager Tick called"));

    UE_LOG(LogTemp, Log, TEXT("ActiveProjectiles size: %d"), ActiveProjectiles.Num());

    for (int32 i = ActiveProjectiles.Num() - 1; i >= 0; --i)
    {
        if (!IsValid(ActiveProjectiles[i]))
        {
            UE_LOG(LogTemp, Warning, TEXT("Invalid projectile found at index %d, removing"), i);
            ActiveProjectiles.RemoveAtSwap(i, 1, false);
            continue;
        }

        AIrGr_Projectile* Projectile = ActiveProjectiles[i];
        if (ensureMsgf(Projectile, TEXT("Valid projectile in ActiveProjectiles array at index %d"), i))
        {
            Projectile->UpdateProjectile(DeltaTime);
            if (Projectile->ShouldDeactivate())
            {
                UE_LOG(LogTemp, Verbose, TEXT("Deactivating projectile %s"), *Projectile->GetName());
                ReturnProjectileToPool(Projectile);
                ActiveProjectiles.RemoveAtSwap(i, 1, false);
            }
        }
    }

    UE_LOG(LogTemp, Verbose, TEXT("%d active projectiles after update"), ActiveProjectiles.Num());
}


void UIrGr_ProjectileManager::Deinitialize()
{
    FScopeLock Lock(&ProjectileManagerLock);
    ProjectilePools.Empty();
    ActiveProjectiles.Empty();
    Super::Deinitialize();
}

void UIrGr_ProjectileManager::EnsureProjectilePoolInitialized(TSubclassOf<AIrGr_Projectile> ProjectileClass, int32 DesiredPoolSize)
{
    if (!ProjectileClass) return;

    FScopeLock Lock(&ProjectileManagerLock);
    TIrGrObjectPool<AIrGr_Projectile>* Pool = GetOrCreatePool(ProjectileClass);
    
    if (Pool->GetPoolSize() < DesiredPoolSize)
    {
        int32 AdditionalSize = FMath::Max(DesiredPoolSize - Pool->GetPoolSize(), PoolGrowthSize);
        Pool->GrowPool(AdditionalSize);
    }
}

AIrGr_Projectile* UIrGr_ProjectileManager::SpawnProjectile(const FProjectileData& Data, const FVector& Location, const FVector& Direction)
{
    if (!Data.ProjectileClass) return nullptr;

    FScopeLock Lock(&ProjectileManagerLock);
    TIrGrObjectPool<AIrGr_Projectile>* Pool = GetOrCreatePool(Data.ProjectileClass);
    AIrGr_Projectile* Projectile = Pool->GetObject();

    if (IsValid(Projectile))
    {
        Projectile->SetActorLocation(Location);
        Projectile->SetActorRotation(Direction.Rotation());
        Projectile->Initialize(Data);
        ActiveProjectiles.Add(Projectile);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn projectile: Invalid projectile returned from pool"));
    }

    return Projectile;
}

void UIrGr_ProjectileManager::UpdateProjectiles(float DeltaTime)
{
    FScopeLock Lock(&ProjectileManagerLock);
    for (int32 i = ActiveProjectiles.Num() - 1; i >= 0; --i)
    {
        AIrGr_Projectile* Projectile = ActiveProjectiles[i];
        if (Projectile)
        {
            Projectile->UpdateProjectile(DeltaTime);
            if (Projectile->ShouldDeactivate())
            {
                ReturnProjectileToPool(Projectile);
                ActiveProjectiles.RemoveAtSwap(i);
            }
        }
        else
        {
            ActiveProjectiles.RemoveAtSwap(i);
        }
    }
}

void UIrGr_ProjectileManager::ReturnProjectileToPool(AIrGr_Projectile* Projectile)
{
    if (!Projectile) return;

    FScopeLock Lock(&ProjectileManagerLock);
    TSubclassOf<AIrGr_Projectile> ProjectileClass = Projectile->GetClass();
    TIrGrObjectPool<AIrGr_Projectile>* Pool = GetOrCreatePool(ProjectileClass);

    if (Pool)
    {
        Projectile->ResetProjectile();
        Pool->ReturnObject(Projectile);
        ActiveProjectiles.Remove(Projectile);
    }
}

void UIrGr_ProjectileManager::PreInitializeProjectilePool(TSubclassOf<AIrGr_Projectile> ProjectileClass, int32 Count)
{
    if (!ProjectileClass) return;

    FScopeLock Lock(&ProjectileManagerLock);
    TIrGrObjectPool<AIrGr_Projectile>* Pool = GetOrCreatePool(ProjectileClass);
    
    int32 CurrentPoolSize = Pool->GetPoolSize();
    if (CurrentPoolSize < Count)
    {
        Pool->GrowPool(Count - CurrentPoolSize);
    }
}

TIrGrObjectPool<AIrGr_Projectile>* UIrGr_ProjectileManager::GetOrCreatePool(TSubclassOf<AIrGr_Projectile> ProjectileClass)
{
    if (!ProjectilePools.Contains(ProjectileClass))
    {
        ProjectilePools.Add(ProjectileClass, MakeUnique<TIrGrObjectPool<AIrGr_Projectile>>(GetWorld(), InitialPoolSize));
    }
    return ProjectilePools[ProjectileClass].Get();
}
