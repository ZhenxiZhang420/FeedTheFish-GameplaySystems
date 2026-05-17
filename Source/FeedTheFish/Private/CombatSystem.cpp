// Fill out your copyright notice in the Description page of Project Settings.

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

    if (FinalDamage <= 0.f) return; // 可能被闪避

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

    // 吸血逻辑（还是保留）
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

        // ✅ 新增：怪物防御减伤
        if (AEnemyActor* Enemy = Cast<AEnemyActor>(Target))
        {
            FinalDamage = FMath::Max(1.f, FinalDamage - Enemy->Defense);
        }

        // 玩家有暴击
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

        // 玩家有闪避
        if (FMath::FRand() < (TargetAttr->GetTotalDodgeRate() / 100.f))
        {
            UE_LOG(LogTemp, Log, TEXT("玩家闪避了怪物攻击！"));
            return 0.f;
        }

        FinalDamage = FMath::Max(1.f, BaseDamage - TargetAttr->GetTotalDefense());
    }

    return FinalDamage;
}




