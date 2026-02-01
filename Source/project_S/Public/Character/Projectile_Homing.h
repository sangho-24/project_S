#pragma once

#include "CoreMinimal.h"
#include "Character/ProjectileBase.h"
#include "Projectile_Homing.generated.h"

class ACharBase;


UCLASS()
class PROJECT_S_API AProjectile_Homing : public AProjectileBase
{
	GENERATED_BODY()

public:
	AProjectile_Homing();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit) override;

	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	void UpdateHomingTarget();
	ACharBase* FindNearestEnemy() const;

public:
	UFUNCTION(BlueprintCallable, Category = "Homing")
	void SetHomingPower(float NewHomingPower);

	UFUNCTION(BlueprintCallable, Category = "Homing")
	void SetHomingRadius(float NewHomingRadius);

private:
	FTimerHandle TargetUpdateTimerHandle;
	TWeakObjectPtr<ACharBase> CurrentTarget;

protected:
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Homing")
	float HomingPower = 800.0f;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Homing")
	float HomingRadius = 2000.0f;

};