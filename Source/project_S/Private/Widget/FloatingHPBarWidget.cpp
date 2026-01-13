#include "Widget/FloatingHPBarWidget.h"
#include "Components/ProgressBar.h"

void UFloatingHPBarWidget::UpdateHP(float CurrentHP, float MaxHP)
{
    if (HPProgressBar && MaxHP > 0.0f)
    {
        const float Percent = CurrentHP / MaxHP;
        HPProgressBar->SetPercent(Percent);
    }
}