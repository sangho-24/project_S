#include "Gas/ArenaAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UArenaAttributeSet::UArenaAttributeSet()
{
	InitCurrentHP(100.0f);
	InitMaxHP(100.0f);
}

void UArenaAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UArenaAttributeSet, CurrentHP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UArenaAttributeSet, MaxHP, COND_None, REPNOTIFY_Always);
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
		CachedHPPercent = GetCurrentHP() / GetMaxHP();
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
					UE_LOG(LogTemp, Warning, TEXT("!!! %s IS DEAD !!!"), *TargetActor->GetName());
					// 여기서 Character->Death() 호출
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