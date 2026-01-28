// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemShop.generated.h"

class ACharBase;
class UBoxComponent;
class UItemTemplate;
class UGameplayEffect;

UCLASS()
class PROJECT_S_API AItemShop : public AActor
{
	GENERATED_BODY()
	
public:	
	AItemShop();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	TArray<UItemTemplate*> ItemTemplates;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	TSubclassOf<UUserWidget> ShopWidgetClass;

	UPROPERTY()
	UUserWidget* CurrentShopWidget;

	UPROPERTY(EditDefaultsOnly, Category = "Shop")
	TSubclassOf<UGameplayEffect> GoldModifyEffectClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shop")
	UBoxComponent* ShopArea;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* ShopOwnerMesh;

protected:
	virtual void BeginPlay() override;

public:	
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void BuyItem(int32 ItemIndex, ACharBase* Buyer);

	UFUNCTION(BlueprintCallable, Category = "Shop")
	void SellItem(int32 SlotIndex, ACharBase* Seller);

	UFUNCTION(BlueprintCallable, Category = "Shop")
	TArray<UItemTemplate*> GetShopItems() const { return ItemTemplates; }

private:
	void ModifyGold(ACharBase* Target, int32 Amount);

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

};
