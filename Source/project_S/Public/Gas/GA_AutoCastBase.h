#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_AutoCastBase.generated.h"

UCLASS()
class PROJECT_S_API UGA_AutoCastBase : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AutoCast")
    float AutoCastCooldown = 5.0f;
};