#include "Gas/GC_ProjectileHit.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

UGC_ProjectileHit::UGC_ProjectileHit()
{
	GameplayCueTag = FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Projectile.Hit"));
}

bool UGC_ProjectileHit::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget || !MyTarget->GetWorld())
	{
		return false;
	}

	FVector Location = Parameters.Location;
	FRotator Rotation = Parameters.Normal.Rotation();

	// HitResult가 있으면 그 정보 사용
	if (Parameters.EffectContext.GetHitResult())
	{
		const FHitResult* HitResult = Parameters.EffectContext.GetHitResult();
		Location = HitResult->ImpactPoint;
		Rotation = HitResult->ImpactNormal.Rotation();
	}

	// 나이아가라 이펙트 재생
	if (HitEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			MyTarget->GetWorld(),
			HitEffect,
			Location,
			Rotation,
			HitEffectScale,
			true,
			true,
			ENCPoolMethod::AutoRelease
		);
	}

	// 사운드 재생
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			MyTarget,
			HitSound,
			Location
		);
	}

	return true;
}