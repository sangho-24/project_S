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
}

void UArenaAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	// CurrentHP 변경 시
	if (Data.EvaluatedData.Attribute == GetCurrentHPAttribute())
	{
		// 서버 측 검증 및 사망 처리
		// 로그 확인용: 타겟 액터가 존재하면 무조건 로그 찍도록 변경 (LocallyControlled 체크 제거)
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

// 클라이언트 동기화
void UArenaAttributeSet::OnRep_CurrentHP(const FGameplayAttributeData& OldCurrentHP)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UArenaAttributeSet, CurrentHP, OldCurrentHP);
}

void UArenaAttributeSet::OnRep_MaxHP(const FGameplayAttributeData& OldMaxHP)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UArenaAttributeSet, MaxHP, OldMaxHP);
}