#include "EnemyBehaviorBase.h"
#include "EnemyActor.h"
#include "EnemyCombatComponent.h"
#include "CombatSystem.h"
#include "PlayerCharacter.h"

UEnemyBehaviorBase::UEnemyBehaviorBase()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UEnemyBehaviorBase::BeginPlay()
{
    Super::BeginPlay();

    OwnerEnemy = Cast<AEnemyActor>(GetOwner());
}

void UEnemyBehaviorBase::StartBehavior_Implementation()
{
    // 默认无行为，可在子类中实现巡逻/远程/AI
}

void UEnemyBehaviorBase::ExecuteAttackBehavior_Implementation()
{

}
