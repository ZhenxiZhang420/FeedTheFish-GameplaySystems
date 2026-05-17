// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PaperSpriteComponent.h"
#include "Components/BoxComponent.h"
#include "FollowCameraActor.h"
#include "PaperFlipbookComponent.h" 
#include "Components/WidgetComponent.h"
#include "PlayerCharacter.generated.h"


class USkillManager;
class UCharacterAttributeComponent;
class UPlayerCombatComponent;
class UPaperSpriteComponent;
class UBoxComponent;

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
    Normal,
    Stunned,
    Dead
};

UCLASS()
class FEEDTHEFISH_API APlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    APlayerCharacter();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
        float SwimSpeed = 600.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
        UWidgetComponent* WidgetComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
        UPaperSpriteComponent* SpriteComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
        AFollowCameraActor* FollowCameraActor;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
        ECharacterState CurrentState = ECharacterState::Normal;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
        UBoxComponent* AttackCollider;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flipbook")
        UPaperFlipbookComponent* FlipbookComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flipbook")
        UPaperFlipbook* IdleFlipbook;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flipbook")
        UPaperFlipbook* BasicAttackFlipbook;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flipbook")
        UPaperFlipbook* AttackFlipbook;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flipbook")
        UPaperFlipbook* DeathFlipbook;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        USkillManager* SkillManager;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        UCharacterAttributeComponent* AttributeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        UPlayerCombatComponent* CombatComponent;

    UFUNCTION(BlueprintCallable)
        bool TryUseSkill(FName SkillID, AActor* Caster, AActor* Target);

    // 移动 & 目标控制
    UPROPERTY()
        FVector TargetMoveLocation;

    UPROPERTY()
        bool bIsMoving = false;

    UFUNCTION(BlueprintCallable)
        void MoveToLocation(FVector Location);

    UPROPERTY(BlueprintReadOnly)
        AActor* TargetEnemy = nullptr;

    UFUNCTION(BlueprintCallable)
        void SetTargetEnemy(AActor* Enemy);

    UFUNCTION(BlueprintCallable)
        void ClearTargetEnemy();

    UFUNCTION()
        void OnCharacterDeath();

};

