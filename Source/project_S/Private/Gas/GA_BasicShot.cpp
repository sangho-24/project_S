
#include "Gas/GA_BasicShot.h"
#include "Character/CharBase.h"
#include "Character/ProjectileBase.h"
#include "Gas/ArenaAttributeSet.h"
#include "DrawDebugHelpers.h"
#include "GameplayTagContainer.h"

UGA_BasicShot::UGA_BasicShot()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	//AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.BasicShot")));
	SetAssetTags(FGameplayTagContainer(FGameplayTag::RequestGameplayTag(TEXT("Ability.Attack.BasicShot"))));
	// HandleGameplayEvent 어빌리티는 트리거 태그가 필요함
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Attack.BasicShot"));
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

void UGA_BasicShot::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 서버에서만 실행
	if (!HasAuthority(&ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	ACharBase* Character = Cast<ACharBase>(ActorInfo->AvatarActor.Get());
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("캐릭터 없음"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	UArenaAttributeSet* AttributeSet = Character->GetAttributeSet();
	if (!AttributeSet)
	{
		UE_LOG(LogTemp, Warning, TEXT("AttributeSet 없음"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// Attribute에서 공격력 가져오기
	//float ProjectileDamage = 5.f;
	//float ProjectileSpeed = 1000.f;

	// EventData에서 마우스 위치 가져오기
	FVector MouseCursorLocation = FVector::ZeroVector;
	if (TriggerEventData && TriggerEventData->TargetData.IsValid(0))
	{
		const FGameplayAbilityTargetData* TargetData = TriggerEventData->TargetData.Get(0);
		if (TargetData)
		{
			const FGameplayAbilityTargetData_LocationInfo* LocationData =
				static_cast<const FGameplayAbilityTargetData_LocationInfo*>(TargetData);

			if (LocationData)
			{
				// TargetLocation에서 위치 추출
				FTransform TargetTransform = LocationData->TargetLocation.GetTargetingTransform();
				MouseCursorLocation = TargetTransform.GetLocation();
			}
		}
	}

	// 투사체 위치, 회전 계산
	FVector SpawnLocation = Character->GetActorLocation();
	MouseCursorLocation.Z = SpawnLocation.Z;
	FVector Direction = (MouseCursorLocation - SpawnLocation).GetSafeNormal();
	FRotator SpawnRotation = Direction.Rotation();
	
	// 투사체 스폰
	if (ProjectileClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AProjectileBase* Projectile = GetWorld()->SpawnActor<AProjectileBase>(
			ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);

		if (Projectile)
		{
			Projectile->SetDamage(ProjectileDamage);
			Projectile->SetSpeed(ProjectileSpeed);
			Projectile->SetProjectileLifeSpan(ProjectileLifeSpan);
			Projectile->Launch(Direction);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("투사체 스폰 실패!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ProjectileClass가 설정되지 않음!"));
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

bool UGA_BasicShot::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayTagContainer* SourceTags, 
	const FGameplayTagContainer* TargetTags, 
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// 죽었으면 발사 불가
	ACharBase* Character = Cast<ACharBase>(ActorInfo->AvatarActor.Get());
	if (Character && Character->GetIsDead())
	{
		return false;
	}

	return true;
}

void UGA_BasicShot::InputReleased(
	const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo)
{

}

void UGA_BasicShot::CancelAbility(
	const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo, 
	bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}
