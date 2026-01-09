// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileBase.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;

UCLASS()
class PROJECT_S_API AProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectileBase();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;

	// 값들
	UPROPERTY(BlueprintReadOnly, Category = "Projectile")
	float Damage = 10.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Projectile")
	float Speed = 2000.0f;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float LifeSpan = 5.0f;


public:	
	virtual void Tick(float DeltaTime) override;
	
	// 값 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SetDamage(float NewDamage);

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SetSpeed(float NewSpeed);

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void Launch(const FVector& Direction);

	FORCEINLINE float GetDamage() const { return Damage; }

protected:
	// 히트 이벤트
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

};
