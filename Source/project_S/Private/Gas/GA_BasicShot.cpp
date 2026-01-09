
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
	SetAssetTags(FGameplayTagContainer(FGameplayTag::RequestGameplayTag(TEXT("Ability.BasicShot"))));
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
	float AttackPower = 5.f;

	// 투사체 위치, 회전 계산 (수정 필요)
	FVector CharLocation = Character->GetActorLocation();
	FVector ForwardVector = Character->GetActorForwardVector();
	FVector SpawnLocation = CharLocation;
	FRotator SpawnRotation = ForwardVector.Rotation();

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
			Projectile->SetDamage(AttackPower);
			Projectile->SetSpeed(ProjectileSpeed);
			Projectile->Launch(ForwardVector);

			UE_LOG(LogTemp, Warning, TEXT("투사체 발사 성공! 데미지: %.1f, 속도: %.1f"),
				AttackPower, ProjectileSpeed);
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
	//ACharBase* Character = Cast<ACharBase>(ActorInfo->AvatarActor.Get());
	//if (Character && Character->IsDead())
	//{
	//	return false;
	//}

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
