#include "Character/ProjectileBase.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "GameplayCueManager.h"
#include "NiagaraComponent.h"




// Sets default values
AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// 콜리전 컴포넌트
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(15.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionObjectType(ECC_Projectile);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionComponent->SetGenerateOverlapEvents(true);
	RootComponent = CollisionComponent;

	// 메시 컴포넌트
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CollisionComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 나이아가라 컴포넌트
	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	NiagaraComponent->SetupAttachment(CollisionComponent);
	NiagaraComponent->bAutoActivate = true;

	// 프로젝타일 무브먼트 컴포넌트
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->Bounciness = 1.0f;
	// 수명
	InitialLifeSpan = LifeSpan;
}

void AProjectileBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AProjectileBase, Damage);
	DOREPLIFETIME(AProjectileBase, Speed);
	DOREPLIFETIME(AProjectileBase, MaxBounces);
	DOREPLIFETIME(AProjectileBase, Bounciness);
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	if (ProjectileMovement)
	{
		if (!ProjectileMovement->Velocity.IsNearlyZero())
		{
			ProjectileMovement->Velocity = ProjectileMovement->Velocity.GetSafeNormal() * Speed;
		}
	}
	if (CollisionComponent)
	{
		CollisionComponent->OnComponentHit.AddDynamic(this, &AProjectileBase::OnHit);
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AProjectileBase::OnOverlapBegin);
		if (AActor* OwnerActor = GetOwner())
		{
			CollisionComponent->IgnoreActorWhenMoving(OwnerActor, true);
		}
	}
}


void AProjectileBase::SetDamage(float NewDamage)
{
	Damage = NewDamage;
}

void AProjectileBase::SetSpeed(float NewSpeed)
{
	Speed = NewSpeed;
	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = Speed;
		ProjectileMovement->MaxSpeed = Speed;
	}
}

void AProjectileBase::SetMaxBounces(int32 NewMaxBounces)
{
	MaxBounces = NewMaxBounces;
}

void AProjectileBase::SetBounciness(float NewBounciness)
{
	Bounciness = NewBounciness;
	if (ProjectileMovement)
	{
		ProjectileMovement->Bounciness = Bounciness;
	}
	UE_LOG(LogTemp, Warning, TEXT("[%s] SetBounciness: %f"),
		HasAuthority() ? TEXT("서버") : TEXT("클라"), Bounciness);
}

void AProjectileBase::Launch(const FVector& Direction)
{
	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = Direction.GetSafeNormal() * Speed;
	}
}

void AProjectileBase::OnRep_MaxBounces()
{
}

void AProjectileBase::OnRep_Bounciness()
{
	if (ProjectileMovement)
	{
		ProjectileMovement->Bounciness = Bounciness;
	}
	UE_LOG(LogTemp, Warning, TEXT("[클라-OnRep] Bounciness: %f"), Bounciness);
}

void AProjectileBase::OnRep_Damage()
{
}

void AProjectileBase::OnRep_Speed()
{
	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = Speed;
		ProjectileMovement->MaxSpeed = Speed;
	}
}



void AProjectileBase::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
}

void AProjectileBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void AProjectileBase::ExecuteHitGameplayCue(AActor* TargetActor, const FHitResult& HitResult, bool bIsHit)
{
	if (!TargetActor)
	{
		return;
	}

	// Gameplay Cue Parameters 설정
	FGameplayCueParameters CueParams;
	// HitResult가 없으면 실행시킨 투사체의 위치와 방향을 넣음
	CueParams.Location = HitResult.ImpactPoint.IsZero() ? FVector(GetActorLocation()) : FVector(HitResult.ImpactPoint);
	CueParams.Normal = HitResult.ImpactNormal.IsZero() ? FVector(GetActorForwardVector()) : FVector(HitResult.ImpactNormal);
	CueParams.PhysicalMaterial = HitResult.PhysMaterial;
	CueParams.Instigator = GetInstigator();
	CueParams.EffectCauser = this;
	// HitResult 설정 (EffectContext)
	FGameplayEffectContextHandle ContextHandle = FGameplayEffectContextHandle(UAbilitySystemGlobals::Get().AllocGameplayEffectContext());
	ContextHandle.AddHitResult(HitResult);
	CueParams.EffectContext = ContextHandle;

	// 대상이 ASC를 가지고 있으면 ASC를 통해 실행 (자동 복제)
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (TargetASC)
	{
		TargetASC->ExecuteGameplayCue(bIsHit?HitCueTag:OverlapCueTag, CueParams);
	}
	else
	{
		// ASC가 없으면 GameplayCue Manager를 통해 실행
		if (UWorld* World = GetWorld())
		{
			if (UGameplayCueManager* CueManager = UAbilitySystemGlobals::Get().GetGameplayCueManager())
			{
				CueManager->HandleGameplayCue(TargetActor, bIsHit ? HitCueTag : OverlapCueTag, EGameplayCueEvent::Executed, CueParams);
			}
		}
	}
}