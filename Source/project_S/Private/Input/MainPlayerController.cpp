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