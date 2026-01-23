#include "Gas/GA_AutoCastBase.h"
#include "Character/CharBase.h"

UGA_AutoCastBase::UGA_AutoCastBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

bool UGA_AutoCastBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	ACharBase* Character = Cast<ACharBase>(ActorInfo->AvatarActor.Get());
	if (Character && Character->GetIsDead())
	{
		return false;
	}

	return true;
}

void UGA_AutoCastBase::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);
	ACharBase* Character = Cast<ACharBase>(ActorInfo->AvatarActor.Get());
	if (Character)
	{
		Character->RegisterAbility(AbilityTag, Spec.Handle);
	}
}

void UGA_AutoCastBase::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	ACharBase* Character = Cast<ACharBase>(ActorInfo->AvatarActor.Get());
	if (Character)
	{
		Character->UnregisterAbility(AbilityTag, Spec.Handle);
	}
	Super::OnRemoveAbility(ActorInfo, Spec);
}
