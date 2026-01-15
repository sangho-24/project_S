#include "Widget/InventoryWidget.h"
#include "Data/InventoryComponent.h"
#include "Data/InventoryItem.h"
#include "Components/UniformGridPanel.h"
#include "Widget/ItemSlotWidget.h"



void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryUpdated.AddDynamic(this, &UInventoryWidget::OnInventoryUpdated);
	}
}

void UInventoryWidget::SetInventoryComponent(UInventoryComponent* InInventoryComponent)
{
	// 기존 델리게이트 해제
	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryUpdated.RemoveDynamic(this, &UInventoryWidget::OnInventoryUpdated);
	}

	InventoryComponent = InInventoryComponent;

	// 새로운 델리게이트 바인딩
	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryUpdated.AddDynamic(this, &UInventoryWidget::OnInventoryUpdated);
	}
}

void UInventoryWidget::RefreshInventory()
{
	if (!InventoryComponent || !ItemGridPanel)
	{
		return;
	}

	// 기존 슬롯 클리어
	ItemGridPanel->ClearChildren();

	const TArray<FInventoryItem>& Inventory = InventoryComponent->GetInventory();

	// 그리드 크기 설정
	const int32 ColumnCount = 4;

	for (int32 i = 0; i < Inventory.Num(); i++)
	{
		const FInventoryItem& Item = Inventory[i];

		// 아이템 슬롯 위젯 생성
		if (ItemSlotWidgetClass)
		{
			UItemSlotWidget* SlotWidget = CreateWidget<UItemSlotWidget>(this, ItemSlotWidgetClass);
			if (SlotWidget)
			{
				SlotWidget->SetItemData(Item, i);
				// 그리드에 추가 (행, 열 계산)
				int32 Row = i / ColumnCount;
				int32 Column = i % ColumnCount;
				ItemGridPanel->AddChildToUniformGrid(SlotWidget, Row, Column);
			}
		}
	}
}

void UInventoryWidget::OnInventoryUpdated(int32 SlotIndex, const FInventoryItem& Item)
{
	// 인벤토리가 변경되면 전체 갱신
	RefreshInventory();
}