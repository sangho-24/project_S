#include "Character/Projectile_Homing.h"
#include "Character/CharBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagContainer.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

AProjectile_Homing::AProjectile_Homing()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AProjectile_Homing::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AProjectile_Homing, HomingPower);
	DOREPLIFETIME(AProjectile_Homing, HomingRadius);
}

void AProjectile_Homing::BeginPlay()
{
	Super::BeginPlay();

	// 주기적으로 타겟 업데이트
	GetWorldTimerManager().SetTimer(
		TargetUpdateTimerHandle,
		this,
		&AProjectile_Homing::UpdateHomingTarget,
		0.1f,
		true
	);
}

void AProjectile_Homing::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ProjectileMovement && ProjectileMovement->bIsHomingProjectile)
	{
		// Z축 호밍 비활성화 (속도 유지)
		FVector Velocity = ProjectileMovement->Velocity;
		float OriginalSpeed = Velocity.Size(); // 원래 속도 저장

		Velocity.Z = 0.0f;

		if (!Velocity.IsNearlyZero())
		{
			// 원래 속도 복원
			Velocity = Velocity.GetSafeNormal() * OriginalSpeed;
			ProjectileMovement->Velocity = Velocity;
		}
	}
}

void AProjectile_Homing::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::OnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
	if (OtherActor == GetOwner())
	{
		return;
	}

	ExecuteHitGameplayCue(OtherActor, Hit, true);
	if (HasAuthority())
	{
		Destroy();
	}
}

void AProjectile_Homing::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
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
				FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(TEXT("Data.Damage"));
				SpecHandle.Data.Get()->SetSetByCallerMagnitude(DamageTag, Damage);

				TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
	ExecuteHitGameplayCue(OtherActor, SweepResult, false);
	Destroy();
}

void AProjectile_Homing::UpdateHomingTarget()
{
	if (!ProjectileMovement)
	{
		return;
	}

	if (ProjectileMovement->bIsHomingProjectile && CurrentTarget.IsValid())
	{
		return;
	}

	ACharBase* NewTarget = FindNearestEnemy();

	if (NewTarget)
	{
		CurrentTarget = NewTarget;

		ProjectileMovement->bIsHomingProjectile = true;
		ProjectileMovement->HomingTargetComponent = NewTarget->GetRootComponent();
		ProjectileMovement->HomingAccelerationMagnitude = HomingPower;
	}
	else
	{
		ProjectileMovement->bIsHomingProjectile = false;
		CurrentTarget = nullptr;
	}
}

ACharBase* AProjectile_Homing::FindNearestEnemy() const
{
	if (!GetOwner())
	{
		return nullptr;
	}

	ACharBase* NearestEnemy = nullptr;
	float SearchDist = HomingRadius * HomingRadius;
	FVector CurrentLocation = GetActorLocation();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharBase::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (Actor == GetOwner())
		{
			continue;
		}

		ACharBase* CharActor = Cast<ACharBase>(Actor);
		if (CharActor && CharActor->GetIsDead())
		{
			continue;
		}

		// 수평 거리만 계산 (Z축 제외)
		FVector ToTarget = CharActor->GetActorLocation() - CurrentLocation;
		ToTarget.Z = 0.0f;
		float Dist = ToTarget.SizeSquared();

		if (Dist < SearchDist)
		{
			SearchDist = Dist;
			NearestEnemy = CharActor;
		}
	}

	return NearestEnemy;
}

void AProjectile_Homing::SetHomingPower(float NewHomingPower)
{
	HomingPower = NewHomingPower;
}

void AProjectile_Homing::SetHomingRadius(float NewHomingRadius)
{
	HomingRadius = NewHomingRadius;
}