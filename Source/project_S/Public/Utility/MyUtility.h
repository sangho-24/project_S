#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MyUtility.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;

UCLASS()
class PROJECT_S_API UMyUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "GAS")
	static void ModifyGold(UAbilitySystemComponent* ASC, TSubclassOf<UGameplayEffect> GoldEffect, int32 Amount);
	
};
