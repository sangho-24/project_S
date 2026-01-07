// Fill out your copyright notice in the Description page of Project Settings.


#include "Gas/GA_Jump.h"
#include "GameFramework/Pawn.h"
#include "Character/CharBase.h"
#include "DrawDebugHelpers.h"
#include "GameplayTagContainer.h"

UGA_Jump::UGA_Jump()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Jump")));
}

void UGA_Jump::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle, 
    const FGameplayAbilityActorInfo *ActorInfo, 
    const FGameplayAbilityActivationInfo ActivationInfo, 
    const FGameplayEventData *TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    ACharBase* Character = Cast<ACharBase>(ActorInfo->AvatarActor.Get());
    if (Character && Character->GetPhysicsComponent())
    {
        // Z 속도 제거
        FVector Vel = Character->GetPhysicsComponent()->GetPhysicsLinearVelocity();
        Vel.Z = 0.f;
        Character->GetPhysicsComponent()->SetPhysicsLinearVelocity(Vel, false, NAME_None);        
        // 위쪽으로 임펄스 적용
        FVector JumpImpulse = FVector(0.0f, 0.0f, JumpForce);
        Character->GetPhysicsComponent()->AddImpulse(JumpImpulse, NAME_None, true);
    }
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

bool UGA_Jump::CanActivateAbility(
    const FGameplayAbilitySpecHandle Handle, 
    const FGameplayAbilityActorInfo *ActorInfo, 
    const FGameplayTagContainer *SourceTags, 
    const FGameplayTagContainer *TargetTags, 
    FGameplayTagContainer *OptionalRelevantTags) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }

    // 바닥인지 확인(레이트레이스)
    ACharBase* Character = Cast<ACharBase>(ActorInfo->AvatarActor.Get());
    if (Character && Character->GetPhysicsComponent())
    {
        FVector TraceStart = Character->GetPhysicsComponent()->GetComponentLocation();
        FVector TraceEnd = TraceStart + FVector(0.0f, 0.0f, -100.0f);
        
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(Character);
        
        bool bHit = Character->GetPhysicsComponent()->GetWorld()->LineTraceSingleByChannel(
            HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams);
        
        // 디버그 드로잉
        #if ENABLE_DRAW_DEBUG
        DrawDebugLine(
            Character->GetPhysicsComponent()->GetWorld(),
            TraceStart,
            TraceEnd,
            bHit ? FColor::Green : FColor::Red,
            false,
            2.0f,
            0,
            2.0f
        );

        if (bHit)
        {
            DrawDebugSphere(
                Character->GetPhysicsComponent()->GetWorld(),
                HitResult.ImpactPoint,
                10.0f,
                12,
                FColor::Green,
                false,
                2.0f
            );
        }
        #endif
        
        if (!bHit)
        {
            return false;
        }
    }
return true;
}

void UGA_Jump::InputReleased(
    const FGameplayAbilitySpecHandle Handle, 
    const FGameplayAbilityActorInfo *ActorInfo, 
    const FGameplayAbilityActivationInfo ActivationInfo)
{
    if (ActorInfo != nullptr && ActorInfo->AvatarActor != nullptr)
    {
        CancelAbility(Handle, ActorInfo, ActivationInfo, true);
    }
}

void UGA_Jump::CancelAbility(
    const FGameplayAbilitySpecHandle Handle, 
    const FGameplayAbilityActorInfo *ActorInfo, 
    const FGameplayAbilityActivationInfo ActivationInfo, 
    bool bReplicateCancelAbility)
{
    Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}
