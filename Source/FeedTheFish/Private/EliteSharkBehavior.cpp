#include "EliteSharkBehavior.h"
#include "EnemyActor.h"
#include "PlayerCharacter.h"
#include "CharacterAttributeComponent.h"
#include "CombatSystem.h"
#include "Kismet/GameplayStatics.h"
#include "PaperFlipbook.h"
#include "PaperFlipbookComponent.h"
#include "EnemyCombatComponent.h"

void UEliteSharkBehavior::BeginPlay()
{
    Super::BeginPlay();
}

void UEliteSharkBehavior::ExecuteAttackBehavior_Implementation()
{
    if (!OwnerEnemy || !bCanUseSmash || !OwnerEnemy->IsAlive()) return;

    APlayerCharacter* TargetPlayer = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
    bCanUseSmash = false;

    GetWorld()->GetTimerManager().SetTimer(SmashCooldownHandle, [this]()
        {
            bCanUseSmash = true;
            UE_LOG(LogTemp, Log, TEXT("精英鲨鱼 Smash 技能冷却完毕"));
        }, SmashCooldown, false);

    UE_LOG(LogTemp, Log, TEXT("精英鲨鱼使用 Smash 技能（三连击）"));

    if (OwnerEnemy->CombatComponent)
    {
        OwnerEnemy->CombatComponent->ClearAttackTimerOnly();
    }

    SmashAttackHandles.Empty();

    float StartDelay = 0.2f;
    float AnimationDuration = 0.8f;

    if (OwnerEnemy->FlipbookMap.Contains("Attack") && OwnerEnemy->FlipbookMap["Attack"])
    {
        AnimationDuration = OwnerEnemy->FlipbookMap["Attack"]->GetTotalDuration();
    }

    if (OwnerEnemy->FlipbookComponent && OwnerEnemy->FlipbookMap.Contains("Attack"))
    {
        OwnerEnemy->FlipbookComponent->SetFlipbook(OwnerEnemy->FlipbookMap["Attack"]);
        OwnerEnemy->FlipbookComponent->SetPlayRate(1.0f);
        OwnerEnemy->FlipbookComponent->Play();
    }


    int32 TotalHits = 3;

    for (int32 i = 0; i < TotalHits; ++i)
    {
        int32 HitIndex = i + 1;
        float Delay = StartDelay + AnimationDuration * HitIndex;

        FTimerHandle Handle;
        GetWorld()->GetTimerManager().SetTimer(
            Handle,
            [this, HitIndex]()
            {
                PerformSmashAttack(HitIndex);
            },
            Delay,
            false
        );

        SmashAttackHandles.Add(Handle);
    }

    // ✅ 在最后一击之后恢复普攻
    float TotalDuration = 2 * StartDelay + AnimationDuration * TotalHits;
    GetWorld()->GetTimerManager().SetTimer(SmashCooldownHandle, [this]()
        {
            if (OwnerEnemy && OwnerEnemy->CombatComponent)
            {
                OwnerEnemy->CombatComponent->StartAttackTimerOnly();
                UE_LOG(LogTemp, Log, TEXT("精英鲨鱼 Smash 结束，恢复普通攻击"));
            }
        }, TotalDuration, false);

}


void UEliteSharkBehavior::PerformSmashAttack(int32 HitIndex)
{
    if (!OwnerEnemy || !OwnerEnemy->IsAlive()) return;

    APlayerCharacter* TargetPlayer = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
    float Multiplier = 1.0f;
    if (HitIndex == 2) Multiplier = 1.5f;
    else if (HitIndex == 3) Multiplier = 2.0f;

    float FinalDamage = OwnerEnemy->BaseDamage * Multiplier;

    UE_LOG(LogTemp, Log, TEXT("Smash 第 %d 击，造成 %.1f 伤害"), HitIndex, FinalDamage);

    UCombatSystem::ApplyDamage(OwnerEnemy, TargetPlayer, FinalDamage, FSkillData());

    // ✅ 第三击后恢复普通攻击逻辑
    if (HitIndex == 3 && OwnerEnemy->CombatComponent)
    {
        OwnerEnemy->CombatComponent->StartAttackTimerOnly();
    }
}


