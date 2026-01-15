#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWidget.generated.h"

class UInventoryComponent;
class UUniformGridPanel;

UCLASS()
class PROJECT_S_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta = (BindWidget))
	UUniformGridPanel* ItemGridPanel;

	UPROPERTY()
	UInventoryComponent* InventoryComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UUserWidget> ItemSlotWidgetClass;

public:
	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetInventoryComponent(UInventoryComponent* InInventoryComponent);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RefreshInventory();

private:
	UFUNCTION()
	void OnInventoryUpdated(int32 SlotIndex, const FInventoryItem& Item);
};
