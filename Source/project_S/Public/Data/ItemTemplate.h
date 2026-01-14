#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ItemTemplate.generated.h"

class UGameplayAbility;
class UGameplayEffect;

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Equipment UMETA(DisplayName = "장비"),
	Skill UMETA(DisplayName = "스킬"),
	Consumable UMETA(DisplayName = "소비")
};

UCLASS()
class PROJECT_S_API UItemTemplate : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	// 아이템 기본 정보
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText ItemName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	UTexture2D* Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	EItemType ItemType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FGameplayTag ItemTag;

	// 스킬 (능력 부여)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayAbility>> GrantedAbilities;

	// 장비 (스탯 증가)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayEffect>> PassiveEffects;

	// 소비 아이템용 - 사용 시 적용되는 이펙트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Consumable")
	TSubclassOf<UGameplayEffect> ConsumeEffect;

	// 최대 스택 수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	int32 MaxStackCount = 1;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("Item", GetFName());
	}
};
