// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbility.h"
#include "CharBase.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class USceneComponent;
struct FInputActionValue;
class APlayerController;
class UInputAction;
class UAbilitySystemComponent;

UCLASS()
class PROJECT_S_API ACharBase : public APawn, public IAbilitySystemInterface
{
	GENERATED_BODY()

// 퍼블릭 컴포넌트
public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USpringArmComponent *SpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCameraComponent *Camera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent *CameraRoot;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UAbilitySystemComponent* AbilitySystemComponent;

// 프로텍트 컴포넌트
protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent *Sphere;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
    TArray<TSubclassOf<UGameplayAbility>> StartingAbilities;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	FGameplayTag JumpAbilityTag;

// 인풋 액션
protected:
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction *MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction *ShootAction;

// 멤버 변수
public:
	ACharBase();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float MaxSpeed = 500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float ImpulseStrength = 6000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float DragPowerRate = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float DragPowerFlat = 50.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float AngularDragPower = 60.0f;

// 임시 변수
private:
    FVector2D CurrentInputVector;

// 오버라이드 함수
protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;


// 커스텀 함수
protected:
	void Move(const FInputActionValue &Value);
	void MoveCompleted();
	void Shoot(const FInputActionValue &Value);
	void MovementImpulse(FVector2D InputVector);
	void ApplyHorizontalDamping(float DeltaTime);

	UFUNCTION(Server, Unreliable)
	void ServerMovementImpulse(FVector2D InputVector);

public:
	FORCEINLINE UPrimitiveComponent* GetPhysicsComponent() const { return Sphere; }
private:
    void InitializeAbilitySystem();
    void GiveStartingAbilities();
};
