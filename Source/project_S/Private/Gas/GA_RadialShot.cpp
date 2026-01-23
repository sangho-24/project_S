#include "Gas/GA_RadialShot.h"
#include "Character/CharBase.h"
#include "Character/ProjectileBase.h"
#include "Gas/ArenaAttributeSet.h"
#include "GameplayTagContainer.h"


UGA_RadialShot::UGA_RadialShot()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	AbilityTag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Attack.RadialShot"));
    SetAssetTags(FGameplayTagContainer(AbilityTag));
}

void UGA_RadialShot::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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

	// 투사체 스폰
	if (ProjectileClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		float RandomRotation = FMath::FRandRange(0.0f, 360.0f/ProjectileCount);
		float AngleRad = FMath::DegreesToRadians(RandomRotation);
		FVector Direction = FVector(FMath::Cos(AngleRad), FMath::Sin(AngleRad), 0.0f);
		FVector SpawnLocation = Character->GetActorLocation();
		FRotator SpawnRotation = Direction.Rotation();

		AProjectileBase* Projectile = GetWorld()->SpawnActor<AProjectileBase>(
			ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
		if (Projectile)
		{
			Projectile->SetDamage(ProjectileDamage);
			Projectile->SetSpeed(ProjectileSpeed);
			Projectile->Launch(Direction);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("투사체 스폰 실패!"));
		}

		// 람다용 로컬 변수 복사본
		TSubclassOf<AProjectileBase> LocalProjectileClass = ProjectileClass;
		float LocalProjectileDamage = ProjectileDamage;
		float LocalProjectileSpeed = ProjectileSpeed;
		int32 LocalProjectileCount = ProjectileCount;
		UWorld* LocalWorld = Character->GetWorld();

		for (int32 i = 1; i < ProjectileCount; i++)
		{
			FTimerHandle TimerHandle;
			Character->GetWorldTimerManager().SetTimer(
				TimerHandle, 
				[SpawnParams, Character, LocalProjectileClass, LocalProjectileDamage, LocalProjectileSpeed, LocalProjectileCount, RandomRotation, LocalWorld, i]()
			{
			float AngleDeg = RandomRotation + i * (360.0f / LocalProjectileCount);
			float AngleRad = FMath::DegreesToRadians(AngleDeg);
			FVector Direction = FVector(FMath::Cos(AngleRad), FMath::Sin(AngleRad), 0.0f);
			FVector SpawnLocation = Character->GetActorLocation();
			FRotator SpawnRotation = Direction.Rotation();

			AProjectileBase* Projectile = LocalWorld->SpawnActor<AProjectileBase>(
				LocalProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
			if (Projectile)
			{
				Projectile->SetDamage(LocalProjectileDamage);
				Projectile->SetSpeed(LocalProjectileSpeed);
				Projectile->Launch(Direction);
			}
			}, i* ProjectileInterval, false);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ProjectileClass가 설정되지 않음!"));
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

bool UGA_RadialShot::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	ACharBase* Character = Cast<ACharBase>(ActorInfo->AvatarActor.Get());
	if (Character && Character->GetIsDead())
	{
		return false;
	}

	return true;
}

void UGA_RadialShot::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
}

void UGA_RadialShot::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UGA_RadialShot::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);
	ACharBase* Character = Cast<ACharBase>(ActorInfo->AvatarActor.Get());
	if (Character)
	{
		Character->RegisterAbility(AbilityTag, Spec.Handle);
	}
}

void UGA_RadialShot::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	ACharBase* Character = Cast<ACharBase>(ActorInfo->AvatarActor.Get());
	if (Character)
	{
		// 여기서 TMap을 만들어서 전달하는것과, Character->UnregisterAbility 안에서 Tag랑 Spec으로 TMap을 만들어 쓰는 것 중 뭐가 더 나아?
		// 그리고 TMap에 키, 값 있잖아 Tag가 키고 Spec이 값인거지? 키 중복 돼?
		Character->UnregisterAbility(AbilityTag, Spec.Handle);
	}
	Super::OnRemoveAbility(ActorInfo, Spec);
}

//void UGA_RadialShot::StartAutoCastTimer()
//{
//	UE_LOG(LogTemp, Warning, TEXT("[스타트 호출]"));
//	UE_LOG(LogTemp, Warning, TEXT("AutoCastSpecHandles.Num : %d"), AutoCastSpecHandles.Num());
//	if (AutoCastSpecHandles.Num() == 0)
//	{
//		return;
//	}
//
//	if (UWorld* World = GetWorld())
//	{
//		const float Interval = SkillStack > 0 ? Cooldown/ SkillStack : Cooldown;
//		UE_LOG(LogTemp, Warning, TEXT("Interval : %f"), Interval);
//		World->GetTimerManager().SetTimer(
//			AutoCastTimerHandle,
//			this,
//			&UGA_RadialShot::AutoCastSkill,
//			Interval,
//			true,
//			0.0f);
//	}
//}
//
//void UGA_RadialShot::StopAutoCastTimer()
//{
//	UE_LOG(LogTemp, Warning, TEXT("[스탑 호출]"));
//	if (UWorld* World = GetWorld())
//	{
//		World->GetTimerManager().ClearTimer(AutoCastTimerHandle);
//	}
//}
//
//void UGA_RadialShot::AutoCastSkill()
//{
//	UE_LOG(LogTemp, Warning, TEXT("[오토캐스트 호출] IsActive: %s, 스택: %d"),
//		IsActive() ? TEXT("TRUE") : TEXT("FALSE"), SkillStack);
//
//	if (AutoCastSpecHandles.Num()==0)
//	{
//		UE_LOG(LogTemp, Warning, TEXT("[탈출]AutoCastSpecHandles.Num()==0"));
//		StopAutoCastTimer();
//		return;
//	}
//
//	FGameplayAbilitySpecHandle ActiveSpecHandle = AutoCastSpecHandles[0];
//	if (!ActiveSpecHandle.IsValid())
//	{
//		UE_LOG(LogTemp, Warning, TEXT("[탈출]ActiveSpecHandle.IsValid() == false"));
//		StopAutoCastTimer();
//		return;
//	}
//
//	UAbilitySystemComponent* ASC = CachedASC.Get();
//	if (!ASC || ASC->GetOwnerRole() != ROLE_Authority || IsActive())
//	{
//		UE_LOG(LogTemp, Warning, TEXT("[탈출]ASC 권한 없음 또는 이미 활성화됨"));
//		return;
//	}
//
//	ASC->TryActivateAbility(ActiveSpecHandle);
//}