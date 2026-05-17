#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FollowCameraActor.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class FEEDTHEFISH_API AFollowCameraActor : public AActor
{
    GENERATED_BODY()

public:

    // 地图边界设置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FollowCamera")
        float MapLeft = -1350.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FollowCamera")
        float MapRight = 1350.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FollowCamera")
        float MapBottom = -800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FollowCamera")
        float MapTop = 800.f;

    // 视野设置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FollowCamera")
        float OrthoWidth = 900.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FollowCamera")
        float OrthoHeight = 1600.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FollowCamera")
        float VerticalOffset = 200.f;

    UFUNCTION(BlueprintCallable, Category = "FollowCamera")
        void UpdateToLocation(const FVector& TargetLocation);
};
