// InventoryManager.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DropActor.h"
#include "PaperSprite.h"
#include "InventoryManager.generated.h"

USTRUCT(BlueprintType)
struct FInventoryItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FName ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        EDropItemType ItemType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FText Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        EMaterialGrade MaterialGrade = EMaterialGrade::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        UTexture2D* Texture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 Quantity = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FText Description;
};

USTRUCT(BlueprintType)
struct FInventoryEquipmentItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 UniqueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FName EquipmentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        EMaterialGrade MaterialGrade;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        EEquipmentSlot Slot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FText EquipmentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        UTexture2D* Icon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float StrengthenExp = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventorySimpleEvent);

UCLASS()
class FEEDTHEFISH_API UInventoryManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DropSystem")
        UDataTable* DropTable;
public:
    // 材料添加
    UFUNCTION(BlueprintCallable, Category = "Inventory")
        void AddItem(FName ItemID, EDropItemType ItemType, FText Name, EMaterialGrade Grade, UTexture2D* Texture, FText Description);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
        bool AddItemFromDropTable(FName ItemID, EMaterialGrade Grade, int32 Quantity = 1);

    // 装备添加
    UFUNCTION(BlueprintCallable, Category = "Inventory")
        void AddEquipment(FName EquipmentID, EMaterialGrade Grade, EEquipmentSlot Slot, UTexture2D* Texture, const FText& EquipmentName, float StrengthenExp = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
        void AddEquipmentFromInstance(const FInventoryEquipmentItem& EquipmentItem);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
        int32 GetItemQuantityByID(FName ItemID) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
        TArray<FInventoryItem> GetAllItems() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
        TArray<FInventoryEquipmentItem> GetAllEquipments() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
        bool ConsumeItem(FName ItemID, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
        bool RemoveEquipmentByUniqueID(int32 UniqueID);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
        bool ModifyEquipmentStrengthenExp(int32 EquipmentUniqueID, float AddExp);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
        bool FindEquipmentByUniqueID(int32 EquipmentUniqueID, FInventoryEquipmentItem& OutEquipment) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
        TArray<FInventoryItem> GetItemsByType(EDropItemType ItemType) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
        bool FindItemByID(FName ItemID, FInventoryItem& OutItem) const;

public:
    UPROPERTY(BlueprintAssignable, Category = "Inventory Events")
        FOnInventorySimpleEvent OnInventoryUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Inventory Events")
        FOnInventorySimpleEvent OnEquipmentUpdated;

private:
    // 材料背包（可堆叠）
    UPROPERTY()
        TMap<FName, FInventoryItem> Inventory;

    // 装备背包（独立实例）
    UPROPERTY()
        TArray<FInventoryEquipmentItem> EquipmentInventory;

    // 生成装备用的自增唯一ID
    UPROPERTY()
        int32 NextEquipmentUniqueID;
};
