#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterAttributeComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttributeChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExperienceChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLevelUpSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChangedSignature, float, NewHealthRatio);


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FEEDTHEFISH_API UCharacterAttributeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCharacterAttributeComponent();

protected:
    virtual void BeginPlay() override;

public:
    // ===== 基础属性 =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
        float Attack = 20.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
        float Defense = 5.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
        float MaxHP = 100.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Stats")
        float CurrentHP = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
        float MoveSpeed = 600.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
        float AttackSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
        float LifeSteal = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
        float DodgeRate = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
        float CritRate = 0.f;

    // ===== 等级系统 =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
        int32 Level = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
        int32 MaxLevel = 99;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
        int32 CurrentExp = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
        int32 ExpToNextLevel = 50;

    // ===== 装备加成属性 =====
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "EquipmentBonus")
        float BonusAttack = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "EquipmentBonus")
        float BonusDefense = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "EquipmentBonus")
        float BonusMaxHP = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "EquipmentBonus")
        float BonusMoveSpeed = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "EquipmentBonus")
        float BonusLifeSteal = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "EquipmentBonus")
        float BonusAttackSpeed = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "EquipmentBonus")
        float BonusDodgeRate = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "EquipmentBonus")
        float BonusCritRate = 0.f;

    // ===== 属性函数 =====
    UFUNCTION(BlueprintCallable)
        bool IsAlive() const;

    UFUNCTION(BlueprintCallable)
        void ReceiveDamage(float Amount);

    UFUNCTION(BlueprintCallable)
        void AddExperience(int32 Exp);

    UFUNCTION(BlueprintCallable)
        void LevelUp();

    UFUNCTION(BlueprintCallable)
        void GetAllAttributeValues(float& OutAttack, float& OutDefense, float& OutMaxHP, float& OutMoveSpeed, float& OutAttackSpeed, float& OutLifeSteal, float& OutDodgeRate, float& OutCritRate) const;

    UFUNCTION(BlueprintCallable, Category = "CharacterStats")
        float GetTotalAttack() const;

    UFUNCTION(BlueprintCallable, Category = "CharacterStats")
        float GetTotalDefense() const;

    UFUNCTION(BlueprintCallable, Category = "CharacterStats")
        float GetTotalMaxHP() const;

    UFUNCTION(BlueprintCallable, Category = "CharacterStats")
        float GetTotalMoveSpeed() const;

    UFUNCTION(BlueprintCallable, Category = "CharacterStats")
        float GetTotalLifeSteal() const;

    UFUNCTION(BlueprintCallable, Category = "CharacterStats")
        float GetTotalAttackSpeed() const;

    UFUNCTION(BlueprintCallable, Category = "CharacterStats")
        float GetTotalDodgeRate() const;

    UFUNCTION(BlueprintCallable, Category = "CharacterStats")
        float GetTotalCritRate() const;

    UFUNCTION(BlueprintCallable)
        void RestoreFullHealth(bool ConsumeSeed);

    // ===== 事件 =====
    UPROPERTY(BlueprintAssignable, Category = "Events")
        FOnCharacterDeath OnDeath;

    UPROPERTY(BlueprintAssignable, Category = "Events")
        FOnAttributeChanged OnAttributeChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
        FOnExperienceChanged OnExperienceChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
        FOnLevelUpSignature OnLevelUp;

    UPROPERTY(BlueprintAssignable, Category = "Events")
        FOnHealthChangedSignature OnHealthChanged;
};
