#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MyUtility.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;
class AFloatingDamageActor;

UCLASS()
class PROJECT_S_API UMyUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static void ModifyGold(UAbilitySystemComponent* ASC, TSubclassOf<UGameplayEffect> GoldEffect, int32 Amount);
	
	UFUNCTION(BlueprintCallable)
	static void SpawnFloatingDamage(
		UObject* WorldContextObject,
		TSubclassOf<AFloatingDamageActor> DamageActorClass,
		FVector Location,
		float DamageAmount,
		bool bIsHeal = false,
		bool bIsCritical = false);
};
