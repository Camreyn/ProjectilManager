#include "Actor/IrGr_Projectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "IrGr_GameplayTags.h"
#include "AbilitySystem/IrGr_AbilitySystemLibrary.h"

AIrGr_Projectile::AIrGr_Projectile()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    CollisionComponent = CreateDefaultSubobject<USphereComponent>("SphereComponent");
    SetRootComponent(CollisionComponent);
    CollisionComponent->SetCollisionProfileName("Projectile");
    CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AIrGr_Projectile::OnSphereOverlap);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
    ProjectileMovement->UpdatedComponent = CollisionComponent;
    ProjectileMovement->InitialSpeed = 3000.0f;
    ProjectileMovement->MaxSpeed = 3000.0f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;
    ProjectileMovement->ProjectileGravityScale = 0.0f;

    LifeSpan = 5.0f;
    CurrentLifeTime = 0.0f;
    bHit = false;
    CurrentState = FIrGr_GameplayTags::Get().Projectile_State_Inactive;
}

void AIrGr_Projectile::BeginPlay()
{
    Super::BeginPlay();
}

void AIrGr_Projectile::Initialize(const FProjectileData& Data)
{
    ProjectileData = Data;
    CurrentLifeTime = 0.0f;
    if (ProjectileMovement)
    {
        ProjectileMovement->InitialSpeed = Data.Speed;
        ProjectileMovement->MaxSpeed = Data.Speed;
        ProjectileMovement->SetActive(true);
    }
    DamageEffectParams = Data.DamageEffectParams;
    SetInstigator(Data.Instigator);
    OwningActor = Data.Instigator;

    bHit = false;
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);
    
    if (ProjectileMovement)
    {
        ProjectileMovement->SetActive(true);
    }
    if (CollisionComponent)
    {
        CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }

    CurrentState = FIrGr_GameplayTags::Get().Projectile_State_Active;
}

void AIrGr_Projectile::UpdateProjectile(float DeltaTime)
{
    if (bHit) return;

    CurrentLifeTime += DeltaTime;
    if (CurrentLifeTime >= LifeSpan)
    {
        OnHit();
    }
}

bool AIrGr_Projectile::ShouldDeactivate() const
{
    return (CurrentLifeTime >= LifeSpan) || bHit;
}

void AIrGr_Projectile::ResetProjectile()
{
    CurrentLifeTime = 0.0f;
    SetActorLocation(FVector::ZeroVector);
    SetActorRotation(FRotator::ZeroRotator);
    
    if (ProjectileMovement)
    {
        ProjectileMovement->Velocity = FVector::ZeroVector;
        ProjectileMovement->SetActive(false);
    }
    
    OwningActor = nullptr;
    SetInstigator(nullptr);
    bHit = false;
    DamageEffectParams = FDamageEffectParams();
    
    CurrentState = FIrGr_GameplayTags::Get().Projectile_State_Inactive;
}

void AIrGr_Projectile::DeactivateProjectile()
{
    CurrentState = FIrGr_GameplayTags::Get().Projectile_State_Inactive;
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
    if (ProjectileMovement)
    {
        ProjectileMovement->SetActive(false);
    }
    
    SetActorLocation(FVector::ZeroVector);
    SetActorRotation(FRotator::ZeroRotator);
}

void AIrGr_Projectile::SetInstigator(AActor* InInstigator)
{
    if (APawn* PawnInstigator = Cast<APawn>(InInstigator))
    {
        Super::SetInstigator(PawnInstigator);
    }
    else
    {
        OwningActor = InInstigator;
    }
}

AActor* AIrGr_Projectile::GetOwningActor() const
{
    return GetInstigator() ? GetInstigator() : OwningActor;
}

void AIrGr_Projectile::OnHit()
{
    if (bHit) return;

    bHit = true;
    CurrentState = FIrGr_GameplayTags::Get().Projectile_State_Exploding;
    DeactivateProjectile();
}

void AIrGr_Projectile::SetProjectileType(const FGameplayTag& TypeTag)
{
    ProjectileTypeTag = TypeTag;
}

FGameplayTag AIrGr_Projectile::GetProjectileState() const
{
    return CurrentState;
}

void AIrGr_Projectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != GetOwningActor())
    {
        HandleImpact(OtherActor, SweepResult);
    }
}

bool AIrGr_Projectile::IsValidOverlap(AActor* OtherActor) const
{
    return OtherActor && OtherActor != GetOwningActor() && !bHit;
}

void AIrGr_Projectile::HandleImpact(AActor* OtherActor, const FHitResult& SweepResult)
{
    UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
    if (!TargetASC)
    {
        UE_LOG(LogTemp, Warning, TEXT("AIrGr_Projectile: TargetASC is null for actor %s. Cannot apply damage."), *OtherActor->GetName());
    }
    else if (UIrGr_AbilitySystemLibrary::IsNotFriend(GetOwningActor(), OtherActor))
    {
        UE_LOG(LogTemp, Log, TEXT("AIrGr_Projectile: Applying damage to %s"), *OtherActor->GetName());
        DamageEffectParams.TargetAbilitySystemComponent = TargetASC;
        UIrGr_AbilitySystemLibrary::ApplyDamageEffect(DamageEffectParams);
    }

    OnHit();
}
