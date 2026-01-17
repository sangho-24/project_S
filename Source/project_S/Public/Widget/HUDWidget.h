// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UArenaAttributeSet;

UCLASS()
class PROJECT_S_API UHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 체력바 업데이트 함수
	void UpdateHP(float CurrentHP, float MaxHP);
	void UpdateStats(const UArenaAttributeSet* AttributeSet);

protected:
	// 위젯 블루프린트에서 이름이 "HPProgressBar"인 요소와 자동 연결
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HPProgressBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HPText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AttackPowerText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DefenseText;
};
