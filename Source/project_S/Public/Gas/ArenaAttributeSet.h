// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "ArenaAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class PROJECT_S_API UArenaAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
    UArenaAttributeSet();

    ATTRIBUTE_ACCESSORS(UArenaAttributeSet, CurrentHP)
    ATTRIBUTE_ACCESSORS(UArenaAttributeSet, MaxHP)
protected:
    // 현재 체력
    UPROPERTY(BlueprintReadOnly, Category = "Attributes")
    FGameplayAttributeData CurrentHP;

    // 최대 체력
    UPROPERTY(BlueprintReadOnly, Category = "Attributes")
    FGameplayAttributeData MaxHP;
};