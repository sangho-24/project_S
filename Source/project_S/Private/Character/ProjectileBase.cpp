#include "Character/ProjectileBase.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "GameplayCueManager.h"



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
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMeshAsset.Succeeded())
	{
		MeshComponent->SetStaticMesh(SphereMeshAsset.Object);
		MeshComponent->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f));
	}

	// 프로젝타일 무브먼트 컴포넌트
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	// 수명
	InitialLifeSpan = LifeSpan;
}

void AProjectileBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AProjectileBase, Damage);
	DOREPLIFETIME(AProjectileBase, Speed);
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	if (CollisionComponent)
	{
		CollisionComponent->OnComponentHit.AddDynamic(this, &AProjectileBase::OnHit);
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AProjectileBase::OnOverlapBegin);
		if (AActor* OwnerActor = GetOwner())
		{
			CollisionComponent->IgnoreActorWhenMoving(OwnerActor, true);
		}
	}
	if (!CueTag.IsValid())
	{
		CueTag = FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Projectile.Hit"));
	}
}

// Called every frame
void AProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

void AProjectileBase::Launch(const FVector& Direction)
{
	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = Direction.GetSafeNormal() * Speed;
	}
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
	if (OtherActor == GetOwner())
	{
		return;
	}

	ExecuteHitGameplayCue(OtherActor, Hit);
	if (HasAuthority())
	{
	UE_LOG(LogTemp, Warning, TEXT("투사체가 벽에 충돌! 대상: %s"), *OtherActor->GetName());
	Destroy();
	}
}

void AProjectileBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == GetOwner())
	{
		return;
	}

	if (!HasAuthority())
	{
		return;
	}

	// GAS 데미지 적용
	if (OtherActor && DamageEffect)
	{	// 대상 ASC 가져오기
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
		if (TargetASC)
		{
			// 공격자 ASC 가져오기
			UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
			// Context 생성(공격자 ASC우선, 없으면 타겟ASC에 생성)
			FGameplayEffectContextHandle ContextHandle = SourceASC ? SourceASC->MakeEffectContext() : TargetASC->MakeEffectContext();
			ContextHandle.AddHitResult(SweepResult);
			ContextHandle.AddInstigator(GetOwner(), this);

			// Spec 생성
			FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(DamageEffect, 1.0f, ContextHandle);

			if (SpecHandle.IsValid())
			{
				FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(TEXT("Data.Damage"));
				SpecHandle.Data.Get()->SetSetByCallerMagnitude(DamageTag, -Damage);

				// GE 적용
				TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				UE_LOG(LogTemp, Warning, TEXT("투사체 적중! 대상: %s, 데미지: %.1f"), *OtherActor->GetName(), Damage);
			}
		}
	}
	// Gameplay Cue 실행
	ExecuteHitGameplayCue(OtherActor, SweepResult);
	Destroy();
}

void AProjectileBase::ExecuteHitGameplayCue(AActor* TargetActor, const FHitResult& HitResult)
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
		TargetASC->ExecuteGameplayCue(CueTag, CueParams);
	}
	else
	{
		// ASC가 없으면 GameplayCue Manager를 통해 실행
		if (UWorld* World = GetWorld())
		{
			if (UGameplayCueManager* CueManager = UAbilitySystemGlobals::Get().GetGameplayCueManager())
			{
				CueManager->HandleGameplayCue(TargetActor, CueTag, EGameplayCueEvent::Executed, CueParams);
			}
		}
	}
}