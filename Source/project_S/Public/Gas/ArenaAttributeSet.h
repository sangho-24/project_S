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

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
    virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

    ATTRIBUTE_ACCESSORS(UArenaAttributeSet, CurrentHP)
    ATTRIBUTE_ACCESSORS(UArenaAttributeSet, MaxHP)
    ATTRIBUTE_ACCESSORS(UArenaAttributeSet, AttackPower)
    ATTRIBUTE_ACCESSORS(UArenaAttributeSet, Defense)
    ATTRIBUTE_ACCESSORS(UArenaAttributeSet, Gold)

protected:
    UPROPERTY(ReplicatedUsing = OnRep_CurrentHP, BlueprintReadOnly, Category = "Attributes")
    FGameplayAttributeData CurrentHP;

    UPROPERTY(ReplicatedUsing = OnRep_MaxHP, BlueprintReadOnly, Category = "Attributes")
    FGameplayAttributeData MaxHP;

    UPROPERTY(ReplicatedUsing = OnRep_AttackPower, BlueprintReadOnly, Category = "Attributes")
    FGameplayAttributeData AttackPower;

    UPROPERTY(ReplicatedUsing = OnRep_Defense, BlueprintReadOnly, Category = "Attributes")
    FGameplayAttributeData Defense;

    UPROPERTY(ReplicatedUsing = OnRep_Gold, BlueprintReadOnly, Category = "Currency")
    FGameplayAttributeData Gold;

private:
    float CachedHPPercent = 1.0f;

protected:
    UFUNCTION()
    void OnRep_CurrentHP(const FGameplayAttributeData& OldCurrentHP);

    UFUNCTION()
    void OnRep_MaxHP(const FGameplayAttributeData& OldMaxHP);

    UFUNCTION()
    void OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower);

    UFUNCTION()
    void OnRep_Defense(const FGameplayAttributeData& OldDefense);

    UFUNCTION()
    void OnRep_Gold(const FGameplayAttributeData& OldGold);
};