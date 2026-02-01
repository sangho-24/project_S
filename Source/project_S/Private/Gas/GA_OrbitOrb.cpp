#include "Gas/GA_OrbitOrb.h"
#include "Character/CharBase.h"
#include "Character/Projectile_Orbit.h"


void UGA_OrbitOrb::ActivateAbility(
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

	if (!HasAuthority(&ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	ACharBase* Character = Cast<ACharBase>(ActorInfo->AvatarActor.Get());
	if (!Character || !ProjectileClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (ProjectileClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		float RandomRotation = FMath::FRandRange(0.0f, 360.0f / OrbitCount);

		for (int32 i = 0; i < OrbitCount; i++)
		{
			float AngleOffset = RandomRotation + (360.0f / OrbitCount) * i;

			AProjectileBase* Projectile = GetWorld()->SpawnActor<AProjectileBase>(
				ProjectileClass,
				Character->GetActorLocation(),
				FRotator::ZeroRotator,
				SpawnParams);

			if (Projectile)
			{
				Projectile->SetDamage(ProjectileDamage);
				Projectile->SetProjectileLifeSpan(ProjectileLifeSpan);
				if (AProjectile_Orbit* Orb = Cast<AProjectile_Orbit>(Projectile))
				{
				Orb->Initialize(OrbitRadius, OrbitSpeed, AngleOffset);
				}
			}
		}
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}