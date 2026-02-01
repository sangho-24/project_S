#include "Gas/GA_HomingShot.h"
#include "Character/CharBase.h"
#include "Character/ProjectileBase.h"
#include "Kismet/GameplayStatics.h"

void UGA_HomingShot::ActivateAbility(
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

    FVector SpawnLocation = Character->GetActorLocation();
    FVector TargetDirection = FindTargetDirection(Character, SpawnLocation);

    if (ProjectileClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = Character;
        SpawnParams.Instigator = Character;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        for (int32 i = 0; i < ProjectileCount; i++)
        {
            // 여러 발일 경우 퍼뜨려서 발사
            float AngleOffset = (i - (ProjectileCount - 1) / 2.0f) * 15.0f;
            FVector Direction = TargetDirection.RotateAngleAxis(AngleOffset, FVector::UpVector);
            FRotator SpawnRotation = Direction.Rotation();

            AProjectileBase* Projectile = GetWorld()->SpawnActor<AProjectileBase>(
                ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);

            if (Projectile)
            {
                Projectile->SetDamage(ProjectileDamage);
                Projectile->SetSpeed(ProjectileSpeed);
                Projectile->SetProjectileLifeSpan(ProjectileLifeSpan);
                Projectile->Launch(Direction);
            }
        }
    }
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

FVector UGA_HomingShot::FindTargetDirection(ACharBase* Character, const FVector& SpawnLocation) const
{
    ACharBase* NearestEnemy = nullptr;
    float SearchDist = TargetSearchRadius * TargetSearchRadius;

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(Character->GetWorld(), ACharBase::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (Actor == Character)
        {
            continue;
        }

        ACharBase* CharActor = Cast<ACharBase>(Actor);
        if (CharActor && CharActor->GetIsDead())
        {
            continue;
        }
        // 가장 가까운 적 찾기
        float Dist = FVector::DistSquared(SpawnLocation, CharActor->GetActorLocation());
        if (Dist < SearchDist)
        {
            SearchDist = Dist;
            NearestEnemy = CharActor;
        }
    }

    // 적을 있으면 해당 방향
    if (NearestEnemy)
    {
        FVector Direction = NearestEnemy->GetActorLocation() - SpawnLocation;
        Direction.Z = 0.0f;
        return Direction.GetSafeNormal();
    }

    // 적이 없으면 랜덤 방향
    float RandomAngle = FMath::FRandRange(0.0f, 360.0f);
    float AngleRad = FMath::DegreesToRadians(RandomAngle);
    return FVector(FMath::Cos(AngleRad), FMath::Sin(AngleRad), 0.0f);
}