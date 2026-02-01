#pragma once

#include "CoreMinimal.h"
#include "Gas/GA_AutoCastBase.h"
#include "GA_OrbitOrb.generated.h"

class AProjectileBase;

UCLASS()
class PROJECT_S_API UGA_OrbitOrb : public UGA_AutoCastBase
{
	GENERATED_BODY()
	
public:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<AProjectileBase> ProjectileClass;

	// 오브 개수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	int32 OrbitCount = 3;

	// 회전 반경
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float OrbitRadius = 200.0f;

	// 회전 속도 (초당 각도)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float OrbitSpeed = 180.0f;

	// 지속 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float ProjectileLifeSpan = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float ProjectileDamage = 10.0f;
};
