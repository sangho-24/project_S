// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FloatingHPBarWidget.generated.h"

class UProgressBar;

UCLASS()
class PROJECT_S_API UFloatingHPBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    void UpdateHP(float CurrentHP, float MaxHP);

protected:
    UPROPERTY(meta = (BindWidget))
    UProgressBar* HPProgressBar;
};
