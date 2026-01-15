#include "Input/MainPlayerController.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Widget/HUDWidget.h"
#include "Widget/InventoryWidget.h"
#include "Data/InventoryComponent.h"
#include "Character/CharBase.h"
#include "AbilitySystemComponent.h"
#include "Gas/ArenaAttributeSet.h" 

void AMainPlayerController::CreateHUD()
{
    if (HUDWidget)
    {
        return;
    }
    if (!IsLocalPlayerController())
    {
        return;
    }
    if (HUDWidgetClass)
    {
        HUDWidget = CreateWidget<UHUDWidget>(this, HUDWidgetClass);
        if (HUDWidget)
        {
            HUDWidget->AddToViewport();
            ACharBase* CharBase = Cast<ACharBase>(GetPawn());
            if (CharBase)
            {
                UAbilitySystemComponent* ASC = CharBase->GetAbilitySystemComponent();
                UArenaAttributeSet* AttributeSet = CharBase->GetAttributeSet();
                if (ASC && AttributeSet)
                {
                    ASC->GetGameplayAttributeValueChangeDelegate(
                        AttributeSet->GetCurrentHPAttribute()).AddUObject(
                            this, &AMainPlayerController::OnHealthChanged);
                    ASC->GetGameplayAttributeValueChangeDelegate(
                        AttributeSet->GetMaxHPAttribute()).AddUObject(
                            this, &AMainPlayerController::OnHealthChanged);
                    HUDWidget->UpdateHP(AttributeSet->GetCurrentHP(), AttributeSet->GetMaxHP());
                }
            }
        }
    }
}

FVector AMainPlayerController::GetMouseCursorLocation() const
{
    FVector MouseWorldLocation, MouseWorldDirection;
    DeprojectMousePositionToWorld(MouseWorldLocation, MouseWorldDirection);
    const FVector TraceEnd = MouseWorldLocation + MouseWorldDirection * 10000.f;
    FHitResult HitResult;

    if (GetWorld()->LineTraceSingleByChannel(
        HitResult, MouseWorldLocation, TraceEnd, ECC_Visibility))
    {
        return HitResult.Location;
    }
    else
    {
        // 평면과 교차점 계산
        FVector PlaneIntersectionPoint = FMath::LinePlaneIntersection(
            MouseWorldLocation,
            TraceEnd,
            FVector(0, 0, 0),
            FVector::UpVector);
        return PlaneIntersectionPoint;
    }
}

void AMainPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        for (int32 i = 0; i < DefaultMappingContexts.Num(); i++)
        {
            if (DefaultMappingContexts[i])
            {
                Subsystem->AddMappingContext(DefaultMappingContexts[i], i);
            }
        }
    }
}
void AMainPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent(); 
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
    {
        EnhancedInputComponent->BindAction(OpenInventoryAction, ETriggerEvent::Triggered, this, &AMainPlayerController::OpenInventory);
    }
}

void AMainPlayerController::OpenInventory()
{
	UE_LOG(LogTemp, Warning, TEXT("인벤토리 열기"));
    ACharBase* CharBase = Cast<ACharBase>(GetPawn());
    if (!CharBase)
    {
        return;
    }

    UInventoryComponent* InventoryComponent = CharBase->FindComponentByClass<UInventoryComponent>();
    if (!InventoryComponent)
    {
        return;
    }

    // 위젯 생성
    if (!InventoryWidget && InventoryWidgetClass)
    {
        InventoryWidget = CreateWidget<UInventoryWidget>(this, InventoryWidgetClass);
        if (InventoryWidget)
        {
            InventoryWidget->SetInventoryComponent(InventoryComponent);
        }
    }

    if (InventoryWidget)
    {
        if (InventoryWidget->IsInViewport())
        {
            // 인벤토리 닫기
            InventoryWidget->RemoveFromParent();
        }
        else
        {
            // 인벤토리 열기 (HUD보다 위에)
            InventoryWidget->AddToViewport(1);
            InventoryWidget->RefreshInventory();
        }
    }
}

void AMainPlayerController::OnHealthChanged(const FOnAttributeChangeData& Data)
{
    if (HUDWidget)
    {
        ACharBase* CharBase = Cast<ACharBase>(GetPawn());
        if (CharBase && CharBase->GetAttributeSet())
        {
            float CurrentHP = CharBase->GetAttributeSet()->GetCurrentHP();
            float MaxHP = CharBase->GetAttributeSet()->GetMaxHP();
            HUDWidget->UpdateHP(CurrentHP, MaxHP);
        }
    }
}
