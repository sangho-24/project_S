#pragma once

#include "CoreMinimal.h"
#include "Character/ProjectileBase.h"
#include "Projectile_Orbit.generated.h"

UCLASS()
class PROJECT_S_API AProjectile_Orbit : public AProjectileBase
{
	GENERATED_BODY()

public:
	void Initialize(float InRadius, float InSpeed, float InAngleOffset);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void Tick(float DeltaTime) override;

	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Orbit")
	float OrbitRadius = 200.0f;
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Orbit")
	float OrbitSpeed = 180.0f;
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Orbit")
	float CurrentAngle = 0.0f;
};