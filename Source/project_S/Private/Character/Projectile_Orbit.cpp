#include "Character/Projectile_Orbit.h"
#include "Character/CharBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagContainer.h"
#include "Net/UnrealNetwork.h"
#include "Gas/ArenaAttributeSet.h"

void AProjectile_Orbit::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AProjectile_Orbit, OrbitRadius);
	DOREPLIFETIME(AProjectile_Orbit, OrbitSpeed);
	DOREPLIFETIME(AProjectile_Orbit, CurrentAngle);
}

void AProjectile_Orbit::Initialize(float InRadius, float InSpeed, float InAngleOffset)
{
	OrbitRadius = InRadius;
	OrbitSpeed = InSpeed;
	CurrentAngle = InAngleOffset;

	// 초기 위치 설정
	if (GetOwner())
	{
		float AngleRad = FMath::DegreesToRadians(CurrentAngle);
		FVector StartLocation = GetOwner()->GetActorLocation() + FVector(
			FMath::Cos(AngleRad) * OrbitRadius,
			FMath::Sin(AngleRad) * OrbitRadius,
			0.0f
		);
		SetActorLocation(StartLocation);
	}
}

void AProjectile_Orbit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ACharBase* Character = Cast<ACharBase>(GetOwner());
	if (!Character || Character->GetIsDead())
	{
		Destroy();
		return;
	}

	// 각도 업데이트
	CurrentAngle += OrbitSpeed * DeltaTime;
	if (CurrentAngle >= 360.0f)
	{
		CurrentAngle -= 360.0f;
	}

	// 새 위치 계산 및 설정
	float AngleRad = FMath::DegreesToRadians(CurrentAngle);
	FVector NewLocation = Character->GetActorLocation() + FVector(
		FMath::Cos(AngleRad) * OrbitRadius,
		FMath::Sin(AngleRad) * OrbitRadius,
		0.0f
	);
	SetActorLocation(NewLocation);
}

void AProjectile_Orbit::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	if (OtherActor == GetOwner())
	{
		return;
	}

	if (!HasAuthority())
	{
		return;
	}

	if (OtherActor && DamageEffect)
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
		if (TargetASC)
		{
			UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
			FGameplayEffectContextHandle ContextHandle = SourceASC ? SourceASC->MakeEffectContext() : TargetASC->MakeEffectContext();
			ContextHandle.AddHitResult(SweepResult);
			ContextHandle.AddInstigator(GetOwner(), this);

			FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(DamageEffect, 1.0f, ContextHandle);
			if (SpecHandle.IsValid())
			{
				float FinalDamage = Damage;
				if (SourceASC && AttackPowerMultiplier > 0.0f)
				{
					const UArenaAttributeSet* SourceAttributeSet = SourceASC->GetSet<UArenaAttributeSet>();
					if (SourceAttributeSet)
					{
						float AttackPower = SourceAttributeSet->GetAttackPower();
						FinalDamage += AttackPower * AttackPowerMultiplier;
					}
				}

				FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(TEXT("Data.Damage"));
				SpecHandle.Data.Get()->SetSetByCallerMagnitude(DamageTag, FinalDamage);

				// GE 적용
				TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
	ExecuteHitGameplayCue(OtherActor, SweepResult, false);
}