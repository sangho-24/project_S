// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayEffectTypes.h"
#include "MainPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UHUDWidget;
class UInventoryWidget;

UCLASS()
class PROJECT_S_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UHUDWidget> HUDWidgetClass;

	UPROPERTY(VisibleAnywhere, Category = "UI")
	UHUDWidget* HUDWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UInventoryWidget> InventoryWidgetClass;

	UPROPERTY(VisibleAnywhere, Category = "UI")
	UInventoryWidget* InventoryWidget;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* OpenInventoryAction;

public:
	// 캐릭터(폰)의 ASC초기화가 끝나면 호출.
	void CreateHUD();
	FVector GetMouseCursorLocation() const;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	void OpenInventory();

private:
	// 어트리뷰트 값 변경 시 호출되는 콜백함수
	void OnHealthChanged(const FOnAttributeChangeData& Data);
	void OnStatsChanged(const FOnAttributeChangeData& Data);
	void OnGoldChanged(const FOnAttributeChangeData& Data);
};

