#include "PlayerCharacter.h"
#include "SkillManager.h"
#include "PlayerCombatComponent.h"
#include "CharacterAttributeComponent.h"
#include "EnemyActor.h"

#include "PaperSpriteComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EngineUtils.h"
#include "Components/ProgressBar.h"
#include "DrawDebugHelpers.h"
#include "EnemyPatrolComponent.h"

APlayerCharacter::APlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // 组件创建
    SkillManager = CreateDefaultSubobject<USkillManager>(TEXT("SkillManager"));
    AttributeComponent = CreateDefaultSubobject<UCharacterAttributeComponent>(TEXT("AttributeComponent"));
    CombatComponent = CreateDefaultSubobject<UPlayerCombatComponent>(TEXT("CombatComponent"));

    SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("Sprite"));
    SpriteComponent->SetupAttachment(RootComponent);
    SpriteComponent->SetRelativeLocation(FVector(0, 0, 0));

    FlipbookComponent = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("Flipbook"));
    FlipbookComponent->SetupAttachment(RootComponent);
    FlipbookComponent->SetRelativeLocation(FVector(0, 0, 0));

    AttackCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackCollider"));
    AttackCollider->SetupAttachment(RootComponent);
    AttackCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    AttackCollider->SetCollisionResponseToAllChannels(ECR_Ignore);
    AttackCollider->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    AttackCollider->SetBoxExtent(FVector(30.f, 30.f, 30.f));

    // 移动限制设置
    GetCharacterMovement()->GravityScale = 0.0f;
    GetCharacterMovement()->bConstrainToPlane = true;
    GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0, 1, 0));

    WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
    WidgetComponent->SetupAttachment(RootComponent);
}

void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    // 初始化当前血量
    if (AttributeComponent)
    {
        AttributeComponent->CurrentHP = AttributeComponent->MaxHP;

        // 设置死亡状态响应
        AttributeComponent->OnDeath.AddDynamic(this, &APlayerCharacter::OnCharacterDeath);
    }

    // 初始化战斗组件
    if (CombatComponent)
    {
        CombatComponent->Initialize();
    }

    for (TActorIterator<AFollowCameraActor> It(GetWorld()); It; ++It)
    {
        FollowCameraActor = *It;
        FollowCameraActor->UpdateToLocation(GetActorLocation());
    }
}

void APlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsMoving) return;

    FVector CurrentLocation = GetActorLocation();
    FVector TargetLocation = TargetMoveLocation;
    FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
    FVector NewLocation = CurrentLocation + Direction * SwimSpeed * DeltaTime;

    float DistanceToTarget = FVector::Dist(CurrentLocation, TargetLocation);

    if (DistanceToTarget <= 10.f)
    {
        //SetActorLocation(TargetLocation);
        FlipbookComponent->SetPlayRate(1.5f);
        bIsMoving = false;
    }
    else
    {
        if (FMath::Abs(Direction.X) > 0.01f)
        {
            FVector CurrentScale = FlipbookComponent->GetRelativeScale3D();
            float BaseScaleX = FMath::Abs(CurrentScale.X);
            FlipbookComponent->SetRelativeScale3D(FVector(Direction.X < 0 ? -BaseScaleX : BaseScaleX, CurrentScale.Y, CurrentScale.Z));
        }
        SetActorLocation(NewLocation);
        FollowCameraActor->UpdateToLocation(NewLocation);
    }

}

bool APlayerCharacter::TryUseSkill(FName SkillID, AActor* Caster, AActor* Target)
{
    if (CurrentState != ECharacterState::Normal || !SkillManager) return false;
    return SkillManager->TryUseSkill(SkillID, Caster, Target);
}

void APlayerCharacter::MoveToLocation(FVector Location)
{
    TargetMoveLocation = Location;
    bIsMoving = true;

    if (FlipbookComponent && IdleFlipbook && FlipbookComponent->GetFlipbook() != IdleFlipbook)
    {
        FlipbookComponent->SetFlipbook(IdleFlipbook);
    }
    FlipbookComponent->SetPlayRate(3.f);
    FlipbookComponent->Play();
}

void APlayerCharacter::SetTargetEnemy(AActor* Enemy)
{
    //UE_LOG(LogTemp, Log, TEXT("SetTargetEnemy called"));
    //UE_LOG(LogTemp, Log, TEXT("TargetEnemy is %s"), TargetEnemy ? *TargetEnemy->GetName() : TEXT("nullptr"));
    //UE_LOG(LogTemp, Log, TEXT("Enemy is %s"), Enemy ? *Enemy->GetName() : TEXT("nullptr"));
    if (!TargetEnemy && Enemy)
    {
        UE_LOG(LogTemp, Log, TEXT("SetTargetEnemy After Check"));
        TargetEnemy = Enemy;

        // 获取怪物位置与朝向
        FVector EnemyLocation = Enemy->GetActorLocation();
        FVector EnemyForward = Enemy->GetActorForwardVector();

        // 获取怪物头尾偏移
        float EnemyOffset = 80.f;
        AEnemyActor* EnemyActor = Cast<AEnemyActor>(Enemy);
        if (EnemyActor && EnemyActor->HurtBox)
        {
            EnemyOffset = EnemyActor->HurtBox->GetScaledBoxExtent().X;
            UE_LOG(LogTemp, Log, TEXT("Enemy Offset (BoxExtent X): %.2f"), EnemyOffset);
        }

        // 计算头尾位置
        FVector HeadLocation = EnemyLocation + EnemyForward * EnemyOffset;
        FVector TailLocation = EnemyLocation - EnemyForward * EnemyOffset;

        // 获取玩家头部偏移
        float PlayerHeadOffset = 40.f;
        if (AttackCollider)
        {
            PlayerHeadOffset = AttackCollider->GetScaledBoxExtent().X - 20.f;
            UE_LOG(LogTemp, Log, TEXT("Player Head Offset (BoxExtent X): %.2f"), PlayerHeadOffset);
        }

        // ✅ 计算头部和尾部的延伸点（往外推玩家头部长度）
        FVector HeadApproachPoint = HeadLocation + EnemyForward * PlayerHeadOffset;
        FVector TailApproachPoint = TailLocation - EnemyForward * PlayerHeadOffset;

        // ✅ 判断哪个点离玩家更近
        FVector PlayerLocation = GetActorLocation();
        float DistToHead = FVector::Dist(PlayerLocation, HeadApproachPoint);
        float DistToTail = FVector::Dist(PlayerLocation, TailApproachPoint);

        FVector FinalMoveLocation = (DistToHead < DistToTail) ? HeadApproachPoint : TailApproachPoint;

        // ✅ 可视化调试
        //DrawDebugSphere(GetWorld(), HeadApproachPoint, 15.f, 12, FColor::Cyan, false, 2.f);
        //DrawDebugSphere(GetWorld(), TailApproachPoint, 15.f, 12, FColor::Magenta, false, 2.f);
        //DrawDebugSphere(GetWorld(), FinalMoveLocation, 20.f, 12, FColor::Green, false, 2.f);

        // ✅ 移动玩家
        MoveToLocation(FinalMoveLocation);

        // ✅ 停止敌人巡逻 1 秒
        if (EnemyActor && EnemyActor->PatrolComponent)
        {
            EnemyActor->PatrolComponent->InterruptPatrolForDuration(2.0f);
        }
    }
}





void APlayerCharacter::ClearTargetEnemy()
{
    TargetEnemy = nullptr;
}

void APlayerCharacter::OnCharacterDeath()
{
    //Destroy();
    FlipbookComponent->SetFlipbook(DeathFlipbook);
    FlipbookComponent->SetLooping(false); // ✅ 强制不循环
    FlipbookComponent->SetPlayRate(1.0f);
    FlipbookComponent->Play();

}
