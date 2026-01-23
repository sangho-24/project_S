#pragma once

#include "CoreMinimal.h"
#include "Character/ProjectileBase.h"
#include "Projectile_BouncingBlade.generated.h"


UCLASS()
class PROJECT_S_API AProjectile_BouncingBlade : public AProjectileBase
{
	GENERATED_BODY()

public:
	AProjectile_BouncingBlade();

protected:
	int32 CurrentBounces = 0;
protected:
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit) override;

	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
};