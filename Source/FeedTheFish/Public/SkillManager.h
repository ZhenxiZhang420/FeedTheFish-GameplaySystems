#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Components/ActorComponent.h"
#include "SkillManager.generated.h"

USTRUCT(BlueprintType)
struct FSkillLevelData
{
    GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float Cooldown = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float AttackSpeedBonus = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float BuffDuration = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float FirstHitMultiplier = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float SecondHitMultiplier = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float ThirdHitMultiplier = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 RequiredBlueprintCount = 0;
};

USTRUCT(BlueprintType)
struct FSkillData : public FTableRowBase
{
    GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FName SkillID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        UTexture2D* SkillIcon = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FName RequiredBlueprintItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TArray<FSkillLevelData> LevelUpData;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSkillChanged, FName, SkillID, int32, NewLevel, float, NewCooldown);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FEEDTHEFISH_API USkillManager : public UActorComponent
{
    GENERATED_BODY()

public:
    USkillManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        UDataTable* SkillDataTable;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
        TMap<FName, int32> LearnedSkills;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
        TMap<FName, float> SkillCooldowns;

    UFUNCTION(BlueprintCallable, Category = "Skill")
        bool LearnSkill(FName SkillID);

    UFUNCTION(BlueprintCallable, Category = "Skill")
        bool UpgradeSkill(FName SkillID);

    UFUNCTION(BlueprintCallable, Category = "Skill")
        bool TryUseSkill(FName SkillID, AActor* Caster, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Skill")
        int32 GetSkillLevel(FName SkillID) const;

    UFUNCTION(BlueprintCallable, Category = "Skill")
        float GetCooldown(FName SkillID) const;

    UFUNCTION(BlueprintCallable, Category = "Skill")
        TArray<FSkillData> GetLearnedSkillData() const;

    bool ApplyBerserkEffect(AActor* Caster, const FSkillLevelData& LevelData);

public:
    UPROPERTY(BlueprintAssignable, Category = "Skill Events")
        FOnSkillChanged OnSkillLearned;

    UPROPERTY(BlueprintAssignable, Category = "Skill Events")
        FOnSkillChanged OnSkillUpgraded;

    // 在类中添加成员变量
    FTimerHandle SmashRecoveryHandle;

    UPROPERTY()
        TArray<FTimerHandle> SmashAttackHandles;

    bool ApplySmashAttack(AActor* Caster, AActor* Target, const FSkillLevelData& LevelData, const FSkillData& SkillConfig);
    void PerformSmashHit(AActor* Caster, AActor* Target, const FSkillLevelData& LevelData, const FSkillData& SkillConfig, int32 HitIndex);

};
