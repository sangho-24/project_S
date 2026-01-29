#include "Utility/MyUtility.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"

void UMyUtility::ModifyGold(UAbilitySystemComponent* ASC, TSubclassOf<UGameplayEffect> GoldEffect, int32 Amount)
{
    if (!ASC || !GoldEffect)
    {
        return;
    }

    FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
    FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(GoldEffect, 1.0f, ContextHandle);

    if (SpecHandle.IsValid())
    {
        SpecHandle.Data.Get()->SetSetByCallerMagnitude(
            FGameplayTag::RequestGameplayTag(FName("Data.Gold")),
            static_cast<float>(Amount)
        );
        ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
    }
}