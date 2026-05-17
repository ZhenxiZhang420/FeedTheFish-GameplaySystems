#pragma once

#include "CoreMinimal.h"
#include "EnemyBehaviorBase.h"
#include "EliteSharkBehavior.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FEEDTHEFISH_API UEliteSharkBehavior : public UEnemyBehaviorBase
{
    GENERATED_BODY()

public:
    virtual void ExecuteAttackBehavior_Implementation() override;

protected:
    virtual void BeginPlay() override;

    // 技能冷却相关
    bool bCanUseSmash = true;

    UPROPERTY(EditAnywhere, Category = "Smash Skill")
        float SmashCooldown = 5.0f;

    FTimerHandle SmashCooldownHandle;

    // 每段攻击的 TimerHandle
    UPROPERTY()
        TArray<FTimerHandle> SmashAttackHandles;

    void PerformSmashAttack(int32 HitIndex);
};
