#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyBehaviorBase.generated.h"

class AEnemyActor;

/**
 * 敌人行为基类：所有行为组件继承它，实现初始化行为与攻击行为
 */
UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FEEDTHEFISH_API UEnemyBehaviorBase : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnemyBehaviorBase();

    /** 初始化行为（通常在 BeginPlay 调用） */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Behavior")
        void StartBehavior();
    virtual void StartBehavior_Implementation();

    /** 每次攻击触发时调用，由子类决定怎么攻击 */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Behavior")
        void ExecuteAttackBehavior();
    virtual void ExecuteAttackBehavior_Implementation();

    bool bUseBehavior;

protected:
    virtual void BeginPlay() override;

    UPROPERTY()
        AEnemyActor* OwnerEnemy;
};
