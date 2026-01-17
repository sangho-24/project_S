#include "Widget/HUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Gas/ArenaAttributeSet.h"

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