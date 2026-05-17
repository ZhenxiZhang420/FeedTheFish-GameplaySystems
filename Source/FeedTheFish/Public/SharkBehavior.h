#pragma once

#include "CoreMinimal.h"
#include "EnemyBehaviorBase.h"
#include "SharkBehavior.generated.h"

class AEnemyActor;
class APlayerCharacter;

/**
 * 鲨鱼行为组件：使用狂暴技能，短时间提升攻击速度并控制攻击节奏
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FEEDTHEFISH_API USharkBehavior : public UEnemyBehaviorBase
{
    GENERATED_BODY()

public:
    virtual void StartBehavior_Implementation() override;

    /** 攻击行为入口，每次攻击时由 CombatComponent 调用 */
    virtual void ExecuteAttackBehavior_Implementation() override;

    /** 尝试释放狂暴技能。返回 true 表示成功释放并接管攻击逻辑 */
    bool TryActivateBerserk();

protected:
    void PerformBerserkAttack();
    void StopBerserk();

private:
    APlayerCharacter* TargetPlayer = nullptr;

    // 狂暴技能参数
    UPROPERTY(EditAnywhere, Category = "Berserk Skill")
        float AttackSpeedMultiplier = 2.5f;

    UPROPERTY(EditAnywhere, Category = "Berserk Skill")
        float BerserkDuration = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Berserk Skill")
        float SkillCooldown = 5.0f;

    float LastSkillTime = -100.f;
    float OriginalAttackSpeed = 1.0f;

    FTimerHandle BerserkAttackTimerHandle;
    FTimerHandle BerserkEndHandle;
};
