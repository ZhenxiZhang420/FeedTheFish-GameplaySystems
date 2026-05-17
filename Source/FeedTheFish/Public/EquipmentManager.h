// EquipmentManager.h
#pragma once

#include "CoreMinimal.h"
#include "CraftingSystem.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InventoryManager.h"
#include "EquipmentManager.generated.h"

USTRUCT(BlueprintType)
struct FEquipmentStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float Attack = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float Defense = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float MaxHP = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float MoveSpeed = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float LifeSteal = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float AttackSpeed = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float DodgeRate = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float CritRate = 0.f;
};

USTRUCT(BlueprintType)
struct FEquipmentData : public FTableRowBase
{
    GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FName EquipmentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        UTexture2D* Icon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        EEquipmentSlot Slot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float StrengthenExp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FName SetID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TMap<EMaterialGrade, FEquipmentStats> Stats;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipmentEquipped, const FInventoryEquipmentItem&, EquippedItem);


UCLASS()
class FEEDTHEFISH_API UEquipmentManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
        UDataTable* EquipmentDataTable;

    // 穿戴装备（根据装备的 UniqueID）
    UFUNCTION(BlueprintCallable, Category = "Equipment")
        void Equip(int32 EquipmentUniqueID);

    // 替换装备
    UFUNCTION(BlueprintCallable, Category = "Equipment")
        void ReplaceEquippedItem(int32 NewEquipmentUniqueID);

    UFUNCTION(BlueprintCallable, Category = "Equipment")
        void Unequip(EEquipmentSlot Slot);

    // 强化装备
    UFUNCTION(BlueprintCallable, Category = "Equipment")
        bool StrengthenEquipment(int32 TargetUniqueID, int32 ConsumedUniqueID, float& OutGainExp);

    // 根据装备实例获取它的加成属性
    UFUNCTION(BlueprintCallable, Category = "Equipment")
        FEquipmentStats GetTotalStatsByEquipmentInstance(const FInventoryEquipmentItem& EquipmentItem) const;

    UFUNCTION(BlueprintCallable, Category = "Equipment")
        bool GetEquippedItem(EEquipmentSlot Slot, FInventoryEquipmentItem& OutItem) const;

public:
    UPROPERTY(BlueprintAssignable, Category = "Equipment Events")
        FOnEquipmentEquipped OnEquipmentEquipped;

private:
    UPROPERTY()
        TMap<EEquipmentSlot, FInventoryEquipmentItem> EquippedItems;

    UInventoryManager* GetInventory() const;
    void ApplyStats(const FEquipmentStats& Stats);
    void RemoveStats(const FEquipmentStats& Stats);

    const FEquipmentData* GetEquipmentDataByID(FName EquipmentID) const;
    EMaterialGrade ParseMaterialGradeFromID(const FName& MaterialID) const;
};
