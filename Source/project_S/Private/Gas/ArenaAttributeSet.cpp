#include "Gas/ArenaAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "Character/CharBase.h"

UArenaAttributeSet::UArenaAttributeSet()
{
	InitCurrentHP(100.0f);
	InitMaxHP(100.0f);
	InitAttackPower(5.0f);
	InitDefense(5.0f);
}

void UArenaAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UArenaAttributeSet, CurrentHP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UArenaAttributeSet, MaxHP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UArenaAttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UArenaAttributeSet, Defense, COND_None, REPNOTIFY_Always);
}

void UArenaAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// 체력 클램핑
	if (Attribute == GetCurrentHPAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHP());
	}
	else if (Attribute == GetMaxHPAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
		CachedHPPercent = GetCurrentHP() / GetMaxHP();
	}
	// 공격력/방어력은 음수 방지
	else if (Attribute == GetAttackPowerAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetDefenseAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}

void UArenaAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	// CurrentHP 변경 시
	if (Data.EvaluatedData.Attribute == GetCurrentHPAttribute())
	{
		if (AActor* TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get())
		{
			UE_LOG(LogTemp, Warning, TEXT("[서버] %s Take Damage! HP: %.1f / %.1f"),
				*TargetActor->GetName(), GetCurrentHP(), GetMaxHP());
			// 사망 체크
				if (GetCurrentHP() <= 0.0f)
				{
					ACharBase* Character = Cast<ACharBase>(TargetActor);
					UE_LOG(LogTemp, Warning, TEXT("!!! %s IS DEAD !!!"), *TargetActor->GetName());
					Character->Death();
				}
		}
	}
}

void UArenaAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	// 최대체력 변경 후 비율에 맞춰 현재체력 재설정
	if (Attribute == GetMaxHPAttribute())
	{
		SetCurrentHP(GetMaxHP() * CachedHPPercent);
		UE_LOG(LogTemp, Warning, TEXT("MaxHP changed. CurrentHP adjusted to %.1f"), GetCurrentHP());
	}
}

// 클라이언트 동기화
void UArenaAttributeSet::OnRep_CurrentHP(const FGameplayAttributeData& OldCurrentHP)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UArenaAttributeSet, CurrentHP, OldCurrentHP);
}
void UArenaAttributeSet::OnRep_MaxHP(const FGameplayAttributeData& OldMaxHP)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UArenaAttributeSet, MaxHP, OldMaxHP);
}
void UArenaAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UArenaAttributeSet, AttackPower, OldAttackPower);
}
void UArenaAttributeSet::OnRep_Defense(const FGameplayAttributeData& OldDefense)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UArenaAttributeSet, Defense, OldDefense);
}