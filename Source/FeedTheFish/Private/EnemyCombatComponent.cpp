#include "EnemyCombatComponent.h"
#include "EnemyActor.h"
#include "PlayerCharacter.h"
#include "CharacterAttributeComponent.h"
#include "CombatSystem.h"
#include "Kismet/GameplayStatics.h"
#include "PaperFlipbookComponent.h"
#include "PaperFlipbook.h"
#include "SkillManager.h"
#include "EnemyPatrolComponent.h"
#include "EnemyBehaviorBase.h"

UEnemyCombatComponent::UEnemyCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UEnemyCombatComponent::BeginPlay()
{
    Super::BeginPlay();
    TargetPlayer = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
}

void UEnemyCombatComponent::InitializeCombat(AEnemyActor* InOwnerEnemy)
{
    OwnerEnemy = InOwnerEnemy;
}

void UEnemyCombatComponent::StartAttackingPlayer()
{
    if (bIsAttacking || !TargetPlayer || !OwnerEnemy) return;

    UE_LOG(LogTemp, Warning, TEXT("Passed all checks, starting attack"));

    if (OwnerEnemy->PatrolComponent)
    {
        OwnerEnemy->PatrolComponent->StopPatrol();
    }

    bIsAttacking = true;

    float Interval = 1.f / FMath::Max(OwnerEnemy->AttackSpeed, 0.01f);

    // ✅ 播放攻击动画（使用 FlipbookMap）
    if (OwnerEnemy->FlipbookMap.Contains("BasicAttack"))
    {
        float PlayRate = OwnerEnemy->FlipbookMap["BasicAttack"]->GetTotalDuration() * OwnerEnemy->AttackSpeed;
        OwnerEnemy->PlayFlipbookByName("BasicAttack", PlayRate, true);
    }

    // ✅ 设置朝向
    if (OwnerEnemy->FlipbookComponent && TargetPlayer)
    {
        float DirX = TargetPlayer->GetActorLocation().X - OwnerEnemy->GetActorLocation().X;

        FVector Scale = OwnerEnemy->FlipbookComponent->GetComponentScale();
        Scale.X = (DirX >= 0) ? -FMath::Abs(Scale.X) : +FMath::Abs(Scale.X);
        OwnerEnemy->FlipbookComponent->SetWorldScale3D(Scale);
    }

    UE_LOG(LogTemp, Warning, TEXT("启用攻击定时器"));
    // ✅ 启动攻击定时器
    GetWorld()->GetTimerManager().SetTimer(
        AttackTimerHandle,
        this,
        &UEnemyCombatComponent::AttackPlayer,
        Interval,
        true
    );
}

void UEnemyCombatComponent::StopAttackingPlayer()
{
    if (!bIsAttacking) return;

    UE_LOG(LogTemp, Warning, TEXT("清除定时器"));
    bIsAttacking = false;
    GetWorld()->GetTimerManager().ClearTimer(AttackTimerHandle);
}

void UEnemyCombatComponent::AttackPlayer()
{
    if (!TargetPlayer || !OwnerEnemy || !OwnerEnemy->IsAlive())
    {
        StopAttackingPlayer();
        return;
    }

    UCharacterAttributeComponent* Attr = TargetPlayer->FindComponentByClass<UCharacterAttributeComponent>();
    if (!Attr || !Attr->IsAlive())
    {
        if (OwnerEnemy && OwnerEnemy->PatrolComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("目标已死亡，敌人回归巡逻状态"));
            OwnerEnemy->PatrolComponent->BeginMove();
            OwnerEnemy->PlayFlipbookByName("Idle", 1.0f, true);
        }

        StopAttackingPlayer();
        return;
    }

    // ✅ 行为组件执行技能（如狂暴等）
    if (OwnerEnemy && OwnerEnemy->BehaviorInstance && OwnerEnemy->BehaviorInstance->bUseBehavior)
    {
        OwnerEnemy->BehaviorInstance->ExecuteAttackBehavior();
    }

    // ✅ 默认普通攻击
    UCombatSystem::ApplyDamage(OwnerEnemy, TargetPlayer, OwnerEnemy->BaseDamage, FSkillData());
}

void UEnemyCombatComponent::ClearAttackTimerOnly()
{
    GetWorld()->GetTimerManager().ClearTimer(AttackTimerHandle);
}

void UEnemyCombatComponent::StartAttackTimerOnly()
{
    if (!TargetPlayer || !OwnerEnemy) return;

    // ✅ 检查玩家是否还活着
    if (UCharacterAttributeComponent* Attr = TargetPlayer->FindComponentByClass<UCharacterAttributeComponent>())
    {
        if (!Attr->IsAlive())
        {
            return;
        }
    }

    float Interval = 1.f / FMath::Max(OwnerEnemy->AttackSpeed, 0.01f);

    // ✅ 播放攻击动画（使用 FlipbookMap）
    if (OwnerEnemy->FlipbookMap.Contains("BasicAttack"))
    {
        float PlayRate = OwnerEnemy->FlipbookMap["BasicAttack"]->GetTotalDuration() * OwnerEnemy->AttackSpeed;
        OwnerEnemy->PlayFlipbookByName("BasicAttack", PlayRate, true);
    }

    // ✅ 启动攻击定时器
    GetWorld()->GetTimerManager().SetTimer(
        AttackTimerHandle,
        this,
        &UEnemyCombatComponent::AttackPlayer,
        Interval,
        true
    );
}
