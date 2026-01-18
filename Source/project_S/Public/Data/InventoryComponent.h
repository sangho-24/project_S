#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/InventoryItem.h"
#include "InventoryComponent.generated.h"

class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryUpdated, int32, SlotIndex, const FInventoryItem&, Item);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_S_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	// 인벤토리 슬롯
	UPROPERTY(ReplicatedUsing = OnRep_InventoryItems, BlueprintReadOnly, Category = "Inventory")
	TArray<FInventoryItem> InventoryItems;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	int32 MaxSlots = 20;

public:	
	// 인벤토리 변경 알림
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

	// 아이템 추가
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(UItemTemplate* ItemTemplate, int32 Count = 1);

	// 아이템 제거
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(int32 SlotIndex, int32 Count = 1);

	// 아이템 사용 (장착/소비)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool UseItem(int32 SlotIndex);

	// 인벤토리 조회
	UFUNCTION(BlueprintPure, Category = "Inventory")
	const TArray<FInventoryItem>& GetInventory() const { return InventoryItems; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FInventoryItem GetItemAtSlot(int32 SlotIndex) const;

protected:
	UFUNCTION()
	void OnRep_InventoryItems();

	// 서버 전용 함수
	UFUNCTION(Server, Reliable)
	void ServerAddItem(UItemTemplate* ItemTemplate, int32 Count);

	UFUNCTION(Server, Reliable)
	void ServerRemoveItem(int32 SlotIndex, int32 Count);

	UFUNCTION(Server, Reliable)
	void ServerUseItem(int32 SlotIndex);


private:
	// GAS 어빌리티/이펙트 부여
	void GrantItemAbilitiesAndEffects(FInventoryItem& Item);
	void RemoveItemAbilitiesAndEffects(FInventoryItem& Item);

	// 헬퍼 함수
	int32 FindEmptySlot() const;

	UAbilitySystemComponent* GetOwnerASC() const;

};
