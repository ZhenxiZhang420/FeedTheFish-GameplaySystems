// CraftingSystem.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DropActor.h"
#include "PaperSprite.h"
#include "CraftingSystem.generated.h"

class UInventoryManager;

UENUM(BlueprintType)
enum class EEquipmentSlot : uint8
{
    Head    UMETA(DisplayName = "头"),
    Back    UMETA(DisplayName = "背"),
    Tail    UMETA(DisplayName = "尾"),
    Fin     UMETA(DisplayName = "鳍"),
    Gill    UMETA(DisplayName = "鳃"),
    Belly   UMETA(DisplayName = "腹")
};

USTRUCT(BlueprintType)
struct FEquipmentCraftingRecipe : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FName ResultEquipmentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FText EquipmentDisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FName MainMaterialID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TMap<FName, int32> SubMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TMap<FName, UTexture2D*> SubMaterialTextures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        EEquipmentSlot EquipSlot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        UTexture2D* ResultTexture;
};

/**
 * 合成系统
 */
UCLASS()
class FEEDTHEFISH_API UCraftingSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Crafting")
        void CraftEquipment(FName MainMaterialID, FName ResultEquipmentID);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
        bool HasEnoughMaterials(FName MainMaterialID, FName ResultEquipmentID) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
        TArray<FEquipmentCraftingRecipe> GetRecipesByMainMaterialBaseID(FName MainMaterialIDWithGrade) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
        UTexture2D* GetSubMaterialTexture(FName MaterialID) const;

    UFUNCTION(BlueprintPure, Category = "Crafting")
        FText GetMaterialGradeTextFromID(FName MaterialID) const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
        UDataTable* EquipmentRecipeTable;

private:
    UInventoryManager* GetInventory() const;
    EMaterialGrade ParseMaterialGradeFromID(const FName& MaterialID) const;
};
