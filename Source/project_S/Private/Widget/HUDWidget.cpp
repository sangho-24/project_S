#include "Widget/HUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

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