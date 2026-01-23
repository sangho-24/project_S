#include "Character/Projectile_BouncingBlade.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagContainer.h"
#include "GameFramework/ProjectileMovementComponent.h"

AProjectile_BouncingBlade::AProjectile_BouncingBlade()
{
	// 바운스 활성화
	if (ProjectileMovement)
	{
		ProjectileMovement->bShouldBounce = true;
		ProjectileMovement->Friction = 0.0f;
		ProjectileMovement->BounceVelocityStopSimulatingThreshold = 5.0f;
	}
}

void AProjectile_BouncingBlade::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::OnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);

	if (OtherActor == GetOwner())
	{
		return;
	}

	CurrentBounces++;
	if (CurrentBounces >= MaxBounces)
	{
		ExecuteHitGameplayCue(OtherActor, Hit, true);
		if (HasAuthority())
		{
			Destroy();
		}
		return;
	}
	ExecuteHitGameplayCue(OtherActor, Hit, true);
}

void AProjectile_BouncingBlade::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (OtherActor == GetOwner() || !HasAuthority())
	{
		return;
	}

	// GAS 데미지 적용
	if (OtherActor && DamageEffect)
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
		if (TargetASC)
		{
			UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
			FGameplayEffectContextHandle ContextHandle = SourceASC ? SourceASC->MakeEffectContext() : TargetASC->MakeEffectContext();
			ContextHandle.AddHitResult(SweepResult);
			ContextHandle.AddInstigator(GetOwner(), this);

			FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(DamageEffect, 1.0f, ContextHandle);
			if (SpecHandle.IsValid())
			{
				FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(TEXT("Data.Damage"));
				SpecHandle.Data.Get()->SetSetByCallerMagnitude(DamageTag, Damage);
				TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
	ExecuteHitGameplayCue(OtherActor, SweepResult, false);
}