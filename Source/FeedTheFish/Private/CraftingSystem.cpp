// CraftingSystem.cpp
#include "CraftingSystem.h"
#include "InventoryManager.h"
#include "Kismet/GameplayStatics.h"

UInventoryManager* UCraftingSystem::GetInventory() const
{
    if (GetGameInstance())
    {
        return GetGameInstance()->GetSubsystem<UInventoryManager>();
    }
    return nullptr;
}

void UCraftingSystem::CraftEquipment(FName MainMaterialID, FName ResultEquipmentID)
{
    UInventoryManager* Inventory = GetInventory();
    if (!Inventory || !EquipmentRecipeTable)
    {
        return;
    }

    static const FString ContextString(TEXT("CraftEquipment"));
    const FEquipmentCraftingRecipe* Recipe = EquipmentRecipeTable->FindRow<FEquipmentCraftingRecipe>(
        ResultEquipmentID, ContextString);

    if (!Recipe)
    {
        UE_LOG(LogTemp, Warning, TEXT("找不到配方：%s"), *ResultEquipmentID.ToString());
        return;
    }

    // 获取主材料等级
    EMaterialGrade Grade = ParseMaterialGradeFromID(MainMaterialID);

    // 扣除主材料
    Inventory->ConsumeItem(MainMaterialID, 1);

    // 扣除副材料
    for (const TPair<FName, int32>& Pair : Recipe->SubMaterials)
    {
        bool bSuccess = Inventory->ConsumeItem(Pair.Key, Pair.Value);
        if (!bSuccess)
        {
            UE_LOG(LogTemp, Warning, TEXT("扣除副材料失败：%s × %d"), *Pair.Key.ToString(), Pair.Value);
        }
    }

    // 生成一件独立的新装备
    Inventory->AddEquipment(ResultEquipmentID, Grade, Recipe->EquipSlot, Recipe->ResultTexture, Recipe->EquipmentDisplayName, 0.0f);

    FText GradeDisplayName = StaticEnum<EMaterialGrade>()->GetDisplayNameTextByValue((int64)Grade);

    UE_LOG(LogTemp, Log, TEXT("成功合成装备：%s（品质等级：%s）"),
        *ResultEquipmentID.ToString(),
        *GradeDisplayName.ToString());
}

bool UCraftingSystem::HasEnoughMaterials(FName MainMaterialID, FName ResultEquipmentID) const
{
    UInventoryManager* Inventory = GetInventory();
    if (!Inventory || !EquipmentRecipeTable)
    {
        return false;
    }

    static const FString ContextString(TEXT("HasEnoughMaterials"));
    FEquipmentCraftingRecipe* Recipe = EquipmentRecipeTable->FindRow<FEquipmentCraftingRecipe>(
        ResultEquipmentID, ContextString);

    if (!Recipe)
    {
        UE_LOG(LogTemp, Warning, TEXT("找不到配方，目标装备ID：%s"), *ResultEquipmentID.ToString());
        return false;
    }

    if (Inventory->GetItemQuantityByID(MainMaterialID) < 1)
    {
        UE_LOG(LogTemp, Warning, TEXT("主材料不足：%s"), *MainMaterialID.ToString());
        return false;
    }

    for (const TPair<FName, int32>& Pair : Recipe->SubMaterials)
    {
        if (Inventory->GetItemQuantityByID(Pair.Key) < Pair.Value)
        {
            UE_LOG(LogTemp, Warning, TEXT("副材料不足：%s"), *Pair.Key.ToString());
            return false;
        }
    }

    return true;
}

TArray<FEquipmentCraftingRecipe> UCraftingSystem::GetRecipesByMainMaterialBaseID(FName MainMaterialIDWithGrade) const
{
    TArray<FEquipmentCraftingRecipe> Result;

    if (!EquipmentRecipeTable) return Result;

    FString FullID = MainMaterialIDWithGrade.ToString();
    FString BaseID;

    if (!FullID.Split(TEXT("_"), &BaseID, nullptr, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
    {
        BaseID = FullID;
    }

    TArray<FEquipmentCraftingRecipe*> Recipes;
    EquipmentRecipeTable->GetAllRows(TEXT("FindAllEquipmentRecipes"), Recipes);

    for (const FEquipmentCraftingRecipe* Recipe : Recipes)
    {
        if (Recipe && Recipe->MainMaterialID.ToString().StartsWith(BaseID))
        {
            Result.Add(*Recipe);
        }
    }

    return Result;
}

UTexture2D* UCraftingSystem::GetSubMaterialTexture(FName MaterialID) const
{
    if (!EquipmentRecipeTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("EquipmentRecipeTable is not valid."));
        return nullptr;
    }

    TArray<FEquipmentCraftingRecipe*> AllRecipes;
    EquipmentRecipeTable->GetAllRows<FEquipmentCraftingRecipe>(TEXT("LookupSubMaterialTexture"), AllRecipes);

    for (FEquipmentCraftingRecipe* Recipe : AllRecipes)
    {
        if (Recipe && Recipe->SubMaterialTextures.Contains(MaterialID))
        {
            return Recipe->SubMaterialTextures[MaterialID];
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("MaterialID not found in any recipe: %s"), *MaterialID.ToString());
    return nullptr;
}

FText UCraftingSystem::GetMaterialGradeTextFromID(FName MaterialID) const
{
    EMaterialGrade Grade = ParseMaterialGradeFromID(MaterialID);
    return StaticEnum<EMaterialGrade>()->GetDisplayNameTextByValue((int64)Grade);
}

EMaterialGrade UCraftingSystem::ParseMaterialGradeFromID(const FName& MaterialID) const
{
    FString IDStr = MaterialID.ToString().ToLower();

    if (IDStr.EndsWith(TEXT("_poor")))      return EMaterialGrade::Poor;
    if (IDStr.EndsWith(TEXT("_normal")))    return EMaterialGrade::Normal;
    if (IDStr.EndsWith(TEXT("_fine")))      return EMaterialGrade::Fine;
    if (IDStr.EndsWith(TEXT("_excellent"))) return EMaterialGrade::Excellent;
    if (IDStr.EndsWith(TEXT("_perfect")))   return EMaterialGrade::Perfect;

    UE_LOG(LogTemp, Warning, TEXT("无法从材料 ID 解析等级：%s，默认返回 Normal"), *IDStr);
    return EMaterialGrade::Normal;
}
