// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/CharBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "AbilitySystemComponent.h"
#include "Kismet/KismetSystemLibrary.h" // 추가
#include "Gas/ArenaAttributeSet.h"


// Sets default values
ACharBase::ACharBase()
{
    PrimaryActorTick.bCanEverTick = true;
    // 리플리케이션 활성화
    bReplicates = true;

    // 캐릭터 회전이 카메라에 영향을 주지 않도록 설정
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // GAS 컴포넌트 생성 및 설정
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    // AttributeSet 생성
    AttributeSet = CreateDefaultSubobject<UArenaAttributeSet>(TEXT("AttributeSet"));

    // Sphere를 RootComponent로 설정하고 물리 활성화
    Sphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sphere"));
    RootComponent = Sphere;
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
    if (SphereMeshAsset.Succeeded())
    {
        Sphere->SetStaticMesh(SphereMeshAsset.Object);
    }
    
    // Sphere에 물리 적용
    Sphere->SetSimulatePhysics(true);
    Sphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Sphere->SetMassOverrideInKg(NAME_None, 300.0f, true);

    // 카메라 루트를 Sphere에 부착
    CameraRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CameraRoot"));
    CameraRoot->SetupAttachment(Sphere);

    // 스프링암을 CameraRoot에 부착
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
    SpringArm->SetupAttachment(CameraRoot);
    SpringArm->TargetArmLength = 1000.0f;
    // SpringArm->SocketOffset = FVector(0.0f, 0.0f, 60.0f); // 높이 조정
    SpringArm->SetWorldRotation(FRotator(-45.0f, 0.0f, 0.0f)); // 각도 조정
    SpringArm->bInheritPitch = false;
    SpringArm->bInheritYaw = false;
    SpringArm->bInheritRoll = false;

    // 카메라를 SpringArm에 부착
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);
    Camera->bUsePawnControlRotation = false;
}

// Called when the game starts or when spawned
void ACharBase::BeginPlay()
{
    Super::BeginPlay();
    if (!JumpAbilityTag.IsValid())
    {
        JumpAbilityTag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Jump"));
    }
    if (!BasicShotAbilityTag.IsValid())
    {
        BasicShotAbilityTag = FGameplayTag::RequestGameplayTag(TEXT("Ability.BasicShot"));
    }
}

// 서버에서만 호출
void ACharBase::PossessedBy(AController* NewController) 
{
    Super::PossessedBy(NewController);
    if (AbilitySystemComponent)
    {
        InitializeAbilitySystem();
    }
}

// 클라이언트에서만 호출
void ACharBase::OnRep_PlayerState() 
{
    Super::OnRep_PlayerState();
    if (AbilitySystemComponent)
    {
        InitializeAbilitySystem();
    }
}

void ACharBase::InitializeAbilitySystem()
{
    AbilitySystemComponent->InitAbilityActorInfo(this, this);
    GiveStartingAbilities();
    UE_LOG(LogTemp, Display, TEXT("화긴"));
    // Attribute 값 확인
    if (AttributeSet && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow,
            FString::Printf(TEXT("CurrentHP: %.1f / MaxHP: %.1f"),
                AttributeSet->GetCurrentHP(), AttributeSet->GetMaxHP()));
    }
    else
    {
        UE_LOG(LogTemp, Display, TEXT("안돼유~!~!~!~!~!"));
    }
}

void ACharBase::GiveStartingAbilities()
{
    if (HasAuthority() && AbilitySystemComponent)
    {
        for (TSubclassOf<UGameplayAbility>& Ability : StartingAbilities)
        {
            if (Ability)
            {
                AbilitySystemComponent->GiveAbility(
                    FGameplayAbilitySpec(Ability, 1, INDEX_NONE, this));
            }
        }
    }
}

// Called every frame
void ACharBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    ApplyHorizontalDamping(DeltaTime);
}

void ACharBase::ApplyHorizontalDamping(float DeltaTime)
{
    if (!Sphere || !Sphere->IsSimulatingPhysics())
    {
        return;
    }


    // 수평 속도 감쇠
    FVector CurrentVelocity = Sphere->GetPhysicsLinearVelocity();
    FVector HorizonVelocity(CurrentVelocity.X, CurrentVelocity.Y, 0.0f);

    if (!HorizonVelocity.IsNearlyZero())
    {
        if (!FMath::IsNearlyZero(CurrentInputVector.Y))
        {
            HorizonVelocity.X = HorizonVelocity.X * (1.0f - FMath::Abs(CurrentInputVector.Y));
        }
        if (!FMath::IsNearlyZero(CurrentInputVector.X))
        {
            HorizonVelocity.Y = HorizonVelocity.Y * (1.0f - FMath::Abs(CurrentInputVector.X));
        }
        FVector DragForce = -HorizonVelocity * DragPowerRate 
            + (-HorizonVelocity.GetSafeNormal()) * DragPowerFlat;
        Sphere->AddForce(DragForce * DeltaTime, NAME_None, true);
    }

    // 회전 감쇠(Z축만)
    const FVector AngularVelocity = Sphere->GetPhysicsAngularVelocityInRadians();
    if (!FMath::IsNearlyZero(AngularVelocity.Z))
    {
        const FVector AngularDrag(0.0f, 0.0f, -AngularVelocity.Z * AngularDragPower);
        Sphere->AddTorqueInRadians(AngularDrag * DeltaTime, NAME_None, true);
    }

    // 속도 표시
    int32 UniqueKey = GetUniqueID();
    int32 UniqueKey2 = GetUniqueID();
    FString ModeString = HasAuthority() ? TEXT("리슨 서버") : TEXT("클라이언트");
    if (GEngine && IsLocallyControlled())
    {
        GEngine->AddOnScreenDebugMessage(1, 5.f,
            HasAuthority() ? FColor::Blue : FColor::Green,
            FString::Printf(TEXT("%s 속도 %f / %f"),
                *ModeString, CurrentVelocity.X, CurrentVelocity.Y));
        GEngine->AddOnScreenDebugMessage(2, 5.f,
            HasAuthority() ? FColor::Blue : FColor::Green,
            FString::Printf(TEXT("%s CurrentInputVector %f / %f"),
                *ModeString, CurrentInputVector.X, CurrentInputVector.Y));
    }
}

// Called to bind functionality to input
void ACharBase::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    if (UEnhancedInputComponent *EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACharBase::Move);
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ACharBase::MoveCompleted);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharBase::Jump);
    }
}

void ACharBase::Move(const FInputActionValue &Value)
{
    FVector2D InputVector = Value.Get<FVector2D>();
    CurrentInputVector = InputVector.GetSafeNormal();

    if (GetLocalRole() == ROLE_Authority)
    {
        MovementImpulse(InputVector);
    }
    else
    {
        ServerMovementImpulse(InputVector);
    }
}

void ACharBase::MoveCompleted()
{
    CurrentInputVector = FVector2D::ZeroVector;
    if (GetLocalRole() != ROLE_Authority)
    {
        ServerMoveCompleted();
    }
}

void ACharBase::MovementImpulse(FVector2D InputVector)
{
    if (!Sphere || !Sphere->IsSimulatingPhysics())
    {
        return;
    }
    FVector ImpulseDirection = FVector::ForwardVector * InputVector.Y + FVector::RightVector * InputVector.X;
    ImpulseDirection.Z = 0.0f;
    ImpulseDirection = ImpulseDirection.GetSafeNormal();

    if (!ImpulseDirection.IsNearlyZero())
    {
        FVector CurrentVelocity = Sphere->GetPhysicsLinearVelocity();
        FVector CurrentHorizontalVelocity = FVector(CurrentVelocity.X, CurrentVelocity.Y, 0.0f);

        float InputDirectionSpeed = FVector::DotProduct(CurrentHorizontalVelocity, ImpulseDirection);
        if (InputDirectionSpeed < MaxSpeed)
        {
            Sphere->AddImpulse(ImpulseDirection * ImpulseStrength, NAME_None, false);
        }
    }
}

void ACharBase::ServerMovementImpulse_Implementation(FVector2D InputVector)
{
    CurrentInputVector = InputVector.GetSafeNormal();
    MovementImpulse(InputVector);
}

void ACharBase::ServerMoveCompleted_Implementation()
{
    CurrentInputVector = FVector2D::ZeroVector;
}

void ACharBase::Jump(const FInputActionValue &Value)
{
    if (!AbilitySystemComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("ASC 없음"));
        return;
    }
    
    const bool bOk = AbilitySystemComponent->TryActivateAbilitiesByTag(FGameplayTagContainer(JumpAbilityTag),true);
    if (GEngine)
    GEngine->AddOnScreenDebugMessage(-1, 5.f, bOk ? FColor::Blue : FColor::Red, FString::Printf(TEXT("쩜프 %s"), bOk ? TEXT("띠용") : TEXT("실패")));
    UE_LOG(LogTemp, Warning, TEXT("쩜프 %s"), bOk ? TEXT("띠용") : TEXT("실패"));
    FString TagString = JumpAbilityTag.ToString();
    UE_LOG(LogTemp, Warning, TEXT("%s"), *TagString);
}

void ACharBase::BasicShot(const FInputActionValue& Value)
{
    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("빵이예요~!")));
}

UAbilitySystemComponent* ACharBase::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}