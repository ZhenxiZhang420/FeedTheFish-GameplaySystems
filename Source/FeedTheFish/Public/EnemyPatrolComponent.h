#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyPatrolComponent.generated.h"

class AEnemyActor;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FEEDTHEFISH_API UEnemyPatrolComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnemyPatrolComponent();

    void InitializePatrol(AEnemyActor* InOwnerEnemy);
    void StopPatrol();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    AEnemyActor* OwnerEnemy = nullptr;

    FVector PatrolCenter;
    FVector CurrentTarget;
    bool bMoving = false;

    // Editable parameters
    UPROPERTY(EditAnywhere, Category = "Patrol")
        float PatrolRadius = 250.f;

    UPROPERTY(EditAnywhere, Category = "Patrol")
        float MoveSpeed = 100.f;

    UPROPERTY(EditAnywhere, Category = "Patrol")
        float WaitTime = 1.5f;

    FTimerHandle WaitTimerHandle;

    void ChooseNewTarget();

public:
    void BeginMove();
    void StopMove();

    UFUNCTION(BlueprintCallable)
        void InterruptPatrolForDuration(float Duration);
};
