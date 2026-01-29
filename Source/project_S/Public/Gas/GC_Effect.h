#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "GC_Effect.generated.h"

class UNiagaraSystem;
class USoundBase;

UCLASS()
class PROJECT_S_API UGC_Effect : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	UGC_Effect();
	// Gameplay Cue가 실행될 때 호출
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* EffectVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	float EffectScale = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SFX")
	USoundBase* EffectSFX;
};