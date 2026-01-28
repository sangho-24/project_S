#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemSlotWidget.generated.h"

class UImage;
class UTextBlock;
class UButton;
struct FInventoryItem;

DECLARE_DELEGATE_OneParam(FOnInventorySlotClicked, int32);

UCLASS()
class PROJECT_S_API UItemSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 아이템 데이터 설정
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetItemData(const FInventoryItem& Item, int32 SlotIndex);

	FOnInventorySlotClicked OnSlotClickedDelegate;

protected:
	UPROPERTY(meta = (BindWidget))
	UImage* ItemIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemCountText;

	UPROPERTY(meta = (BindWidget))
	UButton* SlotButton;

	UPROPERTY()
	int32 CachedSlotIndex;

	UFUNCTION()
	void OnSlotClicked();
};