#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_AutoCastBase.generated.h"

UCLASS()
class PROJECT_S_API UGA_AutoCastBase : public UGameplayAbility
{
    GENERATED_BODY()


public:
    UGA_AutoCastBase();
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AutoCast")
    float AutoCastCooldown = 5.0f;

    virtual bool CanActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayTagContainer* SourceTags,
        const FGameplayTagContainer* TargetTags,
        FGameplayTagContainer* OptionalRelevantTags) const override;

    virtual void OnGiveAbility(
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilitySpec& Spec) override;

    virtual void OnRemoveAbility(
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilitySpec& Spec) override;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
    FGameplayTag AbilityTag;
};