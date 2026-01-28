#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShopSlotWidget.generated.h"

class UImage;
class UTextBlock;
class UButton;
class UItemTemplate;
enum class EItemType : uint8;

DECLARE_DELEGATE_OneParam(FOnShopItemClicked, int32);

UCLASS()
class PROJECT_S_API UShopSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void SetShopItemData(UItemTemplate* ItemTemplate, int32 ItemIndex);

	FOnShopItemClicked OnItemClicked;

protected:
	UPROPERTY(meta = (BindWidget))
	UImage* ItemIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemTypeText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemInfoText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemPriceText;

	UPROPERTY(meta = (BindWidget))
	UButton* ItemButton;

	UPROPERTY()
	int32 CachedItemIndex;

	UPROPERTY()
	UItemTemplate* CachedItemTemplate;

	UFUNCTION()
	void OnItemButtonClicked();

private:
	void SetItemTypeText(EItemType ItemType) const;

};
