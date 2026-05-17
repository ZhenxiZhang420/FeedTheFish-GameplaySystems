/*
 * EquipmentManager.cpp
 *
 * Equipment subsystem for equipping, replacing, enhancing, and unequipping gear.
 * Responsibilities:
 * - Move equipment between inventory and equipped slots.
 * - Apply and remove equipment stat bonuses.
 * - Strengthen equipment using other equipment as upgrade materials.
 * - Broadcast equipment updates to refresh UI and character attributes.
 */

#include "EquipmentManager.h"
#include "CharacterAttributeComponent.h"
#include "Kismet/KismetMathLibrary.h"

UInventoryManager* UEquipmentManager::GetInventory() const
{
    if (GetGameInstance())
    {
        return GetGameInstance()->GetSubsystem<UInventoryManager>();
    }
    return nullptr;
}

void UEquipmentManager::Equip(int32 EquipmentUniqueID)
{
    UInventoryManager* Inventory = GetInventory();
    if (!Inventory || !EquipmentDataTable) return;

    const TArray<FInventoryEquipmentItem>& AllEquipments = Inventory->GetAllEquipments();
    const FInventoryEquipmentItem* EquipmentItem = AllEquipments.FindByPredicate([&](const FInventoryEquipmentItem& Item)
        {
            return Item.UniqueID == EquipmentUniqueID;
        });

    if (!EquipmentItem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Equip失败，找不到UniqueID：%d"), EquipmentUniqueID);
        return;
    }

    const FEquipmentData* EquipData = GetEquipmentDataByID(EquipmentItem->EquipmentID);
    if (!EquipData)
    {
        UE_LOG(LogTemp, Warning, TEXT("找不到装备数据：%s"), *EquipmentItem->EquipmentID.ToString());
        return;
    }

    EEquipmentSlot Slot = EquipData->Slot;

    // Replace the currently equipped item if this slot is already occupied
    if (EquippedItems.Contains(Slot))
    {
        int32 OldUniqueID = EquippedItems[Slot].UniqueID; // Use the unique equipment instance ID
        ReplaceEquippedItem(EquipmentUniqueID);
        return;
    }

    // Equip the new item instance
    EquippedItems.Add(Slot, *EquipmentItem); // Store the full equipment instance instead of only the unique ID

    FEquipmentStats Stats = GetTotalStatsByEquipmentInstance(*EquipmentItem);
    ApplyStats(Stats);

    Inventory->RemoveEquipmentByUniqueID(EquipmentUniqueID);

    OnEquipmentEquipped.Broadcast(*EquipmentItem);
    UE_LOG(LogTemp, Log, TEXT("成功穿戴装备，UniqueID：%d，槽位：%d"), EquipmentUniqueID, (int32)Slot);
}


void UEquipmentManager::ReplaceEquippedItem(int32 NewEquipmentUniqueID)
{
    UInventoryManager* Inventory = GetInventory();
    if (!Inventory || !EquipmentDataTable) return;

    const TArray<FInventoryEquipmentItem>& AllEquipments = Inventory->GetAllEquipments();
    const FInventoryEquipmentItem* NewEquipmentItem = AllEquipments.FindByPredicate([&](const FInventoryEquipmentItem& Item)
        {
            return Item.UniqueID == NewEquipmentUniqueID;
        });

    if (!NewEquipmentItem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Replace失败，找不到新装备UniqueID：%d"), NewEquipmentUniqueID);
        return;
    }

    const FEquipmentData* NewData = GetEquipmentDataByID(NewEquipmentItem->EquipmentID);
    if (!NewData)
    {
        UE_LOG(LogTemp, Warning, TEXT("找不到新装备数据"));
        return;
    }

    EEquipmentSlot Slot = NewData->Slot;

    if (EquippedItems.Contains(Slot))
    {
        FInventoryEquipmentItem OldEquipInstance = EquippedItems[Slot];

        FEquipmentStats OldStats = GetTotalStatsByEquipmentInstance(OldEquipInstance);
        RemoveStats(OldStats);

        Inventory->AddEquipmentFromInstance(OldEquipInstance);

        EquippedItems.Remove(Slot);
    }

    EquippedItems.Add(Slot, *NewEquipmentItem);

    FEquipmentStats NewStats = GetTotalStatsByEquipmentInstance(*NewEquipmentItem);
    ApplyStats(NewStats);

    Inventory->RemoveEquipmentByUniqueID(NewEquipmentUniqueID);
    OnEquipmentEquipped.Broadcast(*NewEquipmentItem);
    UE_LOG(LogTemp, Log, TEXT("替换装备成功，UniqueID：%d"), NewEquipmentUniqueID);
}


bool UEquipmentManager::StrengthenEquipment(int32 TargetUniqueID, int32 ConsumedUniqueID, float& OutGainExp)
{
    OutGainExp = 0.0f; // Initialize the output value to avoid unexpected results

    UInventoryManager* Inventory = GetInventory();
    if (!Inventory) return false;

    // Find the target equipment instance to enhance
    FInventoryEquipmentItem TargetItem;
    if (!Inventory->FindEquipmentByUniqueID(TargetUniqueID, TargetItem))
    {
        UE_LOG(LogTemp, Warning, TEXT("强化失败，找不到目标装备 UniqueID：%d"), TargetUniqueID);
        return false;
    }

    // Calculate the current enhancement stage based on accumulated experience
    int32 Stage = FMath::FloorToInt(TargetItem.StrengthenExp / 10.0f);
    float MinExp = 1.0f;
    float MaxExp = 3.0f;

    switch (Stage)
    {
    case 0:
        MinExp = 10.0f;
        MaxExp = 12.0f;
        break;
    case 1:
        MinExp = 4.0f;
        MaxExp = 6.0f;
        break;
    case 2:
        MinExp = 3.0f;
        MaxExp = 5.0f;
        break;
    default:
        MinExp = 1.0f;
        MaxExp = 3.0f;
        break;
    }

    OutGainExp = FMath::FRandRange(MinExp, MaxExp); // Generate and store gained enhancement experience

    // Remove the material equipment instance after it is consumed
    Inventory->RemoveEquipmentByUniqueID(ConsumedUniqueID);

    // Add enhancement experience to the target equipment
    bool bSuccess = Inventory->ModifyEquipmentStrengthenExp(TargetUniqueID, OutGainExp);

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("成功强化装备：UniqueID=%d，增加经验 %.2f，当前阶段 %d"),
            TargetUniqueID, OutGainExp, Stage);
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("强化失败，找不到目标装备 UniqueID"));
        return false;
    }
}



FEquipmentStats UEquipmentManager::GetTotalStatsByEquipmentInstance(const FInventoryEquipmentItem& EquipmentItem) const
{
    FEquipmentStats FinalStats;

    const FEquipmentData* EquipData = GetEquipmentDataByID(EquipmentItem.EquipmentID);
    if (!EquipData)
    {
        return FinalStats;
    }

    const FEquipmentStats* BaseStats = EquipData->Stats.Find(EquipmentItem.MaterialGrade);
    if (BaseStats)
    {
        FinalStats = *BaseStats;

        float StrengthenFactor = EquipmentItem.StrengthenExp / 100.f;

        if (BaseStats->Attack > 0)
        {
            FinalStats.Attack += FMath::RoundToInt(BaseStats->Attack * StrengthenFactor);
        }

        if (BaseStats->Defense > 0)
        {
            FinalStats.Defense += FMath::RoundToInt(BaseStats->Defense * StrengthenFactor);
        }

        if (BaseStats->MaxHP > 0)
        {
            FinalStats.MaxHP += FMath::RoundToInt(BaseStats->MaxHP * StrengthenFactor);
        }
    }

    return FinalStats;
}


void UEquipmentManager::ApplyStats(const FEquipmentStats& Stats)
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    APawn* Pawn = PC->GetPawn();
    if (!Pawn) return;

    UCharacterAttributeComponent* Attr = Pawn->FindComponentByClass<UCharacterAttributeComponent>();
    if (!Attr) return;

    Attr->BonusAttack += Stats.Attack;
    Attr->BonusDefense += Stats.Defense;
    Attr->BonusMaxHP += Stats.MaxHP;
    Attr->BonusMoveSpeed += Stats.MoveSpeed;
    Attr->BonusLifeSteal += Stats.LifeSteal;
    Attr->BonusAttackSpeed += Stats.AttackSpeed;
    Attr->BonusDodgeRate += Stats.DodgeRate;
    Attr->BonusCritRate += Stats.CritRate;

    Attr->OnAttributeChanged.Broadcast();
}

void UEquipmentManager::RemoveStats(const FEquipmentStats& Stats)
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    APawn* Pawn = PC->GetPawn();
    if (!Pawn) return;

    UCharacterAttributeComponent* Attr = Pawn->FindComponentByClass<UCharacterAttributeComponent>();
    if (!Attr) return;

    Attr->BonusAttack -= Stats.Attack;
    Attr->BonusDefense -= Stats.Defense;
    Attr->BonusMaxHP -= Stats.MaxHP;
    Attr->BonusMoveSpeed -= Stats.MoveSpeed;
    Attr->BonusLifeSteal -= Stats.LifeSteal;
    Attr->BonusAttackSpeed -= Stats.AttackSpeed;
    Attr->BonusDodgeRate -= Stats.DodgeRate;
    Attr->BonusCritRate -= Stats.CritRate;

    Attr->OnAttributeChanged.Broadcast();
}

const FEquipmentData* UEquipmentManager::GetEquipmentDataByID(FName EquipmentID) const
{
    if (!EquipmentDataTable) return nullptr;

    static const FString Context = TEXT("GetEquipmentDataByID");
    return EquipmentDataTable->FindRow<FEquipmentData>(EquipmentID, Context);
}

EMaterialGrade UEquipmentManager::ParseMaterialGradeFromID(const FName& MaterialID) const
{
    FString IDStr = MaterialID.ToString().ToLower();

    if (IDStr.EndsWith(TEXT("_poor")))      return EMaterialGrade::Poor;
    if (IDStr.EndsWith(TEXT("_normal")))    return EMaterialGrade::Normal;
    if (IDStr.EndsWith(TEXT("_fine")))      return EMaterialGrade::Fine;
    if (IDStr.EndsWith(TEXT("_excellent"))) return EMaterialGrade::Excellent;
    if (IDStr.EndsWith(TEXT("_perfect")))   return EMaterialGrade::Perfect;

    return EMaterialGrade::Normal;
}

bool UEquipmentManager::GetEquippedItem(EEquipmentSlot Slot, FInventoryEquipmentItem& OutItem) const
{
    if (const FInventoryEquipmentItem* FoundItem = EquippedItems.Find(Slot))
    {
        OutItem = *FoundItem; // Copy the full equipment instance data
        return true;
    }

    return false;
}

void UEquipmentManager::Unequip(EEquipmentSlot Slot)
{
    UInventoryManager* Inventory = GetInventory();
    if (!Inventory || !EquipmentDataTable) return;

    if (!EquippedItems.Contains(Slot))
    {
        UE_LOG(LogTemp, Warning, TEXT("Unequip失败：该槽位没有装备"));
        return;
    }

    FInventoryEquipmentItem EquippedItem = EquippedItems[Slot];

    // Remove the equipped item stat bonuses
    FEquipmentStats Stats = GetTotalStatsByEquipmentInstance(EquippedItem);
    RemoveStats(Stats);

    // Return the equipped item to the inventory
    Inventory->AddEquipmentFromInstance(EquippedItem);

    // Remove the equipped slot record
    EquippedItems.Remove(Slot);

    UE_LOG(LogTemp, Log, TEXT("成功卸下装备，UniqueID：%d，槽位：%d"), EquippedItem.UniqueID, (int32)Slot);
}


