#include "Gas/MMC_BasicDamageCalculation.h"
#include "Gas/ArenaAttributeSet.h"

UMMC_BasicDamageCalculation::UMMC_BasicDamageCalculation()
{
	// 대상의 방어력 캡처
	DefenseDef.AttributeToCapture = UArenaAttributeSet::GetDefenseAttribute();
	DefenseDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	DefenseDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(DefenseDef);
}

float UMMC_BasicDamageCalculation::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;

	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	float Defense = 0.0f;
	GetCapturedAttributeMagnitude(DefenseDef, Spec, EvaluationParameters, Defense);

	float Damage = Spec.GetSetByCallerMagnitude(
		FGameplayTag::RequestGameplayTag("Data.Damage"), false, 0.0f);

	// 최종 데미지 계산 : 데미지 * (방어력 / (방어력+상수))
	float DamageReduction = Defense / (Defense + ArmorConstant);
	float DamageMultiplier = FMath::Clamp(1.0f - DamageReduction, 0.0f, 1.0f);
	float FinalDamage = FMath::Max(Damage * DamageMultiplier, 1.0f);

	UE_LOG(LogTemp, Log, TEXT("데미지 %.0f, 대상 방어력 %.0f, 데미지 감소율 %.0f%% (데미지 감소 %.1f -> %.1f)"),
		Damage, Defense, DamageReduction*100, Damage, FinalDamage);

	return -FinalDamage;
}