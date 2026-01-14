// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "ActiveGameplayEffectHandle.h"
#include "InventoryItem.generated.h"

class UItemTemplate;

USTRUCT(BlueprintType)
struct FInventoryItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UItemTemplate* ItemTemplate = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StackCount = 1;

	// GAS에서 부여한 어빌리티 핸들 (제거용)
	TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;

	// GAS에서 적용한 이펙트 핸들 (제거용)
	TArray<FActiveGameplayEffectHandle> ActiveEffectHandles;

	FInventoryItem()
		: ItemTemplate(nullptr)
		, StackCount(1)
	{
	}

	FInventoryItem(UItemTemplate* InItemTemplate, int32 InStackCount = 1)
		: ItemTemplate(InItemTemplate)
		, StackCount(InStackCount)
	{
	}

	bool IsValid() const
	{
		return ItemTemplate != nullptr && StackCount > 0;
	}
};