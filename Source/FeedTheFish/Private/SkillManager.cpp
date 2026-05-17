/*
 * SkillManager.cpp
 *
 * Player skill component for learning, upgrading, cooldown tracking, and skill execution.
 * Responsibilities:
 * - Learn and upgrade skills using required blueprint items.
 * - Track learned skill levels and cooldown timers.
 * - Apply skill effects during combat.
 * - Support temporary stat-based skill effects such as attack speed boosts.
 */

#include "SkillManager.h"
#include "CombatSystem.h"
#include "Engine/DataTable.h"
#include "PlayerCharacter.h"
#include "PlayerCombatComponent.h"
#include "InventoryManager.h"
#include "CharacterAttributeComponent.h"
#include "EnemyActor.h"

USkillManager::USkillManager()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void USkillManager::BeginPlay()
{
    Super::BeginPlay();
}

void USkillManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    for (auto& Pair : SkillCooldowns)
    {
        if (Pair.Value > 0.f)
        {
            Pair.Value -= DeltaTime;
        }
    }
}

bool USkillManager::LearnSkill(FName SkillID)
{
    if (LearnedSkills.Contains(SkillID) || !SkillDataTable) return false;

    const FSkillData* Skill = SkillDataTable->FindRow<FSkillData>(SkillID, TEXT("LearnSkill"));
    if (!Skill || Skill->LevelUpData.Num() == 0) return false;

    int32 RequiredCount = Skill->LevelUpData[0].RequiredBlueprintCount;
    FName BlueprintID = Skill->RequiredBlueprintItemID;

    if (UInventoryManager* Inventory = GetWorld()->GetGameInstance()->GetSubsystem<UInventoryManager>())
    {
        if (!Inventory->ConsumeItem(BlueprintID, RequiredCount)) return false;
    }
    else return false;

    LearnedSkills.Add(SkillID, 1);
    float NewCooldown = Skill->LevelUpData[0].Cooldown;
    OnSkillLearned.Broadcast(SkillID, 1, NewCooldown);

    UE_LOG(LogTemp, Log, TEXT("成功学习技能：%s（等级 1）"), *Skill->Name);
    return true;
}

bool USkillManager::UpgradeSkill(FName SkillID)
{
    if (!LearnedSkills.Contains(SkillID) || !SkillDataTable) return false;

    const FSkillData* Skill = SkillDataTable->FindRow<FSkillData>(SkillID, TEXT("SkillUpgrade"));
    if (!Skill) return false;

    int32 CurrentLevel = LearnedSkills[SkillID];
    if (Skill->LevelUpData.Num() <= CurrentLevel) return false;

    int32 RequiredCount = Skill->LevelUpData[CurrentLevel].RequiredBlueprintCount;
    FName BlueprintID = Skill->RequiredBlueprintItemID;

    if (UInventoryManager* Inventory = GetWorld()->GetGameInstance()->GetSubsystem<UInventoryManager>())
    {
        if (!Inventory->ConsumeItem(BlueprintID, RequiredCount)) return false;
    }
    else return false;

    LearnedSkills[SkillID]++;
    float NewCooldown = Skill->LevelUpData[CurrentLevel].Cooldown;
    OnSkillUpgraded.Broadcast(SkillID, LearnedSkills[SkillID], NewCooldown);

    UE_LOG(LogTemp, Log, TEXT("技能 %s 升级至等级 %d"), *Skill->Name, LearnedSkills[SkillID]);
    return true;
}

int32 USkillManager::GetSkillLevel(FName SkillID) const
{
    const int32* Level = LearnedSkills.Find(SkillID);
    return Level ? *Level : 0;
}

float USkillManager::GetCooldown(FName SkillID) const
{
    const float* CD = SkillCooldowns.Find(SkillID);
    return CD ? *CD : 0.f;
}

bool USkillManager::TryUseSkill(FName SkillID, AActor* Caster, AActor* Target)
{
    if (!SkillDataTable || !LearnedSkills.Contains(SkillID) || !Caster || !Target)
    {
        return false;
    }

    const FSkillData* Skill = SkillDataTable->FindRow<FSkillData>(SkillID, TEXT("Skill Context"));
    if (!Skill) return false;

    int32 Level = LearnedSkills[SkillID];
    if (Level < 1 || Level > Skill->LevelUpData.Num()) return false;

    const FSkillLevelData& LevelData = Skill->LevelUpData[Level - 1];

    UE_LOG(LogTemp, Warning, TEXT("TryUseSkill called: SkillID = %s"), *SkillID.ToString());
    if (SkillID == "Berserk")
    {
        return ApplyBerserkEffect(Caster, LevelData);
    }

    if (SkillID == "Smash")
    {
        return ApplySmashAttack(Caster, Target, LevelData, *Skill);
    }

    return false;
}

TArray<FSkillData> USkillManager::GetLearnedSkillData() const
{
    TArray<FSkillData> LearnedData;
    if (!SkillDataTable) return LearnedData;

    for (const auto& Pair : LearnedSkills)
    {
        const FSkillData* Skill = SkillDataTable->FindRow<FSkillData>(Pair.Key, TEXT(""));
        if (Skill) LearnedData.Add(*Skill);
    }
    return LearnedData;
}

bool USkillManager::ApplyBerserkEffect(AActor* Caster, const FSkillLevelData& LevelData)
{
    if (!Caster) return false;

    if (UCharacterAttributeComponent* Attr = Caster->FindComponentByClass<UCharacterAttributeComponent>())
    {
        float OriginalSpeed = Attr->AttackSpeed;
        float Multiplier = LevelData.AttackSpeedBonus > 0.f ? LevelData.AttackSpeedBonus : 1.3f;
        float Duration = LevelData.BuffDuration > 0.f ? LevelData.BuffDuration : 3.0f;

        Attr->AttackSpeed *= Multiplier;

        if (UPlayerCombatComponent* Combat = Caster->FindComponentByClass<UPlayerCombatComponent>())
        {
            Combat->RestartAutoAttack();
        }

        FTimerHandle RevertHandle;
        Caster->GetWorldTimerManager().SetTimer(RevertHandle, [=]()
            {
                if (Attr && Attr->IsAlive())
                {
                    Attr->AttackSpeed = OriginalSpeed;

                    if (UPlayerCombatComponent* Combat = Caster->FindComponentByClass<UPlayerCombatComponent>())
                    {
                        Combat->RestartAutoAttack();
                    }
                }
            }, Duration, false);

        return true;
    }

    return false;
}

bool USkillManager::ApplySmashAttack(AActor* Caster, AActor* Target, const FSkillLevelData& LevelData, const FSkillData& SkillConfig)
{
    if (!Caster || !Target) return false;

    UE_LOG(LogTemp, Warning, TEXT("ApplySmashAttack"));
    SmashAttackHandles.Empty();

    float StartDelay = 0.2f;
    float AnimationDuration = 0.8f;
    int32 TotalHits = 3;


    float TotalDuration = StartDelay * 2 + AnimationDuration * TotalHits;

    UPlayerCombatComponent* PlayerCombat = Caster->FindComponentByClass<UPlayerCombatComponent>();
    if (PlayerCombat)
    {
        PlayerCombat->LockSkillInput(TotalDuration);
    }

    if (APlayerCharacter* Player = Cast<APlayerCharacter>(Caster))
    {
        if (Player->FlipbookComponent && Player->AttackFlipbook)
        {
            Player->FlipbookComponent->SetFlipbook(Player->AttackFlipbook);
            Player->FlipbookComponent->SetPlayRate(1.0f);
            Player->FlipbookComponent->Play();
        }
    }

    for (int32 i = 0; i < TotalHits; ++i)
    {
        int32 HitIndex = i + 1;
        float Delay = StartDelay + AnimationDuration * HitIndex;

        FTimerHandle Handle;
        GetWorld()->GetTimerManager().SetTimer(
            Handle,
            [this, Caster, Target, LevelData, SkillConfig, HitIndex]()
            {
                PerformSmashHit(Caster, Target, LevelData, SkillConfig, HitIndex);
            },
            Delay,
            false
        );

        SmashAttackHandles.Add(Handle);
    }

    GetWorld()->GetTimerManager().SetTimer(SmashRecoveryHandle,
        [this]()
        {
            UE_LOG(LogTemp, Log, TEXT("SmashAttack 三连击完成"));
        },
        TotalDuration,
        false
    );

    UE_LOG(LogTemp, Log, TEXT("SmashAttack 启动三连击，总时长 %.2f 秒"), TotalDuration);
    return true;
}



void USkillManager::PerformSmashHit(AActor* Caster, AActor* Target, const FSkillLevelData& LevelData, const FSkillData& SkillConfig, int32 HitIndex)
{
    if (!Caster || !Target) return;

    float Multiplier = 1.0f;
    switch (HitIndex)
    {
        case 1: Multiplier = LevelData.FirstHitMultiplier; break;
        case 2: Multiplier = LevelData.SecondHitMultiplier; break;
        case 3: Multiplier = LevelData.ThirdHitMultiplier; break;
        default: break;
    }
    if (Multiplier <= 0.f) Multiplier = 1.0f;

    // Use the character's calculated total attack value
    UCharacterAttributeComponent* Attr = Caster->FindComponentByClass<UCharacterAttributeComponent>();
    float BaseDamage = Attr ? Attr->GetTotalAttack() : 1.0f;

    float FinalDamage = BaseDamage * Multiplier;

    UE_LOG(LogTemp, Log, TEXT("Smash 第 %d 击，造成 %.2f 倍伤害，实际伤害 %.1f"), HitIndex, Multiplier, FinalDamage);

    UCombatSystem::ApplyDamage(Caster, Target, FinalDamage, SkillConfig);
}



