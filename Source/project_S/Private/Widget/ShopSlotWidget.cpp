#include "Widget/ShopSlotWidget.h"
#include "Data/ItemTemplate.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"


void UShopSlotWidget::SetShopItemData(UItemTemplate* ItemTemplate, int32 ItemIndex)
{
	CachedItemIndex = ItemIndex;
	CachedItemTemplate = ItemTemplate;

	if (ItemTemplate)
	{
		if (ItemIcon && ItemTemplate->Icon)
		{
			ItemIcon->SetBrushFromTexture(ItemTemplate->Icon);
		}
		if (ItemNameText)
		{
			ItemNameText->SetText(ItemTemplate->ItemName);
		}
		if (ItemTypeText)
		{
			SetItemTypeText(ItemTemplate->ItemType);
		}
		if (ItemInfoText)
		{
			ItemInfoText->SetText(ItemTemplate->ItemInfo);
		}
		if (ItemPriceText && ItemTemplate->BuyPrice > 0)
		{
			const FString PriceText = FString::Printf(TEXT("%d G"), ItemTemplate->BuyPrice);
			ItemPriceText->SetText(FText::FromString(PriceText));
		}

		// 버튼 클릭 이벤트 바인딩
		if (ItemButton && !ItemButton->OnClicked.IsBound())
		{
			ItemButton->OnClicked.AddDynamic(this, &UShopSlotWidget::OnItemButtonClicked);
		}
	}

}

void UShopSlotWidget::OnItemButtonClicked()
{
	OnItemClicked.ExecuteIfBound(CachedItemIndex);
}

void UShopSlotWidget::SetItemTypeText(EItemType ItemType) const
{
	switch (ItemType)
	{
	case EItemType::Equipment:
		ItemTypeText->SetText(FText::FromString(TEXT("[장비]")));
		ItemTypeText->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));
		return;
	case EItemType::Skill:
		ItemTypeText->SetText(FText::FromString(TEXT("[스킬]")));
		ItemTypeText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
		return	;
	case EItemType::Consumable:
		ItemTypeText->SetText(FText::FromString(TEXT("[소비]")));
		ItemTypeText->SetColorAndOpacity(FSlateColor(FLinearColor::Green));
		return;
	case EItemType::Usable:
		ItemTypeText->SetText(FText::FromString(TEXT("[액티브]")));
		ItemTypeText->SetColorAndOpacity(FSlateColor(FLinearColor::Blue));
		return;	
	default:
		ItemTypeText->SetText(FText::FromString(TEXT("[기타]")));
		ItemTypeText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		return;
	}
}
