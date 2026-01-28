#include "Input/MainPlayerController.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Widget/HUDWidget.h"
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

                // 스탯 델리게이트 바인딩
                UArenaAttributeSet* AttributeSet = CharBase->GetAttributeSet();
                if (ASC && AttributeSet)
                {
                    ASC->GetGameplayAttributeValueChangeDelegate(
                        AttributeSet->GetCurrentHPAttribute()).AddUObject(
                            this, &AMainPlayerController::OnHealthChanged);
                    ASC->GetGameplayAttributeValueChangeDelegate(
                        AttributeSet->GetMaxHPAttribute()).AddUObject(
                            this, &AMainPlayerController::OnHealthChanged);
					ASC->GetGameplayAttributeValueChangeDelegate(
						AttributeSet->GetAttackPowerAttribute()).AddUObject(
							this, &AMainPlayerController::OnStatsChanged);
					ASC->GetGameplayAttributeValueChangeDelegate(
						AttributeSet->GetDefenseAttribute()).AddUObject(
							this, &AMainPlayerController::OnStatsChanged);

					ASC->GetGameplayAttributeValueChangeDelegate(
						AttributeSet->GetGoldAttribute()).AddUObject(
							this, &AMainPlayerController::OnGoldChanged);

					// 초기 HUD 업데이트
                    HUDWidget->UpdateHP(AttributeSet->GetCurrentHP(), AttributeSet->GetMaxHP());
                    HUDWidget->UpdateGold(AttributeSet->GetGold());
                    HUDWidget->UpdateStats(AttributeSet);
                }

                // 인벤토리 초기화
                UInventoryComponent* InventoryComponent = CharBase->FindComponentByClass<UInventoryComponent>();
                if (InventoryComponent)
                {
                    HUDWidget->InitializeInventory(InventoryComponent);
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
        HitResult, MouseWorldLocation, TraceEnd, ECC_GameTraceChannel2))
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
    //if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
    //{
    //}
}


void AMainPlayerController::OnHealthChanged(const FOnAttributeChangeData& Data)
{
    if (!HUDWidget)
    {
        return;
    }
    const ACharBase* CharBase = Cast<ACharBase>(GetPawn());
    if (!CharBase)
    {
        return;
    }
    const UArenaAttributeSet* AttributeSet = CharBase->GetAttributeSet();
    if (!AttributeSet)
    {
        return;
    }
    float CurrentHP = CharBase->GetAttributeSet()->GetCurrentHP();
    float MaxHP = CharBase->GetAttributeSet()->GetMaxHP();
    HUDWidget->UpdateHP(CurrentHP, MaxHP);
}

void AMainPlayerController::OnStatsChanged(const FOnAttributeChangeData& Data)
{
    if (!HUDWidget)
    {
        return;
    }
    const ACharBase* CharBase = Cast<ACharBase>(GetPawn());
    if (!CharBase)
    {
        return;
    }
    const UArenaAttributeSet* AttributeSet = CharBase->GetAttributeSet();
    if (!AttributeSet)
    {
        return;
    }
    HUDWidget->UpdateStats(AttributeSet);
}

void AMainPlayerController::OnGoldChanged(const FOnAttributeChangeData& Data)
{
    if (!HUDWidget)
    {
        return;
    }
    const ACharBase* CharBase = Cast<ACharBase>(GetPawn());
    if (!CharBase)
    {
        return;
    }
    const UArenaAttributeSet* AttributeSet = CharBase->GetAttributeSet();
    if (!AttributeSet)
    {
        return;
    }
    HUDWidget->UpdateGold(AttributeSet->GetGold());
}