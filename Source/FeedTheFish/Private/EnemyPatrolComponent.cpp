#include "EnemyPatrolComponent.h"
#include "EnemyActor.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

UEnemyPatrolComponent::UEnemyPatrolComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UEnemyPatrolComponent::BeginPlay()
{
    Super::BeginPlay();
    PatrolCenter = GetOwner()->GetActorLocation();
    BeginMove();

    //DrawDebugCircle(
    //    GetWorld(),
    //    PatrolCenter,
    //    PatrolRadius,
    //    50,                        // 边数
    //    FColor::Cyan,
    //    true,                      // 持久化（游戏中一直显示）
    //    -1,                        // 生命周期，-1表示永久
    //    0,
    //    5.0f,                      // 厚度
    //    FVector(1, 0, 0),          // X轴方向
    //    FVector(0, 0, 1),          // Y轴方向
    //    false                      // 不要在Z轴投影
    //);
}

void UEnemyPatrolComponent::InitializePatrol(AEnemyActor* InOwnerEnemy)
{
    OwnerEnemy = InOwnerEnemy;
}

void UEnemyPatrolComponent::BeginMove()
{
    bMoving = true;
    ChooseNewTarget();
}

void UEnemyPatrolComponent::ChooseNewTarget()
{
    const FVector RandomOffset = FVector(
        FMath::FRandRange(-PatrolRadius, PatrolRadius),
        0.f,
        FMath::FRandRange(-PatrolRadius, PatrolRadius)
    );
    CurrentTarget = PatrolCenter + RandomOffset;
}

void UEnemyPatrolComponent::StopMove()
{
    bMoving = false;
    GetWorld()->GetTimerManager().SetTimer(WaitTimerHandle, this, &UEnemyPatrolComponent::BeginMove, WaitTime, false);
}

void UEnemyPatrolComponent::StopPatrol()
{
    bMoving = false;
    GetWorld()->GetTimerManager().ClearTimer(WaitTimerHandle);
}


void UEnemyPatrolComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bMoving || !OwnerEnemy) return;

    FVector CurrentLocation = OwnerEnemy->GetActorLocation();
    FVector Direction = (CurrentTarget - CurrentLocation);
    FVector MoveDirection = Direction.GetSafeNormal();

    if (Direction.SizeSquared() < KINDA_SMALL_NUMBER)
    {
        StopMove();
        return;
    }

    // ✅ 设置朝向（仅在X轴翻转 Flipbook）
    if (OwnerEnemy->FlipbookComponent)
    {
        float DirX = MoveDirection.X;
        FVector Scale = OwnerEnemy->FlipbookComponent->GetComponentScale();
        Scale.X = (DirX >= 0) ? -FMath::Abs(Scale.X) : FMath::Abs(Scale.X);
        OwnerEnemy->FlipbookComponent->SetWorldScale3D(Scale);
    }

    // ✅ 移动
    FVector NewLocation = CurrentLocation + MoveDirection * MoveSpeed * DeltaTime;
    OwnerEnemy->SetActorLocation(NewLocation);

    // ✅ 到达目标点？
    if (FVector::DistSquared(NewLocation, CurrentTarget) < 25.f * 25.f)
    {
        StopMove();
    }
}


void UEnemyPatrolComponent::InterruptPatrolForDuration(float Duration)
{
    bMoving = false;
    GetWorld()->GetTimerManager().ClearTimer(WaitTimerHandle);
    GetWorld()->GetTimerManager().SetTimer(WaitTimerHandle, this, &UEnemyPatrolComponent::BeginMove, Duration, false);
}

