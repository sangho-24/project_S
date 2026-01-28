#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShopWidget.generated.h"

class AItemShop;
class ACharBase;
class UItemTemplate;
class UShopSlotWidget;
class UUniformGridPanel;

UCLASS()
class PROJECT_S_API UShopWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void InitializeShop(AItemShop* Shop, ACharBase* Player);

	// 블루프린트에서 아이템 클릭 시 호출
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void OnShopItemClicked(int32 ItemIndex);

	UFUNCTION(BlueprintCallable, Category = "Shop")
	void RefreshShopItems();

	UFUNCTION(BlueprintPure, Category = "Shop")
	AItemShop* GetShop() const { return ShopRep; }

	UFUNCTION(BlueprintPure, Category = "Shop")
	ACharBase* GetPlayer() const { return PlayerRep; }

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(EditDefaultsOnly, Category = "Shop")
	TSubclassOf<UShopSlotWidget> ShopItemSlotWidgetClass;

	UPROPERTY(meta = (BindWidget))
	UUniformGridPanel* ShopGridPanel;

	UPROPERTY(BlueprintReadOnly, Category = "Shop")
	AItemShop* ShopRep;

	UPROPERTY(BlueprintReadOnly, Category = "Shop")
	ACharBase* PlayerRep;
};
