#include "Utility/MyUtility.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Character/FloatingDamageActor.h"
#include "Engine/World.h"

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

void UMyUtility::SpawnFloatingDamage(
	UObject* WorldContextObject,
	TSubclassOf<AFloatingDamageActor> DamageActorClass,
	FVector Location,
	float DamageAmount,
	bool bIsCritical,
	bool bIsHeal)
{
	if (!WorldContextObject || !DamageActorClass)
	{
		return;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return;
	}

	AFloatingDamageActor* DamageActor = World->SpawnActor<AFloatingDamageActor>(
		DamageActorClass,
		Location,
		FRotator::ZeroRotator
	);

	if (DamageActor)
	{
		DamageActor->Initialize(DamageAmount, bIsHeal, bIsCritical);
	}
}