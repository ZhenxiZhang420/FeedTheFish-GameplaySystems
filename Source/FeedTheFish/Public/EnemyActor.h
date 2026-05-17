#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "PaperFlipbookComponent.h"
#include "SkillManager.h"
#include "EnemyActor.generated.h"

class UPaperSpriteComponent;
class UDataTable;
class APlayerCharacter;
class UPaperFlipbook;
class UEnemyCombatComponent;
class UEnemyPatrolComponent;
class UEnemyBehaviorBase;

USTRUCT(BlueprintType)
struct FEnemyConfig : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FName EnemyID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TMap<FName, class UPaperFlipbook*> Flipbooks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TSubclassOf<UEnemyBehaviorBase> BehaviorClass;

    // ✅ 属性
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float MaxHP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float AttackSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float BaseDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float Defense;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 ExpReward;
};

USTRUCT(BlueprintType)
struct FEnemySpawnConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FName EnemyTypeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FTransform Transform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bUseBehaviour = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bAutoRespawn = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyDeath, FName, EnemyID);

UCLASS()
class FEEDTHEFISH_API AEnemyActor : public AActor
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;

public:
    // 视觉
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
        UPaperSpriteComponent* SpriteComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
        UPaperFlipbookComponent* FlipbookComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flipbook")
        TMap<FName, UPaperFlipbook*> FlipbookMap;

    // 属性
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
        float MaxHP;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
        float CurrentHP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
        float AttackSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
        float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
        float BaseDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
        float Defense;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
        FSkillData SkillData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
        int32 ExpReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
        FName EnemyID;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
        FGuid UniqueEnemyID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        UDataTable* EnemyDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        UDataTable* SkillDataTable;

    // 碰撞/血条
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
        UBoxComponent* HurtBox;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
        UWidgetComponent* WidgetComponent;

    // 战斗组件
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
        UEnemyCombatComponent* CombatComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
        UEnemyPatrolComponent* PatrolComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior")
        UEnemyBehaviorBase* BehaviorInstance;
public:

    AEnemyActor();
    // 控制
    UFUNCTION(BlueprintCallable)
        void InitializeEnemy(FName InEnemyID, bool bUseBehavior);

    UFUNCTION(BlueprintCallable)
        void ReceiveDamage(float DamageAmount, AActor* DamageCauser);

    UFUNCTION()
        void Die();

    UFUNCTION()
        void OnDeathAnimationFinished();

    UFUNCTION(BlueprintCallable)
        bool IsAlive() const { return !bIsDead && CurrentHP > 0.f; }

    UFUNCTION(BlueprintCallable)
        void PlayFlipbookByName(FName FlipbookKey, float PlayRate = 1.0f, bool bLoop = true);

public:
    UPROPERTY(BlueprintAssignable, Category = "Events")
        FOnEnemyDeath OnEnemyDeath;

private:
    bool bIsDead = false;
    APlayerCharacter* TargetPlayer = nullptr;

    void InitTargetPlayer();

    FTimerHandle DeathTimerHandle;
};
