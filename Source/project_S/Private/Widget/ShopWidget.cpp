#include "Widget/ShopWidget.h"
#include "Character/ItemShop.h"
#include "Character/CharBase.h"
#include "Data/ItemTemplate.h"
#include "Widget/ShopSlotWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/SizeBox.h"

void UShopWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UShopWidget::InitializeShop(AItemShop* Shop, ACharBase* Player)
{
	ShopRep = Shop;
	PlayerRep = Player;
	RefreshShopItems();
}

void UShopWidget::OnShopItemClicked(int32 ItemIndex)
{
	if (ShopRep && PlayerRep)
	{
		//ShopRep->BuyItem(ItemIndex, PlayerRep);
		PlayerRep->ServerBuyItemFromShop(ShopRep, ItemIndex);
		UE_LOG(LogTemp, Log, TEXT("상점 아이템 %d 구매 요청"), ItemIndex);
	}
}

void UShopWidget::RefreshShopItems()
{
	if (!ShopGridPanel || !ShopRep || !ShopItemSlotWidgetClass)
	{
		return;
	}

	ShopGridPanel->ClearChildren();

	TArray<UItemTemplate*> ShopItems = ShopRep->GetShopItems();

	const int32 ColumnCount = 3;

	for (int32 i = 0; i < ShopItems.Num(); i++)
	{
		if (UShopSlotWidget* SlotWidget = CreateWidget<UShopSlotWidget>(this, ShopItemSlotWidgetClass))
		{
			if (i < ShopItems.Num())
			{
				SlotWidget->SetShopItemData(ShopItems[i], i);
				SlotWidget->OnItemClicked.BindUObject(this, &UShopWidget::OnShopItemClicked);
			}
			else
			{
				SlotWidget->SetShopItemData(nullptr, -1);
			}

			int32 Row = i / ColumnCount;
			int32 Column = i % ColumnCount;
			ShopGridPanel->AddChildToUniformGrid(SlotWidget, Row, Column);
		}
	}
	if (ShopItems.Num() < ColumnCount * 6)
	{
		USizeBox* DummySlot = NewObject<USizeBox>(this);
		DummySlot->SetVisibility(ESlateVisibility::Hidden);
		ShopGridPanel->AddChildToUniformGrid(DummySlot, 5, ColumnCount - 1);
	}
}