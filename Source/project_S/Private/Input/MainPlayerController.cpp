// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/MainPlayerController.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

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
