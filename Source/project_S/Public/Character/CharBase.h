#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbility.h"
#include "Data/InventoryComponent.h"
#include "CharBase.generated.h"

#define ECC_Projectile ECC_GameTraceChannel1

class USpringArmComponent;
class UCameraComponent;
class UBoxComponent;
class UStaticMeshComponent;
class USceneComponent;
struct FInputActionValue;
class APlayerController;
class UInputAction;
class UAbilitySystemComponent;
class UArenaAttributeSet;
class UWidgetComponent;
class UFloatingHPBarWidget;

USTRUCT()
struct FAutoCastAbilityInfo
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> SpecHandles;

	FTimerHandle TimerHandle;
};

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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* WallFadeCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UArenaAttributeSet* AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	UInventoryComponent* InventoryComponent;

// 프로텍트 컴포넌트
protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent *Sphere;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
    TArray<TSubclassOf<UGameplayAbility>> StartingAbilities;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	FGameplayTag JumpAbilityTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	FGameplayTag BasicShotAbilityTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* HPBarComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UFloatingHPBarWidget* HPBarWidget;

	FVector MouseCursorLocation;

// 인풋 액션
protected:
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction *MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction *JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* BasicShotAction;

// 멤버 변수
protected:
	ACharBase();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float MaxSpeed = 600.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float ImpulseStrength = 6000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float DragPowerRate = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float DragPowerFlat = 50.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float AngularDragPower = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	float UpdateInterval = 0.1f;



// 임시 변수
private:
    FVector2D CurrentInputVector;
	bool bIsASCInitialized = false;
	FTimerHandle ScaleUpdateTimerHandle;
	bool bIsDead = false;

// 자동시전 타이머 관련
	int32 SkillStack = 0;
	FTimerHandle AutoCastTimerHandle;
	TArray<FGameplayAbilitySpecHandle> AutoCastSpecHandles;

	TMap<FGameplayTag, FAutoCastAbilityInfo> AutoCastAbilities;

// 오버라이드 함수
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	// 틱대신 타이머
	void StartTimerUpdate();
	void StopTimerUpdate();

	void RegisterAbility(const FGameplayTag& Tag, FGameplayAbilitySpecHandle Handle);
	void UnregisterAbility(const FGameplayTag& Tag, FGameplayAbilitySpecHandle Handle);

protected:
	void StartAutoCastTimer(const FGameplayTag& Tag);
	void StopAutoCastTimer(const FGameplayTag& Tag);
	void AutoCastAbility(const FGameplayTag& Tag);

protected:
	void Move(const FInputActionValue &Value);
	void MoveCompleted();
	void MovementImpulse(FVector2D InputVector);
	void ApplyHorizontalDamping(float DeltaTime);
	void Jump(const FInputActionValue &Value);
	void BasicShot(const FInputActionValue& Value);
	void UpdateScale();



	UFUNCTION(Server, Unreliable)
	void ServerMovementImpulse(FVector2D InputVector);
	UFUNCTION(Server, Unreliable)
	void ServerMoveCompleted();


public:
	FORCEINLINE UPrimitiveComponent* GetPhysicsComponent() const { return Sphere; }
	FORCEINLINE UArenaAttributeSet* GetAttributeSet() const { return AttributeSet; }
	FORCEINLINE FVector GetMouseCursorLocation() const { return MouseCursorLocation; }
	FORCEINLINE bool GetIsDead() const { return bIsDead; }

	void Death();

private:
    void InitializeAbilitySystem();
    void GiveStartingAbilities();
	void InitializeFloatingHPBar();

	// 이벤트 핸들러
private:
	void OnHealthChanged(const FOnAttributeChangeData& Data);
};
