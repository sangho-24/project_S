#pragma once

#include "CoreMinimal.h"
#include "Gas/GA_AutoCastBase.h"
#include "GA_HomingShot.generated.h"

class AProjectileBase;
class ACharBase;

UCLASS()
class PROJECT_S_API UGA_HomingShot : public UGA_AutoCastBase
{
    GENERATED_BODY()

public:
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
    TSubclassOf<AProjectileBase> ProjectileClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
    int32 ProjectileCount = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
    float ProjectileSpeed = 600.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
    float ProjectileDamage = 8.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
    float AttackPowerMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
    float ProjectileLifeSpan = 5.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
    float TargetSearchRadius = 2000.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Homing")
    float HomingPower = 800.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Homing")
    float HomingRadius = 2000.0f;

private:
    FVector FindTargetDirection(ACharBase* Character, const FVector& SpawnLocation) const;
};