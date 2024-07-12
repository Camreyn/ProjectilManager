#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "IrGr_AbilityTypes.h"
#include "World/IrGr_ObjectPool.h" 
#include "IrGr_ProjectileManager.generated.h"

class AIrGr_Projectile;
template<class T> class TIrGrObjectPool;

UCLASS()
class GAMEFEATURESB_API UIrGr_ProjectileManager : public UGameInstanceSubsystem, public FTickableGameObject
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override { return !IsTemplate(); }
    virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UIrGr_ProjectileManager, STATGROUP_Tickables); }
    virtual void Deinitialize() override;

    void EnsureProjectilePoolInitialized(TSubclassOf<AIrGr_Projectile> ProjectileClass, int32 DesiredPoolSize);
    
    UFUNCTION(BlueprintCallable, Category = "Projectiles")
    AIrGr_Projectile* SpawnProjectile(const FProjectileData& Data, const FVector& Location, const FVector& Direction);

    void UpdateProjectiles(float DeltaTime);
    void ReturnProjectileToPool(AIrGr_Projectile* Projectile);

    static const int32 InitialPoolSize = 100;
    static const int32 PoolGrowthSize = 20;

private:
    void PreInitializeProjectilePool(TSubclassOf<AIrGr_Projectile> ProjectileClass, int32 Count);

    TMap<TSubclassOf<AIrGr_Projectile>, TUniquePtr<TIrGrObjectPool<AIrGr_Projectile>>> ProjectilePools;
    
    UPROPERTY()
    TArray<AIrGr_Projectile*> ActiveProjectiles;

    TIrGrObjectPool<AIrGr_Projectile>* GetOrCreatePool(TSubclassOf<AIrGr_Projectile> ProjectileClass);

    FCriticalSection ProjectileManagerLock;
};
