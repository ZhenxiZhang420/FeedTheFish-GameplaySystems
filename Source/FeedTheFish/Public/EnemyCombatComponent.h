#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyCombatComponent.generated.h"

class AEnemyActor;
class APlayerCharacter;
class UPaperFlipbook;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FEEDTHEFISH_API UEnemyCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnemyCombatComponent();

    void InitializeCombat(AEnemyActor* InOwnerEnemy);
    void StartAttackingPlayer();
    void StopAttackingPlayer();
    void AttackPlayer();

    void ClearAttackTimerOnly();
    void StartAttackTimerOnly();

protected:
    virtual void BeginPlay() override;

private:
    FTimerHandle AttackTimerHandle;
    AEnemyActor* OwnerEnemy = nullptr;
    APlayerCharacter* TargetPlayer = nullptr;
    bool bIsAttacking = false;

    TMap<FName, float> SkillCooldownMap;
};
