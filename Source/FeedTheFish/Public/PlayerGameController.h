#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "NiagaraFunctionLibrary.h"
#include "DropSystem.h"
#include "PlayerGameController.generated.h"


class UInputMappingContext;
class UInputAction;

USTRUCT(BlueprintType)
struct FTaskConfig : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FText TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 TaskNum;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        UTexture2D* TaskRewardIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FName RewardItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        EMaterialGrade MaterialGrade = EMaterialGrade::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 RewardItemNum;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FName RequiredTriggerID; // 比如合成ID、击杀ID等
};

/**
 * 玩家控制器，支持增强输入系统（点击移动）
 */
UCLASS()
class FEEDTHEFISH_API APlayerGameController : public APlayerController
{
    GENERATED_BODY()

public:
    APlayerGameController();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
        bool bEnableClick = true;

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    // 处理点击事件
    void HandleClick(const FInputActionValue& ActionValue);

public:
    // 映射上下文（IMC_Player）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input")
        UInputMappingContext* ClickMappingContext;

    // 点击动作（IA_Click）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input")
        UInputAction* ClickAction;

    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
    //    bool EnableClickAction = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
        UNiagaraSystem* ClickEffect; 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MapBoundary")
        float MapLeft = -1350.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MapBoundary")
        float MapRight = 1350.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MapBoundary")
        float MapBottom = -800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MapBoundary")
        float MapTop = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MapBoundary")
        float BoundaryBufferHorizontal = 80.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MapBoundary")
        float BoundaryBufferVertical = 50.f;

    UFUNCTION(BlueprintImplementableEvent, Category = "Tutorial")
        void OnTutorialClick(AActor* ClickedActor, FVector ClickLocation);

private:
    FVector ClampToMapBounds(const FVector& RawTarget) const;
};
