#include "Widget/HUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Widget/ItemSlotWidget.h"
#include "Data/InventoryComponent.h"
#include "Data/InventoryItem.h"
#include "Gas/ArenaAttributeSet.h"


void UHUDWidget::NativeDestruct()
{
	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryUpdated.RemoveDynamic(this, &UHUDWidget::OnInventoryUpdated);
		InventoryComponent = nullptr;
	}
	Super::NativeDestruct();

}

void UHUDWidget::UpdateHP(float CurrentHP, float MaxHP)
{
	if (HPProgressBar && MaxHP > 0.0f)
	{
		const float Percent = CurrentHP / MaxHP;
		HPProgressBar->SetPercent(Percent);
	}

	if (HPText)
	{
		FString HPString = FString::Printf(TEXT("%.0f / %.0f"), CurrentHP, MaxHP);
		HPText->SetText(FText::FromString(HPString));
	}
}

void UHUDWidget::UpdateStats(const UArenaAttributeSet* AttributeSet)
{
	if (!AttributeSet)
	{
		return;
	}

	const float AttackPower = AttributeSet->GetAttackPower();
	const float Defense = AttributeSet->GetDefense();

	if (AttackPowerText)
	{
		const FString AttackPowerString = FString::Printf(TEXT("%.0f"), AttackPower);
		AttackPowerText->SetText(FText::FromString(AttackPowerString));
	}

	if (DefenseText)
	{
		const FString DefenseString = FString::Printf(TEXT("%.0f"), Defense);
		DefenseText->SetText(FText::FromString(DefenseString));
	}
}

void UHUDWidget::InitializeInventory(UInventoryComponent* InInventoryComponent)
{
	if (InventoryComponent == InInventoryComponent)
	{
		return;
	}

	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryUpdated.RemoveDynamic(this, &UHUDWidget::OnInventoryUpdated);
	}

	InventoryComponent = InInventoryComponent;

	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryUpdated.AddDynamic(this, &UHUDWidget::OnInventoryUpdated);
	}

	// 다음 프레임에 RefreshInventory 호출 (Replication 대기)
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
			{
				RefreshInventory();
			});
	}
}

void UHUDWidget::RefreshInventory()
{
	if (!InventoryGridPanel)
	{
		return;
	}

	InventoryGridPanel->ClearChildren();

	if (!InventoryComponent || !ItemSlotWidgetClass)
	{
		return;
	}

	const TArray<FInventoryItem>& Inventory = InventoryComponent->GetInventory();
	const int32 ColumnCount = 4;

	for (int32 Index = 0; Index < Inventory.Num(); Index++)
	{
		if (UItemSlotWidget* SlotWidget = CreateWidget<UItemSlotWidget>(this, ItemSlotWidgetClass))
		{
			SlotWidget->SetItemData(Inventory[Index], Index);
			const int32 Row = Index / ColumnCount;
			const int32 Column = Index % ColumnCount;
			UUniformGridSlot* GridSlot = InventoryGridPanel->AddChildToUniformGrid(SlotWidget, Row, Column);
			if (GridSlot)
			{
				GridSlot->SetHorizontalAlignment(HAlign_Fill);
				GridSlot->SetVerticalAlignment(VAlign_Fill);
			}
		}
	}
}

void UHUDWidget::OnInventoryUpdated()
{
	if (!IsValid(InventoryComponent) || !IsValid(InventoryGridPanel))
	{
		return;
	}
	RefreshInventory();
}

