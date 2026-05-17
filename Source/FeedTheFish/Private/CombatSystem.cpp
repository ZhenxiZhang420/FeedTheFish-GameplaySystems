/*
 * CombatSystem.cpp
 *
 * Central combat calculation utility for applying damage between players and enemies.
 * Responsibilities:
 * - Calculate final damage based on attacker, target, skill data, defense, critical hits, and dodge.
 * - Apply damage to enemy or player targets.
 * - Handle lifesteal recovery after successful damage.
 */

#include "CombatSystem.h"
#include "EnemyActor.h"
#include "SkillManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Actor.h"
#include "PlayerCharacter.h"
#include "CharacterAttributeComponent.h"

void UCombatSystem::ApplyDamage(AActor* Attacker, AActor* Target, float BaseDamage, FSkillData SkillUsed)
{
    if (!Target) return;

    float FinalDamage = CalculateDamage(Attacker, Target, BaseDamage, &SkillUsed);

    if (FinalDamage <= 0.f) return; // Damage may be fully avoided by dodge logic

    if (AEnemyActor* Enemy = Cast<AEnemyActor>(Target))
    {
        FString AttackType = SkillUsed.Name.IsEmpty()
            ? TEXT("普通攻击")
            : FString::Printf(TEXT("技能【%s】"), *SkillUsed.Name);

        UE_LOG(LogTemp, Log, TEXT("玩家使用%s造成了 %.1f 点伤害"), *AttackType, FinalDamage);

        Enemy->ReceiveDamage(FinalDamage, Attacker);
    }
    else if (APlayerCharacter* Player = Cast<APlayerCharacter>(Target))
    {
        UE_LOG(LogTemp, Warning, TEXT("怪物造成了 %.1f 点伤害"), FinalDamage);

        if (UCharacterAttributeComponent* Attr = Player->FindComponentByClass<UCharacterAttributeComponent>())
        {
            Attr->ReceiveDamage(FinalDamage);
        }
    }

    // Apply lifesteal after successful damage
    if (Attacker)
    {
        if (UCharacterAttributeComponent* AttackerAttr = Attacker->FindComponentByClass<UCharacterAttributeComponent>())
        {
            float LifeStealRate = AttackerAttr->GetTotalLifeSteal();
            if (LifeStealRate > 0.f)
            {
                float HealAmount = FinalDamage * LifeStealRate;
                AttackerAttr->CurrentHP = FMath::Clamp(AttackerAttr->CurrentHP + HealAmount, 0.f, AttackerAttr->GetTotalMaxHP());

                UE_LOG(LogTemp, Log, TEXT("玩家吸血回复了 %.1f 生命"), HealAmount);
            }
        }
    }
}


float UCombatSystem::CalculateDamage(AActor* Attacker, AActor* Target, float BaseDamage, const FSkillData* SkillUsed)
{
    if (!Attacker || !Target) return 0.f;

    bool bAttackerIsPlayer = Attacker->IsA(APlayerCharacter::StaticClass());
    bool bTargetIsPlayer = Target->IsA(APlayerCharacter::StaticClass());

    float FinalDamage = BaseDamage;

    if (bAttackerIsPlayer && !bTargetIsPlayer)
    {
        UCharacterAttributeComponent* AttackerAttr = Attacker->FindComponentByClass<UCharacterAttributeComponent>();
        if (!AttackerAttr) return BaseDamage;

        //FinalDamage = AttackerAttr->GetTotalAttack();
        //if (SkillUsed)
        //{
        //    FinalDamage *= SkillUsed->DamageMultiplier;
        //}

        // Reduce player damage by enemy defense
        if (AEnemyActor* Enemy = Cast<AEnemyActor>(Target))
        {
            FinalDamage = FMath::Max(1.f, FinalDamage - Enemy->Defense);
        }

        // Apply player critical hit chance
        if (FMath::FRand() < (AttackerAttr->GetTotalCritRate() / 100.f))
        {
            FinalDamage *= 1.5f;
            UE_LOG(LogTemp, Log, TEXT("%s 暴击了怪物，造成 %.1f 伤害！"), *Attacker->GetName(), FinalDamage);
        }
    }
    else if (!bAttackerIsPlayer && bTargetIsPlayer)
    {
        UCharacterAttributeComponent* TargetAttr = Target->FindComponentByClass<UCharacterAttributeComponent>();
        if (!TargetAttr) return BaseDamage;

        // Apply player dodge chance
        if (FMath::FRand() < (TargetAttr->GetTotalDodgeRate() / 100.f))
        {
            UE_LOG(LogTemp, Log, TEXT("玩家闪避了怪物攻击！"));
            return 0.f;
        }

        FinalDamage = FMath::Max(1.f, BaseDamage - TargetAttr->GetTotalDefense());
    }

    return FinalDamage;
}




