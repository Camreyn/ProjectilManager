#include "Actor/IrGr_Projectile.h"

template<class T>
TIrGrObjectPool<T>::TIrGrObjectPool(UWorld* InWorld, int32 InitialSize)
    : World(InWorld), PoolSize(0), FreeObjectCount(0)
{
    GrowPool(InitialSize);
}

template<class T>
TIrGrObjectPool<T>::~TIrGrObjectPool()
{
    for (T* Object : AllObjects)
    {
        if (Object)
        {
            Object->Destroy();
        }
    }
}

template<class T>
T* TIrGrObjectPool<T>::GetObject()
{
    FScopeLock Lock(&PoolLock);
    T* Object = nullptr;
    
    while (!FreeObjects.IsEmpty())
    {
        FreeObjects.Dequeue(Object);
        FreeObjectCount--;
        
        if (IsValid(Object))
        {
            if (AIrGr_Projectile* Projectile = Cast<AIrGr_Projectile>(Object))
            {
                Projectile->ResetProjectile();
            }
            return Object;
        }
    }

    // If we get here, the pool is empty or all objects were invalid
    int32 GrowthSize = FMath::Max(10, PoolSize / 2);
    GrowPool(GrowthSize);

    // Try to get an object again after growing the pool
    if (FreeObjects.Dequeue(Object) && IsValid(Object))
    {
        FreeObjectCount--;
        if (AIrGr_Projectile* Projectile = Cast<AIrGr_Projectile>(Object))
        {
            Projectile->ResetProjectile();
        }
        return Object;
    }

    return nullptr;
}

template<class T>
void TIrGrObjectPool<T>::ReturnObject(T* Object)
{
    if (IsValid(Object))
    {
        FScopeLock Lock(&PoolLock);
        if (AIrGr_Projectile* Projectile = Cast<AIrGr_Projectile>(Object))
        {
            Projectile->ResetProjectile();
        }
        FreeObjects.Enqueue(Object);
        FreeObjectCount++;
    }
}

template<class T>
void TIrGrObjectPool<T>::GrowPool(int32 GrowthSize)
{
    FScopeLock Lock(&PoolLock);
    int32 ActualGrowth = 0;
    for (int32 i = 0; i < GrowthSize; ++i)
    {
        T* NewObject = CreateNewObject();
        if (IsValid(NewObject))
        {
            AllObjects.Add(NewObject);
            FreeObjects.Enqueue(NewObject);
            FreeObjectCount++;
            ActualGrowth++;
        }
    }
    PoolSize += ActualGrowth;
}

template<class T>
T* TIrGrObjectPool<T>::CreateNewObject()
{
    T* NewObject = World->SpawnActor<T>(T::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
    if (IsValid(NewObject))
    {
        if (AIrGr_Projectile* Projectile = Cast<AIrGr_Projectile>(NewObject))
        {
            Projectile->DeactivateProjectile();
        }
    }
    return NewObject;
}
