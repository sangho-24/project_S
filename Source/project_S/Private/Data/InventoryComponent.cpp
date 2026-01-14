#include "Data/InventoryComponent.h"
#include "Data/ItemTemplate.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Net/UnrealNetwork.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UInventoryComponent, InventoryItems);
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// 인벤토리 초기화
	if (GetOwnerRole() == ROLE_Authority)
	{
		InventoryItems.SetNum(MaxSlots);
	}
}

bool UInventoryComponent::AddItem(UItemTemplate* ItemTemplate, int32 Count)
{
	if (!ItemTemplate || Count <= 0)
	{
		return false;
	}

	if (GetOwnerRole() != ROLE_Authority)
	{
		ServerAddItem(ItemTemplate, Count);
		return true;
	}

	int32 RemainingCount = Count;
	while (RemainingCount > 0)
	{	// 스택 아이템이면 기존 슬롯에 추가
		if (ItemTemplate->MaxStackCount > 1)
		{
			int32 ExistingSlot = FindItemSlot(ItemTemplate);
			if (ExistingSlot != INDEX_NONE)
			{
				FInventoryItem& ExistingItem = InventoryItems[ExistingSlot];
				int32 RemainingSpace = ItemTemplate->MaxStackCount - ExistingItem.StackCount;

				if (RemainingSpace >= RemainingCount)
				{
					ExistingItem.StackCount += RemainingCount;
					OnInventoryUpdated.Broadcast(ExistingSlot, ExistingItem);
					return true;
				}
				else
				{	// 일부만 스택에 추가하고 나머지는 새 슬롯에
					ExistingItem.StackCount = ItemTemplate->MaxStackCount;
					RemainingCount -= RemainingSpace;
					OnInventoryUpdated.Broadcast(ExistingSlot, ExistingItem);
				}
			}
		}
		// 새 슬롯에 추가
		int32 EmptySlot = FindEmptySlot();
		if (EmptySlot == INDEX_NONE)
		{
			UE_LOG(LogTemp, Warning, TEXT("인벤토리 풀! %d개 추가 실패"), RemainingCount);
			return false;
		}
		if (ItemTemplate->MaxStackCount >= RemainingCount)
		{
			InventoryItems[EmptySlot] = FInventoryItem(ItemTemplate, RemainingCount);
			OnInventoryUpdated.Broadcast(EmptySlot, InventoryItems[EmptySlot]);
			UE_LOG(LogTemp, Log, TEXT("아이템 추가: %s x%d"), *ItemTemplate->ItemName.ToString(), RemainingCount);
			return true;
		}
		else
		{
			InventoryItems[EmptySlot] = FInventoryItem(ItemTemplate, ItemTemplate->MaxStackCount);
			OnInventoryUpdated.Broadcast(EmptySlot, InventoryItems[EmptySlot]);
			RemainingCount -= ItemTemplate->MaxStackCount;
		}
	}
	return true;
}

bool UInventoryComponent::RemoveItem(int32 SlotIndex, int32 Count)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		ServerRemoveItem(SlotIndex, Count);
		return true;
	}

	if (!InventoryItems.IsValidIndex(SlotIndex) || !InventoryItems[SlotIndex].IsValid())
	{
		return false;
	}

	FInventoryItem& Item = InventoryItems[SlotIndex];
	
	if (Item.StackCount <= Count)
	{
		// 장착 해제
		RemoveItemAbilitiesAndEffects(Item);
		// 슬롯 비우기
		Item = FInventoryItem();
	}
	else
	{
		Item.StackCount -= Count;
	}

	OnInventoryUpdated.Broadcast(SlotIndex, Item);
	return true;
}

bool UInventoryComponent::UseItem(int32 SlotIndex)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		ServerUseItem(SlotIndex);
		return true;
	}

	if (!InventoryItems.IsValidIndex(SlotIndex) || !InventoryItems[SlotIndex].IsValid())
	{
		return false;
	}

	FInventoryItem& Item = InventoryItems[SlotIndex];
	UItemTemplate* ItemTemplate = Item.ItemTemplate;

	// 이미 장착된 장비는 사용 불가
	if (Item.GrantedAbilityHandles.Num() > 0 || Item.ActiveEffectHandles.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("이미 장착된 아이템"));
		return false;
	}

	switch (ItemTemplate->ItemType)
	{
	case EItemType::Equipment:
	case EItemType::Skill:
		// 장비/패시브 아이템 - 어빌리티와 이펙트 부여
		GrantItemAbilitiesAndEffects(Item);
		UE_LOG(LogTemp, Log, TEXT("아이템 장착: %s"), *ItemTemplate->ItemName.ToString());
		break;

	case EItemType::Consumable:
		// 소비 아이템 - 이펙트 적용 후 제거
		if (ItemTemplate->ConsumeEffect && GetOwnerASC())
		{
			FGameplayEffectContextHandle ContextHandle = GetOwnerASC()->MakeEffectContext();
			ContextHandle.AddSourceObject(GetOwner());
			
			FGameplayEffectSpecHandle SpecHandle = GetOwnerASC()->MakeOutgoingSpec(
				ItemTemplate->ConsumeEffect, 1.0f, ContextHandle);
			
			if (SpecHandle.IsValid())
			{
				GetOwnerASC()->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				RemoveItem(SlotIndex, 1);
				UE_LOG(LogTemp, Log, TEXT("소비 아이템 사용: %s"), *ItemTemplate->ItemName.ToString());
			}
		}
		break;
	}

	OnInventoryUpdated.Broadcast(SlotIndex, Item);
	return true;
}

bool UInventoryComponent::UnequipItem(int32 SlotIndex)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		ServerUnequipItem(SlotIndex);
		return true;
	}

	if (!InventoryItems.IsValidIndex(SlotIndex) || !InventoryItems[SlotIndex].IsValid())
	{
		return false;
	}

	FInventoryItem& Item = InventoryItems[SlotIndex];
	RemoveItemAbilitiesAndEffects(Item);
	OnInventoryUpdated.Broadcast(SlotIndex, Item);

	UE_LOG(LogTemp, Log, TEXT("아이템 장착 해제: %s"), *Item.ItemTemplate->ItemName.ToString());
	return true;
}

FInventoryItem UInventoryComponent::GetItemAtSlot(int32 SlotIndex) const
{
	if (InventoryItems.IsValidIndex(SlotIndex))
	{
		return InventoryItems[SlotIndex];
	}
	return FInventoryItem();
}

void UInventoryComponent::OnRep_InventoryItems()
{
	// 클라이언트에서 인벤토리 업데이트 알림
	for (int32 i = 0; i < InventoryItems.Num(); i++)
	{
		OnInventoryUpdated.Broadcast(i, InventoryItems[i]);
	}
}
void UInventoryComponent::ServerAddItem_Implementation(UItemTemplate* ItemTemplate, int32 Count)
{
	AddItem(ItemTemplate, Count);
}

void UInventoryComponent::ServerRemoveItem_Implementation(int32 SlotIndex, int32 Count)
{
	RemoveItem(SlotIndex, Count);
}

void UInventoryComponent::ServerUseItem_Implementation(int32 SlotIndex)
{
	UseItem(SlotIndex);
}

void UInventoryComponent::ServerUnequipItem_Implementation(int32 SlotIndex)
{
	UnequipItem(SlotIndex);
}

void UInventoryComponent::GrantItemAbilitiesAndEffects(FInventoryItem& Item)
{
	UAbilitySystemComponent* ASC = GetOwnerASC();
	if (!ASC || !Item.ItemTemplate)
	{
		return;
	}

	// 어빌리티 부여
	for (TSubclassOf<UGameplayAbility> AbilityClass : Item.ItemTemplate->GrantedAbilities)
	{
		if (AbilityClass)
		{
			FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, INDEX_NONE, this);
			FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(AbilitySpec);
			Item.GrantedAbilityHandles.Add(Handle);
		}
	}

	// 패시브 이펙트 적용
	for (const FPassiveGameplayEffect& PassiveEffect : Item.ItemTemplate->PassiveEffects)
	{
		if (PassiveEffect.EffectClass)
		{
			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			ContextHandle.AddSourceObject(GetOwner());

			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(PassiveEffect.EffectClass, 1.0f, ContextHandle);
			if (SpecHandle.IsValid())
			{
				for (const FGameplayTag& Tag : PassiveEffect.DataTag)
				{
					SpecHandle.Data.Get()->SetSetByCallerMagnitude(Tag, PassiveEffect.Value);
				}
				FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				Item.ActiveEffectHandles.Add(ActiveHandle);
			}
		}
	}
}

void UInventoryComponent::RemoveItemAbilitiesAndEffects(FInventoryItem& Item)
{
	UAbilitySystemComponent* ASC = GetOwnerASC();
	if (!ASC)
	{
		return;
	}

	// 어빌리티 제거
	for (const FGameplayAbilitySpecHandle& Handle : Item.GrantedAbilityHandles)
	{
		ASC->ClearAbility(Handle);
	}
	Item.GrantedAbilityHandles.Empty();

	// 이펙트 제거
	for (const FActiveGameplayEffectHandle& Handle : Item.ActiveEffectHandles)
	{
		ASC->RemoveActiveGameplayEffect(Handle);
	}
	Item.ActiveEffectHandles.Empty();
}

int32 UInventoryComponent::FindEmptySlot() const
{
	for (int32 i = 0; i < InventoryItems.Num(); i++)
	{
		if (!InventoryItems[i].IsValid())
		{
			return i;
		}
	}
	return INDEX_NONE;
}

int32 UInventoryComponent::FindItemSlot(UItemTemplate* ItemTemplate) const
{
	for (int32 i = 0; i < InventoryItems.Num(); i++)
	{
		if (InventoryItems[i].ItemTemplate == ItemTemplate)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

UAbilitySystemComponent* UInventoryComponent::GetOwnerASC() const
{
	if (AActor* Owner = GetOwner())
	{
		return UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	}
	return nullptr;
}