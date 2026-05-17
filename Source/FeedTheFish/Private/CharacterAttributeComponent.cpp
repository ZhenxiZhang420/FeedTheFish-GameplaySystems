#include "CharacterAttributeComponent.h"
#include "PlayerCharacter.h"
#include "InventoryManager.h"
#include "Engine/GameInstance.h"

UCharacterAttributeComponent::UCharacterAttributeComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterAttributeComponent::BeginPlay()
{
    Super::BeginPlay();

    CurrentHP = GetTotalMaxHP();
}

// ===== 你原来的功能 =====

bool UCharacterAttributeComponent::IsAlive() const
{
    return CurrentHP > 0.f;
}

void UCharacterAttributeComponent::ReceiveDamage(float Amount)
{
    if (CurrentHP <= 0.f) return;

    CurrentHP -= Amount;
    CurrentHP = FMath::Clamp(CurrentHP, 0.f, MaxHP); // 避免负数

    float Ratio = (MaxHP > 0.f) ? (CurrentHP / MaxHP) : 0.f;

    // ✅ 广播血量变更事件
    OnHealthChanged.Broadcast(Ratio);

    if (CurrentHP <= 0.f)
    {
        OnDeath.Broadcast(); // ✅ 死亡事件
    }
}


void UCharacterAttributeComponent::AddExperience(int32 Exp)
{
    CurrentExp += Exp;

    while (CurrentExp >= ExpToNextLevel && Level < MaxLevel)
    {
        CurrentExp -= ExpToNextLevel;
        LevelUp();
    }

    // ✅ 经验变化后触发事件
    OnExperienceChanged.Broadcast();
}

void UCharacterAttributeComponent::LevelUp()
{
    Level++;

    // 属性成长（可以自己定制成长公式）
    Attack += 2.f;
    MaxHP += 10.f;
    AttackSpeed += 0.05f;
    Defense += 1.f;
    LifeSteal += 0.01f;
    DodgeRate += 0.01f;
    CritRate += 0.01f;

    CurrentHP = GetTotalMaxHP(); // 升级回血，注意用总血量！

    ExpToNextLevel += 50;

    OnAttributeChanged.Broadcast();
    OnLevelUp.Broadcast();
}

void UCharacterAttributeComponent::GetAllAttributeValues(float& OutAttack, float& OutDefense, float& OutMaxHP, float& OutMoveSpeed, float& OutAttackSpeed, float& OutLifeSteal, float& OutDodgeRate, float& OutCritRate) const
{
    OutAttack = GetTotalAttack();
    OutDefense = GetTotalDefense();
    OutMaxHP = GetTotalMaxHP();
    OutMoveSpeed = GetTotalMoveSpeed();
    OutAttackSpeed = GetTotalAttackSpeed();
    OutLifeSteal = GetTotalLifeSteal();
    OutDodgeRate = GetTotalDodgeRate();
    OutCritRate = GetTotalCritRate();
}

// ===== 新增：总属性计算（基础+装备加成） =====

float UCharacterAttributeComponent::GetTotalAttack() const
{
    return Attack + BonusAttack;
}

float UCharacterAttributeComponent::GetTotalDefense() const
{
    return Defense + BonusDefense;
}

float UCharacterAttributeComponent::GetTotalMaxHP() const
{
    return MaxHP + BonusMaxHP;
}

float UCharacterAttributeComponent::GetTotalMoveSpeed() const
{
    return MoveSpeed + BonusMoveSpeed;
}

float UCharacterAttributeComponent::GetTotalLifeSteal() const
{
    return LifeSteal + BonusLifeSteal;
}

float UCharacterAttributeComponent::GetTotalAttackSpeed() const
{
    return AttackSpeed + BonusAttackSpeed;
}

float UCharacterAttributeComponent::GetTotalDodgeRate() const
{
    return DodgeRate + BonusDodgeRate;
}

float UCharacterAttributeComponent::GetTotalCritRate() const
{
    return CritRate + BonusCritRate;
}

void UCharacterAttributeComponent::RestoreFullHealth(bool ConsumeSeed)
{
    if (ConsumeSeed)
    {
        FName SeedItemID = "Healing_Bud";

        if (UInventoryManager* InventoryManager = GetWorld()->GetGameInstance()->GetSubsystem<UInventoryManager>())
        {
            bool bSuccess = InventoryManager->ConsumeItem(SeedItemID, 1);
            if (!bSuccess)
            {
                UE_LOG(LogTemp, Warning, TEXT("回血失败：没有足够的种子 %s"), *SeedItemID.ToString());
                return;
            }
        }
    }

    // ✅ 回满血 & 广播事件
    CurrentHP = MaxHP;
    float Ratio = (MaxHP > 0.f) ? (CurrentHP / MaxHP) : 0.f;
    OnHealthChanged.Broadcast(Ratio);

}

