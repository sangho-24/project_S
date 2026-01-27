// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "ProjectileBase.generated.h"

#define ECC_Projectile ECC_GameTraceChannel1

class UBoxComponent;
class UNiagaraComponent;
class UProjectileMovementComponent;
class UGameplayEffect;

UCLASS()
class PROJECT_S_API AProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectileBase();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* NiagaraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;

	// 값들
	UPROPERTY(ReplicatedUsing = OnRep_Damage, BlueprintReadOnly, Category = "Projectile")
	float Damage = 5.0f;

	UPROPERTY(ReplicatedUsing = OnRep_Speed, BlueprintReadOnly, Category = "Projectile")
	float Speed = 1000.0f;

	UPROPERTY(ReplicatedUsing = OnRep_LifeSpan, BlueprintReadOnly, Category = "Projectile")
	float LifeSpan = 5.0f;

	UPROPERTY(ReplicatedUsing = OnRep_MaxBounces, BlueprintReadOnly, Category = "Projectile")
	int32 MaxBounces = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Bounciness, BlueprintReadOnly, Category = "Projectile")
	float Bounciness = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	FGameplayTag HitCueTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	FGameplayTag OverlapCueTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> DamageEffect;

public:



public:	
	// 네트워크 복제 설정
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 값 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SetDamage(float NewDamage);

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SetSpeed(float NewSpeed);

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SetProjectileLifeSpan(float NewLifeSpan);

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SetMaxBounces(int32 NewMaxBounces);

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SetBounciness(float NewBounciness);

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void Launch(const FVector& Direction);

	FORCEINLINE float GetDamage() const { return Damage; }

protected:
	// 리플리케이션 콜백
	UFUNCTION()
	void OnRep_Damage();

	UFUNCTION()
	void OnRep_Speed();

	UFUNCTION()
	void OnRep_LifeSpan();

	UFUNCTION()
	void OnRep_MaxBounces();

	UFUNCTION()
	void OnRep_Bounciness();

	// 히트 이벤트
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Gameplay Cue 실행 헬퍼
	void ExecuteHitGameplayCue(AActor* TargetActor, const FHitResult& HitResult, bool bIsHit);

};
