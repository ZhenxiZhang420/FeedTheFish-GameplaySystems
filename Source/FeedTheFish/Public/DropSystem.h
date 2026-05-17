// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PaperSprite.h" 
#include "Engine/DataTable.h"
#include "FingerClickActor.h"
#include "DropSystem.generated.h"


UENUM(BlueprintType)
enum class EDropItemType : uint8
{
    MainMaterial UMETA(DisplayName = "Main Material"),
    SubMaterial UMETA(DisplayName = "Sub Material"),
    Gem UMETA(DisplayName = "Gem"),
    SkillFragment UMETA(DisplayName = "Skill Fragment"),
    Blueprint UMETA(DisplayName = "Blueprint"),
    Equipment UMETA(DisplayName = "Equipment")
};

UENUM(BlueprintType)
enum class EMaterialGrade : uint8
{
    Poor      UMETA(DisplayName = "劣质"),
    Normal    UMETA(DisplayName = "普通"),
    Fine      UMETA(DisplayName = "良好"),
    Excellent UMETA(DisplayName = "优秀"),
    Perfect   UMETA(DisplayName = "完美")
};

USTRUCT(BlueprintType)
struct FDropItemConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        EDropItemType ItemType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FName ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FText ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float DropChance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 MaxQuantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TMap<EMaterialGrade, float> GradeChances;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        UPaperSprite* Sprite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FText Description;
};

USTRUCT(BlueprintType)
struct FEnemyDropConfig : public FTableRowBase
{
    GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TArray<FDropItemConfig> DropItems;
};

UCLASS(Blueprintable)
class FEEDTHEFISH_API UDropSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DropSystem")
        UDataTable* DropTable;

    UFUNCTION(BlueprintCallable)
        void GenerateDrops(UWorld* World, FName EnemyRowName, FVector SpawnLocation);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
        TSubclassOf<AFingerClickActor> FingerClickActorClass;

private:
    EMaterialGrade GetRandomGradeFromMap(const TMap<EMaterialGrade, float>& GradeChances);

    FVector GetRandomDropOffset(float MinDistance = 30.f, float MaxRadius = 80.f, float MinY = -10.f, float MaxY = 10.f);

    void GenerateTutorialDrops(UWorld* World, FVector SpawnLocation);
};
