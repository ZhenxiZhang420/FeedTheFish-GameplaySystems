#include "DropSystem.h"
#include "DropActor.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "PaperSprite.h" 
#include "FingerClickActor.h"

void UDropSystem::GenerateDrops(UWorld* World, FName EnemyRowName, FVector SpawnLocation)
{
    if (!World || !DropTable) return;

    if (EnemyRowName == "TutorialShark")
    {
        GenerateTutorialDrops(World, SpawnLocation);
        return;
    }

    FEnemyDropConfig* DropConfig = DropTable->FindRow<FEnemyDropConfig>(EnemyRowName, TEXT("DropSearch"));
    if (!DropConfig) return;

    bool bMainDropped = false;

    for (const FDropItemConfig& Item : DropConfig->DropItems)
    {
        if (Item.ItemType == EDropItemType::MainMaterial)
        {
            if (bMainDropped) continue;

            if (FMath::FRand() <= Item.DropChance)
            {
                bMainDropped = true;

                EMaterialGrade Grade = EMaterialGrade::Normal;
                if (Item.GradeChances.Num() > 0)
                {
                    Grade = GetRandomGradeFromMap(Item.GradeChances);
                }

                FVector Offset = GetRandomDropOffset();
                ADropActor* Drop = World->SpawnActor<ADropActor>(ADropActor::StaticClass(), SpawnLocation + Offset, FRotator::ZeroRotator);
                if (Drop)
                {
                    Drop->InitializeDrop(Item.ItemType, Item.ItemID, Item.ItemName, Grade, Item.Sprite, Item.Description);
                    // Drop->SetItemID(Item.ItemID);
                    // Drop->SetMaterialGrade(Grade);

                    FString GradeString = UEnum::GetValueAsString(Grade).Replace(TEXT("EMaterialGrade::"), TEXT(""));
                    UE_LOG(LogTemp, Log, TEXT("掉落主材料：%s，品质：%s"), *Item.ItemID.ToString(), *GradeString);
                }
            }
        }
        else // 副材料、宝石等
        {
            if (FMath::FRand() <= Item.DropChance)
            {
                int32 Quantity = FMath::RandRange(1, Item.MaxQuantity);
                for (int32 i = 0; i < Quantity; ++i)
                {
                    FVector Offset = GetRandomDropOffset();
                    ADropActor* Drop = World->SpawnActor<ADropActor>(ADropActor::StaticClass(), SpawnLocation + Offset, FRotator::ZeroRotator);
                    if (Drop)
                    {
                        EMaterialGrade Grade = EMaterialGrade::Normal;
                        Drop->InitializeDrop(Item.ItemType, Item.ItemID, Item.ItemName, Grade, Item.Sprite, Item.Description);
                        // Drop->SetItemID(Item.ItemID);
                        UE_LOG(LogTemp, Log, TEXT("掉落副材料：%s"), *Item.ItemID.ToString());
                    }
                }
            }
        }
    }
}

EMaterialGrade UDropSystem::GetRandomGradeFromMap(const TMap<EMaterialGrade, float>& GradeChances)
{
    float TotalWeight = 0.f;
    for (const auto& Pair : GradeChances)
    {
        TotalWeight += Pair.Value;
    }

    float RandomPoint = FMath::FRandRange(0.f, TotalWeight);
    float CurrentSum = 0.f;

    for (const auto& Pair : GradeChances)
    {
        CurrentSum += Pair.Value;
        if (RandomPoint <= CurrentSum)
        {
            return Pair.Key;
        }
    }

    return EMaterialGrade::Normal;
}

FVector UDropSystem::GetRandomDropOffset(float MinDistance, float MaxRadius, float MinY, float MaxY)
{
    float Angle = FMath::FRandRange(0.f, 360.f);
    float Distance = FMath::FRandRange(MinDistance, MaxRadius);

    float X = FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance;
    float Z = FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance;
    float Y = FMath::FRandRange(MinY, MaxY); // ✅ 添加 Y 方向偏移

    return FVector(X, Y, Z);
}

void UDropSystem::GenerateTutorialDrops(UWorld* World, FVector SpawnLocation)
{
    if (!World || !DropTable) return;

    World->SpawnActor<AFingerClickActor>(FingerClickActorClass, SpawnLocation, FRotator::ZeroRotator);

    // 从 "Shark" 获取掉落配置
    FEnemyDropConfig* DropConfig = DropTable->FindRow<FEnemyDropConfig>("Shark", TEXT("DropSearch"));
    if (!DropConfig) return;

    // 要掉落的指定材料（ItemID → Quantity）
    TMap<FName, int32> TutorialDropMap = {
        { "shark_upper_jaw", 1 },
        { "shark_heart", 1 },
        { "shark_teeth", 2 },
        { "shark_skin", 1 }
    };

    for (const auto& Item : DropConfig->DropItems)
    {
        int32* DropCount = TutorialDropMap.Find(Item.ItemID);
        if (!DropCount) continue; // 非指定材料，跳过

        for (int32 i = 0; i < *DropCount; ++i)
        {
            FVector Offset = GetRandomDropOffset();
            ADropActor* Drop = World->SpawnActor<ADropActor>(ADropActor::StaticClass(), SpawnLocation + Offset, FRotator::ZeroRotator);
            if (Drop)
            {
                Drop->InitializeDrop(
                    Item.ItemType,
                    Item.ItemID,
                    Item.ItemName,
                    EMaterialGrade::Normal,
                    Item.Sprite,
                    Item.Description
                );
                UE_LOG(LogTemp, Log, TEXT("【新手掉落】%s ×1（品质：Normal）"), *Item.ItemID.ToString());
            }
        }
    }
}

