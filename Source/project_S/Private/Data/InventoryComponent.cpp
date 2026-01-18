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

	// 이미 존재하는 스택부터 채워보기
	if (ItemTemplate->MaxStackCount > 1)
	{
		for (int32 i = 0; i < InventoryItems.Num() && RemainingCount > 0; i++)
		{
			FInventoryItem& ExistingItem = InventoryItems[i];

			if (ExistingItem.IsValid() &&
				ExistingItem.ItemTemplate == ItemTemplate &&
				ExistingItem.StackCount < ItemTemplate->MaxStackCount)
			{
				int32 RemainingSpace = ItemTemplate->MaxStackCount - ExistingItem.StackCount;
				int32 AddCount = FMath::Min(RemainingSpace, RemainingCount);

				ExistingItem.StackCount += AddCount;
				RemainingCount -= AddCount;

				OnInventoryUpdated.Broadcast(i, ExistingItem);
			}
		}
	}
	// 2단계: 남은 아이템을 새 슬롯에 추가
	while (RemainingCount > 0)
	{
		int32 EmptySlot = FindEmptySlot();
		if (EmptySlot == INDEX_NONE)
		{
			UE_LOG(LogTemp, Warning, TEXT("인벤토리 풀! %d개 추가 실패"), RemainingCount);
			return false;
		}

		int32 AddCount = FMath::Min(ItemTemplate->MaxStackCount, RemainingCount);
		InventoryItems[EmptySlot] = FInventoryItem(ItemTemplate, AddCount);
		RemainingCount -= AddCount;

		if (InventoryItems[EmptySlot].GrantedAbilityHandles.Num() == 0 && InventoryItems[EmptySlot].ActiveEffectHandles.Num() == 0)
		{
			GrantItemAbilitiesAndEffects(InventoryItems[EmptySlot]);
		}
		OnInventoryUpdated.Broadcast(EmptySlot, InventoryItems[EmptySlot]);
	}
	return true;
}

bool UInventoryComponent::RemoveItem(int32 SlotIndex, int32 Count)
{
	if (!InventoryItems.IsValidIndex(SlotIndex) || !InventoryItems[SlotIndex].IsValid())
	{
		return false;
	}

	if (GetOwnerRole() != ROLE_Authority)
	{
		ServerRemoveItem(SlotIndex, Count);
		return true;
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
	if (!InventoryItems.IsValidIndex(SlotIndex) || !InventoryItems[SlotIndex].IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("빈 슬롯"));
		return false;
	}

	if (GetOwnerRole() != ROLE_Authority)
	{
		ServerUseItem(SlotIndex);
		return true;
	}

	FInventoryItem& Item = InventoryItems[SlotIndex];
	UItemTemplate* ItemTemplate = Item.ItemTemplate;
	if (!ItemTemplate)
	{
		UE_LOG(LogTemp, Warning, TEXT("아이템 템플릿 없음"));
		return false;
	}
	UAbilitySystemComponent* ASC = GetOwnerASC();
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("ASC 없음"));
		return false;
	}

	if(ItemTemplate->ItemType == EItemType::Equipment || ItemTemplate->ItemType == EItemType::Skill)
	{
		UE_LOG(LogTemp, Log, TEXT("사용 불가 아이템 타입"));
		return false;
	}
	if(ItemTemplate->ItemType == EItemType::Consumable || ItemTemplate->ItemType == EItemType::Usable)
	{
		for (const FGameplayEffectValue& ActiveEffect : Item.ItemTemplate->ActiveEffects)
		{
			if (ActiveEffect.EffectClass)
			{
				FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
				ContextHandle.AddSourceObject(GetOwner());

				FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(ActiveEffect.EffectClass, 1.0f, ContextHandle);
				if (SpecHandle.IsValid())
				{
					SpecHandle.Data.Get()->SetSetByCallerMagnitude(ActiveEffect.DataTag, ActiveEffect.Value);
					FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
					if (ItemTemplate->ItemType == EItemType::Consumable)
					{
						RemoveItem(SlotIndex, 1);
						OnInventoryUpdated.Broadcast(SlotIndex, Item);
					}
					return true;
				}
			}
		}
	}
	return false;
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
	for (const FGameplayEffectValue& EquipmentEffect : Item.ItemTemplate->EquipmentEffects)
	{
		if (EquipmentEffect.EffectClass)
		{
			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			ContextHandle.AddSourceObject(GetOwner());

			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EquipmentEffect.EffectClass, 1.0f, ContextHandle);
			if (SpecHandle.IsValid())
			{
				SpecHandle.Data.Get()->SetSetByCallerMagnitude(EquipmentEffect.DataTag, EquipmentEffect.Value);
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

UAbilitySystemComponent* UInventoryComponent::GetOwnerASC() const
{
	if (AActor* Owner = GetOwner())
	{
		return UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	}
	return nullptr;
}