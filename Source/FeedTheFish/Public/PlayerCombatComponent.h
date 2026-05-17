// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "PlayerCombatComponent.generated.h"

class AEnemyActor;
class UCharacterAttributeComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FEEDTHEFISH_API UPlayerCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    UPlayerCombatComponent();

    virtual void BeginPlay() override;

    // 初始化战斗组件（设置攻击碰撞体等）
    void Initialize();

    UPROPERTY(EditAnywhere, Category = "Effects")
        UNiagaraSystem* DefaultHitEffect;

    UFUNCTION(BlueprintCallable, Category = "Combat")
        void HandleAttackOverlap(AActor* OtherActor);

    void StartAutoAttack(AEnemyActor* Enemy);
    void PerformAttack(AEnemyActor* Enemy);
    void BasicAttack(AActor* Target);
    void PlayAttackAnimation();

    UFUNCTION()
        void StopAutoAttack();

    UFUNCTION()
        void RestartAutoAttack();

    void LockSkillInput(float Duration);
private:
    AActor* OwnerActor;
    UCharacterAttributeComponent* AttributeComponent;

    bool bIsAttacking = false;
    FTimerHandle AttackTimerHandle;

    FTimerHandle SkillLockTimerHandle;
};
