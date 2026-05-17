#include "PlayerGameController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "PlayerCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "EnemyActor.h"
#include "DropActor.h"
#include "FollowCameraActor.h"
#include "EngineUtils.h"
#include "NiagaraFunctionLibrary.h"


APlayerGameController::APlayerGameController()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
}

void APlayerGameController::BeginPlay()
{
    Super::BeginPlay();

    // 绑定输入映射上下文
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (ClickMappingContext)
        {
            Subsystem->AddMappingContext(ClickMappingContext, 0);
            UE_LOG(LogTemp, Warning, TEXT("已添加 Click MappingContext！"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("ClickMappingContext 没有设置！"));
        }
    }

    for (TActorIterator<AFollowCameraActor> It(GetWorld()); It; ++It)
    {
        AFollowCameraActor* Cam = *It;
        if (Cam)
        {
            SetViewTargetWithBlend(Cam, 0.f); // 立即切换视角
            break;
        }
    }
}

void APlayerGameController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
    {
        if (ClickAction)
        {
            EnhancedInput->BindAction(ClickAction, ETriggerEvent::Triggered, this, &APlayerGameController::HandleClick);
            UE_LOG(LogTemp, Warning, TEXT("已绑定 ClickAction！"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("ClickAction 没有设置！"));
        }
    }
}

void APlayerGameController::HandleClick(const FInputActionValue& ActionValue)
{
    if (!bEnableClick) return;

    FHitResult Hit;
    GetHitResultUnderCursor(ECC_Visibility, false, Hit);

    if (!Hit.bBlockingHit) return;

    if (ClickEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ClickEffect, Hit.ImpactPoint);
    }


    //if (!EnableClickAction) return;

    APlayerCharacter* LocalPlayer = Cast<APlayerCharacter>(GetPawn());
    if (!LocalPlayer) return;

    AActor* ClickedActor = Hit.GetActor();

    if (ClickedActor && ClickedActor->IsA(AEnemyActor::StaticClass()))
    {
        // ✅ 点击到了敌人
        LocalPlayer->SetTargetEnemy(ClickedActor);
        UE_LOG(LogTemp, Warning, TEXT("点击敌人：%s"), *ClickedActor->GetName());
    }
    else if (ClickedActor && ClickedActor->IsA(ADropActor::StaticClass()))
    {
        if (ADropActor* DropActor = Cast<ADropActor>(ClickedActor)) 
        {
            DropActor->Pickup();
        }
    }
    else
    {
        FVector FinalTarget = ClampToMapBounds(Hit.ImpactPoint);
        // ✅ 点击地面，清空目标，直接移动
        LocalPlayer->ClearTargetEnemy();
        LocalPlayer->MoveToLocation(FinalTarget);
        UE_LOG(LogTemp, Warning, TEXT("点击移动：%s"), *FinalTarget.ToString());
    }

    OnTutorialClick(ClickedActor, Hit.ImpactPoint);
}

FVector APlayerGameController::ClampToMapBounds(const FVector& RawTarget) const
{
    float ClampX = FMath::Clamp(RawTarget.X, MapLeft + BoundaryBufferHorizontal, MapRight - BoundaryBufferHorizontal);
    float ClampZ = FMath::Clamp(RawTarget.Z, MapBottom + BoundaryBufferVertical, MapTop - BoundaryBufferVertical);
    return FVector(ClampX, 0.f, ClampZ);
}
