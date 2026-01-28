// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/CharBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "AbilitySystemComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Gas/ArenaAttributeSet.h"
#include "Gas/GA_AutoCastBase.h"
#include "Input/MainPlayerController.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Components/WidgetComponent.h"
#include "Widget/FloatingHPBarWidget.h"
#include "Character/ItemShop.h"



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

	// Inventory 생성
    InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
    
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
    Sphere->SetCollisionObjectType(ECC_Pawn);
    Sphere->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    Sphere->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
    Sphere->SetMassOverrideInKg(NAME_None, 300.0f, true);

    // 카메라 루트를 Sphere에 부착
    CameraRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CameraRoot"));
    CameraRoot->SetUsingAbsoluteRotation(true);
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

    // 벽 투명 오버랩박스
    WallFadeCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("WallFadeCollision"));
    WallFadeCollision->SetupAttachment(CameraRoot);
    WallFadeCollision->SetRelativeLocation(FVector(-150.0f, 0.0f, 0.0f));
    WallFadeCollision->SetBoxExtent(FVector(150.0f, 32.0f, 32.0f));
    WallFadeCollision->SetCollisionObjectType(ECC_WorldDynamic);
    WallFadeCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
    WallFadeCollision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
    WallFadeCollision->SetGenerateOverlapEvents(true);

    // 위젯 컴포넌트 생성
    HPBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBarComponent"));
    HPBarComponent->SetupAttachment(GetRootComponent());


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
        BasicShotAbilityTag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Attack.BasicShot"));
    }
    if (AbilitySystemComponent)
    {
        InitializeAbilitySystem();
    }
    StartTimerUpdate();
}

void ACharBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopTimerUpdate();
    Super::EndPlay(EndPlayReason);
}

// 서버에서만 호출
void ACharBase::PossessedBy(AController* NewController) 
{
    Super::PossessedBy(NewController);
    if (AbilitySystemComponent)
    {
        InitializeAbilitySystem();
        ApplyPassiveGoldIncomeGE();
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

void ACharBase::Death()
{
    if (bIsDead)
    {
        return;
    }
	UE_LOG(LogTemp, Warning, TEXT("!@펑@!@펑@! [죽음] !@펑@!@펑@!"));
    bIsDead = true;
}

void ACharBase::SetInShop(bool bInShop, AItemShop* Shop)
{
    bIsInShop = bInShop;
    CurrentShop = Shop;
}

void ACharBase::InitializeAbilitySystem()
{
    if (bIsASCInitialized)
    {
        return;
    }
    AbilitySystemComponent->InitAbilityActorInfo(this, this);
    GiveStartingAbilities();
    if (AMainPlayerController* PC = Cast<AMainPlayerController>(GetController()))
    {
        PC->CreateHUD();
    }
    if (HPBarComponent)
    {
        InitializeFloatingHPBar();
    }
    bIsASCInitialized = true;
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
        EnhancedInputComponent->BindAction(BasicShotAction, ETriggerEvent::Triggered, this, &ACharBase::BasicShot);
    }
}

UAbilitySystemComponent* ACharBase::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void ACharBase::StartTimerUpdate()
{
    if (UWorld* World = GetWorld())
    {
        FTimerManager& TimerManager = World->GetTimerManager();
        if (!TimerManager.IsTimerActive(ScaleUpdateTimerHandle))
        {
            UpdateScale();
            TimerManager.SetTimer(
                ScaleUpdateTimerHandle,
                this,
                &ACharBase::UpdateScale,
                UpdateInterval,
                true);
        }
    }
}

void ACharBase::StopTimerUpdate()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ScaleUpdateTimerHandle);
    }
}

void ACharBase::RegisterAbility(const FGameplayTag& Tag, FGameplayAbilitySpecHandle Handle)
{
    if (!HasAuthority())
    {
        return;
    }
    FAutoCastAbilityInfo* AbilityInfo = AutoCastAbilities.Find(Tag);
    if (AbilityInfo)
    {
        // 이미 해당 태그가 존재하면 핸들만 추가
        AbilityInfo->SpecHandles.AddUnique(Handle);
        UE_LOG(LogTemp, Warning, TEXT("[캐릭터] %s 추가 / 현재 %d개"), *Tag.ToString(), AbilityInfo->SpecHandles.Num());

        // 타이머 재시작 (간격 조정)
        StopAutoCastTimer(Tag);
        StartAutoCastTimer(Tag);
    }
    else
    {
        // 새로운 태그 추가
        FAutoCastAbilityInfo NewInfo;
        NewInfo.SpecHandles.Add(Handle);
        AutoCastAbilities.Add(Tag, NewInfo);
        UE_LOG(LogTemp, Warning, TEXT("[어빌리티] %s 새로 등록: 1개"), *Tag.ToString());

        // 타이머 시작
        StartAutoCastTimer(Tag);
    }
}

void ACharBase::UnregisterAbility(const FGameplayTag& Tag, FGameplayAbilitySpecHandle Handle)
{
    if (!HasAuthority())
    {
        return;
    }

    FAutoCastAbilityInfo* AbilityInfo = AutoCastAbilities.Find(Tag);

    if (!AbilityInfo)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s 태그를 찾을 수 없음"), *Tag.ToString());
        return;
    }

    // 핸들 제거
    AbilityInfo->SpecHandles.Remove(Handle);
    UE_LOG(LogTemp, Warning, TEXT("[어빌리티] %s 제거 / 현재 %d개"), *Tag.ToString(), AbilityInfo->SpecHandles.Num());

    // 타이머 정지
    StopAutoCastTimer(Tag);

    // 핸들이 남아있으면 타이머 재시작
    if (AbilityInfo->SpecHandles.Num() > 0)
    {
        StartAutoCastTimer(Tag);
    }
    else
    {
        // 핸들이 없으면 맵에서 제거
        AutoCastAbilities.Remove(Tag);
        UE_LOG(LogTemp, Warning, TEXT("[어빌리티] %s 완전 제거"), *Tag.ToString());
    }
}

void ACharBase::StartAutoCastTimer(const FGameplayTag& Tag)
{
    FAutoCastAbilityInfo* AbilityInfo = AutoCastAbilities.Find(Tag);

    if (!AbilityInfo || AbilityInfo->SpecHandles.Num() == 0)
    {
        return;
    }

	// 쿨타임을 위해 어빌리티 스펙 가져오기
    FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(AbilityInfo->SpecHandles[0]);
    if (!Spec || !Spec->Ability)
    {
        UE_LOG(LogTemp, Error, TEXT("%s 어빌리티 스펙을 찾을 수 없음"), *Tag.ToString());
        return;
    }

    float BaseCooldown = 5.0f; // 기본값

    if (UGA_AutoCastBase* AutoCastAbility = Cast<UGA_AutoCastBase>(Spec->Ability))
    {
        BaseCooldown = AutoCastAbility->AutoCastCooldown;
    }
    else
    {
		UE_LOG(LogTemp, Warning, TEXT("AutoCast 아님"));
    }

    const float Interval = BaseCooldown / AbilityInfo->SpecHandles.Num();

    UE_LOG(LogTemp, Warning, TEXT("[어빌리티] %s 타이머 시작: Interval : %.1f (개수 : %d)"),
        *Tag.ToString(), Interval, AbilityInfo->SpecHandles.Num());

    GetWorldTimerManager().SetTimer(
        AbilityInfo->TimerHandle,
        [this, Tag]()
        {
            AutoCastAbility(Tag);
        },
        Interval,
        true,
        Interval);
}

void ACharBase::StopAutoCastTimer(const FGameplayTag& Tag)
{
    FAutoCastAbilityInfo* AbilityInfo = AutoCastAbilities.Find(Tag);

    if (AbilityInfo && AbilityInfo->TimerHandle.IsValid())
    {
        GetWorldTimerManager().ClearTimer(AbilityInfo->TimerHandle);
        UE_LOG(LogTemp, Warning, TEXT("[어빌리티] %s 타이머 중지"), *Tag.ToString());
    }
}

void ACharBase::AutoCastAbility(const FGameplayTag& Tag)
{
    FAutoCastAbilityInfo* AbilityInfo = AutoCastAbilities.Find(Tag);

    if (!AbilityInfo || AbilityInfo->SpecHandles.Num() == 0 || !AbilitySystemComponent)
    {
        StopAutoCastTimer(Tag);
		UE_LOG(LogTemp, Warning, TEXT("[어빌리티] %s 시전 불가"), *Tag.ToString());
        return;
    }

    // 첫 번째 핸들로 활성화 (모두 같은 어빌리티이므로)
    FGameplayAbilitySpecHandle Handle = AbilityInfo->SpecHandles[0];
    if (Handle.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[어빌리티] %s 자동 시전 (핸들 개수: %d)"),
            *Tag.ToString(), AbilityInfo->SpecHandles.Num());
        AbilitySystemComponent->TryActivateAbility(Handle);
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

void ACharBase::ServerBuyItemFromShop_Implementation(AItemShop* Shop, int32 ItemIndex)
{
    if (Shop)
    {
        Shop->BuyItem(ItemIndex, this);
    }
}

void ACharBase::ServerSellItemToShop_Implementation(AItemShop* Shop, int32 SlotIndex)
{
    if (Shop)
    {
        Shop->SellItem(SlotIndex, this);
    }
}

void ACharBase::Jump(const FInputActionValue &Value)
{
    if (!AbilitySystemComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("ASC 없음"));
        return;
    }
    
    AbilitySystemComponent->TryActivateAbilitiesByTag(FGameplayTagContainer(JumpAbilityTag),true);
}

void ACharBase::BasicShot(const FInputActionValue& Value)
{
    if (!AbilitySystemComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("ASC 없음"));
        return;
    }

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (PC)
    {
        MouseCursorLocation = Cast<AMainPlayerController>(PC)->GetMouseCursorLocation();
    }

    // EventData 생성
    FGameplayEventData EventData;

    // ContextHandle을 통해 위치 전달
    FGameplayAbilityTargetingLocationInfo TargetLocationInfo;
    TargetLocationInfo.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
    TargetLocationInfo.LiteralTransform = FTransform(MouseCursorLocation);

    // TargetData를 수동으로 생성
    FGameplayAbilityTargetDataHandle TargetDataHandle;
    FGameplayAbilityTargetData_LocationInfo* LocationData = new FGameplayAbilityTargetData_LocationInfo();
    LocationData->TargetLocation = TargetLocationInfo;

    TargetDataHandle.Add(LocationData);
    EventData.TargetData = TargetDataHandle;

    // HandleGameplayEvent 호출
    AbilitySystemComponent->HandleGameplayEvent(BasicShotAbilityTag, &EventData);
}

void ACharBase::UpdateScale()
{
    if(!HPBarWidget)
	{
		return;
	}
    if (const APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        FVector ViewLoc;
        FRotator ViewRot;
        PC->GetPlayerViewPoint(ViewLoc, ViewRot);
        const float Distance = FVector::Dist(
            FVector(ViewLoc.X, 0.0f, 0.0f), 
            FVector(HPBarComponent->GetComponentLocation().X, 0.0f, 0.0f));
	    HPBarWidget->UpdateScale(Distance);
    }
}

void ACharBase::ApplyPassiveGoldIncomeGE()
{
    if (!HasAuthority() || !AbilitySystemComponent)
    {
        return;
    }

    if (PassiveGoldIncomeEffect)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        float GameStartDelay = 3.0f;
        float DelayTime = FMath::Max(0.0f, GameStartDelay - CurrentTime);
        FTimerHandle GoldIncomeTimerHandle;

        GetWorld()->GetTimerManager().SetTimer(
            GoldIncomeTimerHandle,
            [this]()
            {
                if (AbilitySystemComponent && PassiveGoldIncomeEffect)
                {
                    FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
                    ContextHandle.AddSourceObject(this);

                    FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
                        PassiveGoldIncomeEffect,
                        1.0f,
                        ContextHandle
                    );

                    if (SpecHandle.IsValid())
                    {
                        AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
                    }
                }
            },
            DelayTime,
            false
        );
    }
}

void ACharBase::InitializeFloatingHPBar()
{
    if (AbilitySystemComponent && AttributeSet)
    {
        HPBarComponent->InitWidget();
        float CurrentHP = AttributeSet->GetCurrentHP();
        float MaxHP = AttributeSet->GetMaxHP();
		HPBarWidget = Cast<UFloatingHPBarWidget>(HPBarComponent->GetUserWidgetObject());
        if (HPBarWidget)
        {
            HPBarWidget->UpdateHP(CurrentHP, MaxHP);
        }

        AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
            AttributeSet->GetCurrentHPAttribute()).AddUObject(this, &ACharBase::OnHealthChanged);

        AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
            AttributeSet->GetMaxHPAttribute()).AddUObject(this, &ACharBase::OnHealthChanged);
    }
}

void ACharBase::OnHealthChanged(const FOnAttributeChangeData& Data)
{
    if (HPBarComponent&&HPBarWidget)
    {
        if (HPBarWidget && AttributeSet)
        {
            HPBarWidget->UpdateHP(AttributeSet->GetCurrentHP(), AttributeSet->GetMaxHP());
        }
    }
}