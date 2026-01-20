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
	bool bUseColorParameters = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	bool bUseCustomScales = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	FLinearColor Color01 = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	FLinearColor Color02 = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	FLinearColor Color03 = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	float EffectScale = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SFX")
	USoundBase* HitSound;
};