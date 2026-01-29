#include "Gas/GC_Effect.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"


UGC_Effect::UGC_Effect()
{
}

bool UGC_Effect::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget || !MyTarget->GetWorld())
	{
		return false;
	}

	FVector Location = Parameters.Location;
	if (Location.IsZero())
	{
		Location = MyTarget->GetActorLocation();
	}
	FRotator Rotation = Parameters.Normal.Rotation();

	// 나이아가라 이펙트 재생
	if (EffectVFX)
	{
		UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			MyTarget->GetWorld(),
			EffectVFX,
			Location,
			Rotation,
			FVector(EffectScale),
			true,
			true,
			ENCPoolMethod::AutoRelease
		);
	}

	// 사운드 재생
	if (EffectSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(
			MyTarget,
			EffectSFX,
			Location
		);
	}
	return true;
}