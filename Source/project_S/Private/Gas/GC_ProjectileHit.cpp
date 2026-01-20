#include "Gas/GC_ProjectileHit.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"

UGC_ProjectileHit::UGC_ProjectileHit()
{
	// GameplayCueTag = FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Projectile.Hit"));
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
		UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			MyTarget->GetWorld(),
			HitEffect,
			Location,
			Rotation,
			bUseCustomScales ? FVector(1.0f, 1.0f, 1.0f) : FVector(EffectScale),
			true,
			true,
			ENCPoolMethod::AutoRelease
		);

		if (NiagaraComp)
		{
			if (bUseCustomScales)
			{
				NiagaraComp->SetVariableFloat(TEXT("Scale"), EffectScale);
			}
			if (bUseColorParameters)
			{
			NiagaraComp->SetVariableLinearColor(TEXT("Color 01"), Color01);
			NiagaraComp->SetVariableLinearColor(TEXT("Color 02"), Color02);
			NiagaraComp->SetVariableLinearColor(TEXT("Color 03"), Color03);
			}
		}
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