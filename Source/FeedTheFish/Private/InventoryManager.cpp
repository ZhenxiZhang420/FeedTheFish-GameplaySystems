/*
 * InventoryManager.cpp
 *
 * Runtime inventory subsystem for managing materials and equipment instances.
 * Responsibilities:
 * - Add and consume material items.
 * - Store equipment as unique item instances.
 * - Provide item data for inventory UI and gameplay systems.
 * - Broadcast inventory updates to refresh UI.
 */

#include "InventoryManager.h"
#include "DropActor.h"
#include "PaperSprite.h"
#include "EquipmentManager.h"

void UInventoryManager::AddItem(FName ItemID, EDropItemType ItemType, FText Name, EMaterialGrade Grade, UTexture2D* Texture, FText Description)
{
    FName Key = ItemID;

    if (ItemType == EDropItemType::MainMaterial || ItemType == EDropItemType::Equipment)
    {
        FString GradeSuffix = UEnum::GetValueAsString(Grade).Replace(TEXT("EMaterialGrade::"), TEXT(""));
        FString CombinedKey = ItemID.ToString() + TEXT("_") + GradeSuffix;
        Key = FName(*CombinedKey);
    }

    FInventoryItem& Item = Inventory.FindOrAdd(Key);

    if (Item.Quantity == 0)
    {
        Item.ItemID = Key;  // Store the grade-specific item ID
        Item.ItemType = ItemType;
        Item.Name = Name;
        Item.MaterialGrade = Grade;
        Item.Texture = Texture;
        Item.Description = Description;
    }

    Item.Quantity += 1;

    FString TypeStr = UEnum::GetValueAsString(ItemType).Replace(TEXT("EDropItemType::"), TEXT(""));

    UE_LOG(LogTemp, Log, TEXT("获得物品：%s（类型：%s），当前数量：%d"),
        *Item.ItemID.ToString(), *TypeStr, Item.Quantity);  // Log the actual stored item ID

    OnInventoryUpdated.Broadcast();
}

bool UInventoryManager::AddItemFromDropTable(FName ItemID, EMaterialGrade Grade, int32 Quantity)
{
    if (!DropTable || Quantity <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("DropTable为空或数量非法"));
        return false;
    }

    TArray<FEnemyDropConfig*> AllEnemyConfigs;
    DropTable->GetAllRows(TEXT("DropSearch"), AllEnemyConfigs);

    for (const FEnemyDropConfig* EnemyConfig : AllEnemyConfigs)
    {
        if (!EnemyConfig) continue;

        for (const FDropItemConfig& Item : EnemyConfig->DropItems)
        {
            if (Item.ItemID == ItemID)
            {
                // Retrieve the sprite, description, type, and display name for this item ID
                UTexture2D* Texture = Item.Sprite ? Item.Sprite->GetBakedTexture() : nullptr;
                FText Description = Item.Description;
                EDropItemType Type = Item.ItemType;
                FText Name = Item.ItemName;

                for (int32 i = 0; i < Quantity; ++i)
                {
                    AddItem(ItemID, Type, Name, Grade, Texture, Description);
                }

                return true;
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("在DropTable中未找到ID=%s对应的条目"), *ItemID.ToString());
    return false;
}



void UInventoryManager::AddEquipment(FName EquipmentID, EMaterialGrade Grade, EEquipmentSlot Slot, UTexture2D* Texture, const FText& EquipmentName, float StrengthenExp)
{
    FInventoryEquipmentItem NewEquip;
    NewEquip.UniqueID = FDateTime::UtcNow().ToUnixTimestamp() * 1000 + FMath::RandRange(0, 999);
    NewEquip.EquipmentID = EquipmentID;
    NewEquip.MaterialGrade = Grade;
    NewEquip.Slot = Slot;
    NewEquip.Icon = Texture;
    NewEquip.EquipmentName = EquipmentName;
    NewEquip.StrengthenExp = StrengthenExp;

    EquipmentInventory.Add(NewEquip);
    OnEquipmentUpdated.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("新增装备实例：%s（名称：%s，品质：%s，槽位：%d，UniqueID：%d）"),
        *EquipmentID.ToString(),
        *EquipmentName.ToString(),
        *UEnum::GetValueAsString(Grade),
        (int32)Slot,
        NewEquip.UniqueID);
}


void UInventoryManager::AddEquipmentFromInstance(const FInventoryEquipmentItem& EquipmentItem)
{
    EquipmentInventory.Add(EquipmentItem);
    OnEquipmentUpdated.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("回收到背包的装备：%s（UniqueID：%d，强化经验：%.1f）"),
        *EquipmentItem.EquipmentID.ToString(),
        EquipmentItem.UniqueID,
        EquipmentItem.StrengthenExp);
}

int32 UInventoryManager::GetItemQuantityByID(FName ItemID) const
{
    if (const FInventoryItem* Item = Inventory.Find(ItemID))
    {
        return Item->Quantity;
    }
    return 0;
}

TArray<FInventoryItem> UInventoryManager::GetAllItems() const
{
    TArray<FInventoryItem> Items;
    Inventory.GenerateValueArray(Items);
    return Items;
}

TArray<FInventoryEquipmentItem> UInventoryManager::GetAllEquipments() const
{
    TArray<FInventoryEquipmentItem> Result = EquipmentInventory;

    // Sort by material grade from highest to lowest
    Result.Sort([](const FInventoryEquipmentItem& A, const FInventoryEquipmentItem& B)
        {
            return static_cast<int32>(A.MaterialGrade) > static_cast<int32>(B.MaterialGrade);
        });

    return Result;
}

bool UInventoryManager::ConsumeItem(FName ItemID, int32 Count)
{
    if (Count <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ConsumeItem失败：数量必须大于0"));
        return false;
    }

    if (FInventoryItem* Item = Inventory.Find(ItemID))
    {
        if (Item->Quantity < Count)
        {
            UE_LOG(LogTemp, Warning, TEXT("ConsumeItem失败：物品数量不足"));
            return false;
        }

        Item->Quantity -= Count;

        if (Item->Quantity == 0)
        {
            Inventory.Remove(ItemID);
        }

        UE_LOG(LogTemp, Log, TEXT("消耗物品：%s × %d"), *ItemID.ToString(), Count);
        OnInventoryUpdated.Broadcast();
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("ConsumeItem失败：找不到物品 %s"), *ItemID.ToString());
    return false;
}

bool UInventoryManager::RemoveEquipmentByUniqueID(int32 UniqueID)
{
    for (int32 i = 0; i < EquipmentInventory.Num(); ++i)
    {
        if (EquipmentInventory[i].UniqueID == UniqueID)
        {
            EquipmentInventory.RemoveAt(i);

            UE_LOG(LogTemp, Log, TEXT("移除装备，UniqueID：%d"), UniqueID);
            OnEquipmentUpdated.Broadcast();
            return true;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("移除装备失败，找不到UniqueID：%d"), UniqueID);
    return false;
}

bool UInventoryManager::ModifyEquipmentStrengthenExp(int32 EquipmentUniqueID, float AddExp)
{
    for (FInventoryEquipmentItem& Equip : EquipmentInventory)
    {
        if (Equip.UniqueID == EquipmentUniqueID)
        {
            Equip.StrengthenExp += AddExp;
            UE_LOG(LogTemp, Log, TEXT("修改装备 UniqueID:%d 的强化经验，增加了 %f，目前总经验：%f"),
                EquipmentUniqueID, AddExp, Equip.StrengthenExp);
            return true;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("ModifyEquipmentStrengthenExp失败，找不到UniqueID:%d"), EquipmentUniqueID);
    return false;
}

bool UInventoryManager::FindEquipmentByUniqueID(int32 EquipmentUniqueID, FInventoryEquipmentItem& OutEquipment) const
{
    for (const FInventoryEquipmentItem& Equip : EquipmentInventory)
    {
        if (Equip.UniqueID == EquipmentUniqueID)
        {
            OutEquipment = Equip;
            return true;
        }
    }
    return false;
}

TArray<FInventoryItem> UInventoryManager::GetItemsByType(EDropItemType ItemType) const
{
    TArray<FInventoryItem> Result;

    for (const TPair<FName, FInventoryItem>& Pair : Inventory)
    {
        if (Pair.Value.ItemType == ItemType)
        {
            Result.Add(Pair.Value);
        }
    }

    return Result;
}

bool UInventoryManager::FindItemByID(FName ItemID, FInventoryItem& OutItem) const
{
    if (const FInventoryItem* FoundItem = Inventory.Find(ItemID))
    {
        OutItem = *FoundItem;
        return true;
    }

    return false;
}


