// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <DropSystem.h>
#include "GameFramework/Actor.h"
#include "DropActor.generated.h"


UCLASS()
class FEEDTHEFISH_API ADropActor : public AActor
{
	GENERATED_BODY()
	
public:
    ADropActor();

    // 掉落类型
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
        EDropItemType DropType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
        FName DropID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
        FText DropName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        EMaterialGrade DropGrade = EMaterialGrade::Normal;

    // 掉落图标
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drop")
        class UPaperSpriteComponent* SpriteComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
        FText DropDescription;

    // 初始化掉落物属性
    UFUNCTION(BlueprintCallable)
        void InitializeDrop(EDropItemType InType, FName InID, const FText& InName, EMaterialGrade InGrade, class UPaperSprite* InSprite, const FText& InDescription);

    // 玩家拾取
    UFUNCTION()
        void Pickup();

protected:
    virtual void BeginPlay() override;

};
