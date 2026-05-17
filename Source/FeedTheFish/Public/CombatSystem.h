// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CombatSystem.generated.h"

class AActor;
class AEnemyActor;
class ASomePlayerCharacter;
struct FSkillData;
/**
 * 
 */
UCLASS()
class FEEDTHEFISH_API UCombatSystem : public UObject
{
	GENERATED_BODY()
	
public:
    // 核心接口：攻击
    UFUNCTION(BlueprintCallable, Category = "Combat")
        static void ApplyDamage(AActor* Attacker, AActor* Target, float BaseDamage, FSkillData SkillUsed);

    // 可以拓展伤害计算的函数
    static float CalculateDamage(AActor* Attacker, AActor* Target, float BaseDamage, const FSkillData* SkillUsed);
};
