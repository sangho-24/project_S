#include "Gas/GA_BouncingBlade.h"
#include "Character/ProjectileBase.h"
#include "Gas/ArenaAttributeSet.h"
#include "Character/CharBase.h"


void UGA_BouncingBlade::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	if (!HasAuthority(&ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	ACharBase* Character = Cast<ACharBase>(ActorInfo->AvatarActor.Get());
	if (!Character)
	{
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

		// 람다용 로컬 변수 복사본
		TSubclassOf<AProjectileBase> LocalProjectileClass = ProjectileClass;
		float LocalProjectileDamage = ProjectileDamage;
		float LocalProjectileSpeed = ProjectileSpeed;
		float LocalProjectileLifeSpan = ProjectileLifeSpan;
		int32 LocalProjectileCount = ProjectileCount;
		float LocalProjectileBounciness = ProjectileBounciness;
		int32 LocalProjectileMaxBounces = ProjectileMaxBounces;
		UWorld* LocalWorld = Character->GetWorld();

		for (int32 i = 0; i < ProjectileCount; i++)
		{
			FTimerHandle TimerHandle;
			Character->GetWorldTimerManager().SetTimer(
				TimerHandle,
				[SpawnParams, Character, LocalProjectileClass, LocalProjectileDamage, LocalProjectileSpeed, LocalProjectileLifeSpan, LocalProjectileCount, LocalProjectileBounciness, LocalProjectileMaxBounces, LocalWorld, i]()
				{
					float RandomRotation = FMath::FRandRange(0.0f, 360.0f);
					float AngleRad = FMath::DegreesToRadians(RandomRotation);
					FVector Direction = FVector(FMath::Cos(AngleRad), FMath::Sin(AngleRad), 0.0f);
					FVector SpawnLocation = Character->GetActorLocation();
					FRotator SpawnRotation = Direction.Rotation();

					AProjectileBase* Projectile = LocalWorld->SpawnActor<AProjectileBase>(
						LocalProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
					if (Projectile)
					{
						Projectile->SetDamage(LocalProjectileDamage);
						Projectile->SetSpeed(LocalProjectileSpeed);
						Projectile->SetMaxBounces(LocalProjectileMaxBounces);
						Projectile->SetBounciness(LocalProjectileBounciness);
						Projectile->SetProjectileLifeSpan(LocalProjectileLifeSpan);
						Projectile->Launch(Direction);
					}
				}, (i+1) * ProjectileInterval, false);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ProjectileClass가 설정되지 않음!"));
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
