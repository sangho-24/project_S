#include "Widget/ItemSlotWidget.h"
#include "Data/InventoryItem.h"
#include "Data/ItemTemplate.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UItemSlotWidget::SetItemData(const FInventoryItem& Item, int32 SlotIndex)
{
	CachedSlotIndex = SlotIndex;

	if (Item.IsValid() && Item.ItemTemplate)
	{
		// 아이템 아이콘 설정
		if (ItemIcon && Item.ItemTemplate->Icon)
		{
			ItemIcon->SetBrushFromTexture(Item.ItemTemplate->Icon);
			ItemIcon->SetVisibility(ESlateVisibility::Visible);
		}

		// 개수 표시
		if (ItemCountText)
		{
			ItemCountText->SetText(FText::AsNumber(Item.StackCount));
			if (Item.ItemTemplate->MaxStackCount > 1)
			{
				ItemCountText->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				ItemCountText->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
	else
	{
		// 빈 슬롯
		if (ItemIcon)
		{
			ItemIcon->SetVisibility(ESlateVisibility::Hidden);
		}
		if (ItemCountText)
		{
			ItemCountText->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	// 버튼 클릭 이벤트 바인딩
	if (SlotButton && !SlotButton->OnClicked.IsBound())
	{
		SlotButton->OnClicked.AddDynamic(this, &UItemSlotWidget::OnSlotClicked);
	}
}

void UItemSlotWidget::OnSlotClicked()
{
	OnSlotClickedDelegate.ExecuteIfBound(CachedSlotIndex);
}