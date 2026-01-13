#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "GC_ProjectileHit.generated.h"

class UNiagaraSystem;
class USoundBase;

UCLASS()
class PROJECT_S_API UGC_ProjectileHit : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	UGC_ProjectileHit();
	// Gameplay Cue가 실행될 때 호출
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* HitEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	FVector HitEffectScale = FVector(1.0f, 1.0f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SFX")
	USoundBase* HitSound;
};