#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "IrGr_AbilityTypes.h"
#include "IrGr_Projectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UAudioComponent;

UCLASS()
class GAMEFEATURESB_API AIrGr_Projectile : public AActor
{
    GENERATED_BODY()

public:
    AIrGr_Projectile();

    void Initialize(const FProjectileData& Data);
    void UpdateProjectile(float DeltaTime);
    bool ShouldDeactivate() const;
    void ResetProjectile();
    void DeactivateProjectile();

    UFUNCTION(BlueprintCallable)
    void SetInstigator(AActor* InInstigator);

    UFUNCTION(BlueprintCallable)
    AActor* GetOwningActor() const;

    UFUNCTION(BlueprintCallable)
    virtual void OnHit();

    void SetProjectileType(const FGameplayTag& TypeTag);
    
    FGameplayTag GetProjectileState() const;

    UPROPERTY(BlueprintReadWrite, Category = "Projectile")
    FDamageEffectParams DamageEffectParams;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UProjectileMovementComponent* ProjectileMovement;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* CollisionComponent;

    UPROPERTY()
    TObjectPtr<USceneComponent> HomingTargetSceneComponent;

    bool IsValidOverlap(AActor* OtherActor) const;

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* LoopingSoundComponent;
    
    bool bHit;

private:
    FGameplayTag ProjectileTypeTag;
    FGameplayTag CurrentState;
    FProjectileData ProjectileData;
    float LifeSpan;
    float CurrentLifeTime;
    
    UPROPERTY()
    AActor* OwningActor;

    void HandleImpact(AActor* OtherActor, const FHitResult& SweepResult);
};
