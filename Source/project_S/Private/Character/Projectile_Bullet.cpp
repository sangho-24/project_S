#include "Character/Projectile_Bullet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagContainer.h"

void AProjectile_Bullet::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::OnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
	if (OtherActor == GetOwner())
	{
		return;
	}

	ExecuteHitGameplayCue(OtherActor, Hit, true);
	if (HasAuthority())
	{
		Destroy();
	}
}

void AProjectile_Bullet::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	if (OtherActor == GetOwner())
	{
		return;
	}

	if (!HasAuthority())
	{
		return;
	}

	// GAS 데미지 적용
	if (OtherActor && DamageEffect)
	{	// 대상 ASC 가져오기
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
		if (TargetASC)
		{
			// 공격자 ASC 가져오기
			UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
			// Context 생성(공격자 ASC우선, 없으면 타겟ASC에 생성)
			FGameplayEffectContextHandle ContextHandle = SourceASC ? SourceASC->MakeEffectContext() : TargetASC->MakeEffectContext();
			ContextHandle.AddHitResult(SweepResult);
			ContextHandle.AddInstigator(GetOwner(), this);

			// Spec 생성
			FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(DamageEffect, 1.0f, ContextHandle);
			if (SpecHandle.IsValid())
			{
				FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(TEXT("Data.Damage"));
				SpecHandle.Data.Get()->SetSetByCallerMagnitude(DamageTag, Damage);

				// GE 적용
				TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
	// Gameplay Cue 실행
	ExecuteHitGameplayCue(OtherActor, SweepResult, false);
	Destroy();
}
