#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"

template<class T>
class GAMEFEATURESB_API TIrGrObjectPool
{
public:
	TIrGrObjectPool(UWorld* InWorld, int32 InitialSize = 30);
	~TIrGrObjectPool();

	T* GetObject();
	void ReturnObject(T* Object);
	void GrowPool(int32 GrowthSize);
	int32 GetPoolSize() const { return PoolSize; }
	int32 GetFreeObjectCount() const { return FreeObjectCount; }

private:
	T* CreateNewObject();

	UWorld* World;
	TArray<T*> AllObjects;
	TQueue<T*> FreeObjects;
	FCriticalSection PoolLock;
	int32 PoolSize;
	int32 FreeObjectCount;
};

#include "IrGr_ObjectPool.inl"
