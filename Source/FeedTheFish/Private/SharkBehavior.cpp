#include "SharkBehavior.h"
#include "EnemyActor.h"
#include "PlayerCharacter.h"
#include "EnemyCombatComponent.h"
#include "CharacterAttributeComponent.h"
#include "CombatSystem.h"
#include "Kismet/GameplayStatics.h"
#include "PaperFlipbookComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "PaperFlipbook.h"

void USharkBehavior::StartBehavior_Implementation()
{
    TargetPlayer = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
}

void USharkBehavior::ExecuteAttackBehavior_Implementation()
{
    TryActivateBerserk(); // 尝试释放狂暴，若成功将接管攻击流程
}

bool USharkBehavior::TryActivateBerserk()
{   
    if (!OwnerEnemy || !OwnerEnemy->IsAlive()) return false;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastSkillTime < SkillCooldown)
    {
        return false; // 技能冷却中
    }

    // 保存原始攻速并修改
    OriginalAttackSpeed = OwnerEnemy->AttackSpeed;
    OwnerEnemy->AttackSpeed = OriginalAttackSpeed * AttackSpeedMultiplier;

    LastSkillTime = CurrentTime;

    UE_LOG(LogTemp, Warning, TEXT("鲨鱼进入狂暴状态！攻速提升为 %.2f"), OwnerEnemy->AttackSpeed);

    // 停止普通攻击定时器
    if (OwnerEnemy->CombatComponent)
    {
        OwnerEnemy->CombatComponent->ClearAttackTimerOnly();
    }

    // 开启狂暴攻击循环
    float Interval = 1.f / FMath::Max(OwnerEnemy->AttackSpeed, 0.01f);
    GetWorld()->GetTimerManager().SetTimer(BerserkAttackTimerHandle, this, &USharkBehavior::PerformBerserkAttack, Interval, true);

    // 狂暴结束倒计时
    GetWorld()->GetTimerManager().SetTimer(BerserkEndHandle, this, &USharkBehavior::StopBerserk, BerserkDuration, false);

    // 播放狂暴动画（如果有）
    float PlayRate = OwnerEnemy->FlipbookMap["BasicAttack"]->GetTotalDuration() * OwnerEnemy->AttackSpeed;
    OwnerEnemy->PlayFlipbookByName("BasicAttack", PlayRate, true);

    return true;
}

void USharkBehavior::PerformBerserkAttack()
{
    if (!OwnerEnemy || !OwnerEnemy->CombatComponent || !OwnerEnemy->IsAlive()) return;

    OwnerEnemy->CombatComponent->AttackPlayer(); // 重复调用普通攻击逻辑
}

void USharkBehavior::StopBerserk()
{
    if (!OwnerEnemy || !OwnerEnemy->IsAlive()) return;

    OwnerEnemy->AttackSpeed = OriginalAttackSpeed;
    UE_LOG(LogTemp, Warning, TEXT("鲨鱼狂暴结束，攻速恢复为 %.2f"), OriginalAttackSpeed);

    GetWorld()->GetTimerManager().ClearTimer(BerserkAttackTimerHandle);

     //恢复普通攻击管理
    if (OwnerEnemy->CombatComponent)
    {
        OwnerEnemy->CombatComponent->StartAttackTimerOnly();
    }
}
