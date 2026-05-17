#include "EnemyActor.h"
#include "PaperSpriteComponent.h"
#include "Engine/DataTable.h"
#include "PlayerCharacter.h"
#include "Components/ProgressBar.h"
#include "DropSystem.h"
#include "Kismet/GameplayStatics.h"
#include "CharacterAttributeComponent.h"
#include "CombatSystem.h"
#include "PaperFlipbookComponent.h"
#include "PaperFlipbook.h"
#include "EnemyCombatComponent.h"
#include "EnemyPatrolComponent.h"
#include "EnemyBehaviorBase.h"
#include "SharkBehavior.h"

AEnemyActor::AEnemyActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // 逻辑根组件
    USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = SceneRoot;

    // 动画显示组件
    FlipbookComponent = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("Flipbook"));
    FlipbookComponent->SetupAttachment(SceneRoot);

    // 可选的精灵组件（你不再需要它可以删掉）
    SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("Sprite"));
    SpriteComponent->SetupAttachment(SceneRoot);
    SpriteComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SpriteComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    SpriteComponent->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);

    // 受击盒
    HurtBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HurtBox"));
    HurtBox->SetupAttachment(SceneRoot);
    HurtBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    HurtBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    HurtBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    HurtBox->SetBoxExtent(FVector(40.f, 40.f, 40.f));
    HurtBox->SetRelativeLocation(FVector::ZeroVector);

    // 血条组件
    WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
    WidgetComponent->SetupAttachment(SceneRoot);

    // 行为组件
    CombatComponent = CreateDefaultSubobject<UEnemyCombatComponent>(TEXT("CombatComponent"));
    PatrolComponent = CreateDefaultSubobject<UEnemyPatrolComponent>(TEXT("PatrolComponent"));
    BehaviorInstance = CreateDefaultSubobject<USharkBehavior>(TEXT("BehaviorInstance"));
}


void AEnemyActor::BeginPlay()
{
    Super::BeginPlay();
    InitTargetPlayer();

    if (CombatComponent)
    {
        CombatComponent->InitializeCombat(this);
    }

    if (PatrolComponent)
    {
        PatrolComponent->InitializePatrol(this);
    }

    if (BehaviorInstance)
    {
        BehaviorInstance->StartBehavior();
    }
}

void AEnemyActor::InitializeEnemy(FName InEnemyID, bool bUseBehavior) 
{
    EnemyID = InEnemyID;
    UniqueEnemyID = FGuid::NewGuid();
    if (!EnemyDataTable) return;

    FEnemyConfig* Config = EnemyDataTable->FindRow<FEnemyConfig>(EnemyID, TEXT("Lookup EnemyData"));
    if (Config)
    {
        MaxHP = Config->MaxHP;
        CurrentHP = MaxHP;

        AttackSpeed = Config->AttackSpeed;
        AttackRange = Config->AttackRange;
        ExpReward = Config->ExpReward;
        BaseDamage = Config->BaseDamage;
        Defense = Config->Defense;
        FlipbookMap = Config->Flipbooks;

        if (Config->BehaviorClass)
        {
            BehaviorInstance = NewObject<UEnemyBehaviorBase>(this, Config->BehaviorClass);
            BehaviorInstance->bUseBehavior = bUseBehavior;
            if (BehaviorInstance)
            {
                BehaviorInstance->RegisterComponent(); // 非必需但推荐
                BehaviorInstance->StartBehavior();
            }
        }
    }

    if (FlipbookComponent && FlipbookMap.Contains("Idle"))
    {
        FlipbookComponent->SetFlipbook(FlipbookMap["Idle"]);
        FlipbookComponent->SetPlayRate(1.0f);
        FlipbookComponent->Play();
    }
}

void AEnemyActor::ReceiveDamage(float DamageAmount, AActor* DamageCauser)
{
    if (bIsDead) return;

    CurrentHP -= DamageAmount;

    if (CurrentHP <= 0.0f)
    {
        Die();
    }
    else
    {
        if (CombatComponent && DamageCauser->IsA<APlayerCharacter>())
        {
            CombatComponent->StartAttackingPlayer();
        }
    }

    if (UUserWidget* currentWidget = WidgetComponent->GetUserWidgetObject())
    {
        UProgressBar* HpBarProgress = Cast<UProgressBar>(currentWidget->GetWidgetFromName(TEXT("HpBar")));
        if (HpBarProgress)
        {
            HpBarProgress->SetPercent(CurrentHP / MaxHP);
        }
    }
}

void AEnemyActor::Die()
{
    if (bIsDead) return;
    bIsDead = true;

    OnEnemyDeath.Broadcast(EnemyID);

    // 1. 停止攻击
    if (CombatComponent)
    {
        CombatComponent->StopAttackingPlayer();
    }

    // 2. 播放死亡动画并延迟执行销毁逻辑
    if (FlipbookComponent && FlipbookMap.Contains("Death"))
    {
        FlipbookComponent->SetFlipbook(FlipbookMap["Death"]);
        FlipbookComponent->SetPlayRate(1.0f);
        FlipbookComponent->Play();

        float DeathDuration = FlipbookMap["Death"]->GetTotalDuration();

        GetWorld()->GetTimerManager().SetTimer(
            DeathTimerHandle,
            this,
            &AEnemyActor::OnDeathAnimationFinished,
            DeathDuration,
            false
        );
    }
    else
    {
        // 没有动画，直接进入销毁流程
        OnDeathAnimationFinished();
    }
}


void AEnemyActor::OnDeathAnimationFinished()
{
    // 掉落战利品
    if (UDropSystem* DropSystem = GetGameInstance()->GetSubsystem<UDropSystem>())
    {
        DropSystem->GenerateDrops(GetWorld(), EnemyID, GetActorLocation());
    }

    // 加经验给玩家
    if (APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0)))
    {
        if (UCharacterAttributeComponent* Attr = Player->FindComponentByClass<UCharacterAttributeComponent>())
        {
            Attr->AddExperience(ExpReward);
        }
    }

    // 销毁自身
    Destroy();
}


void AEnemyActor::InitTargetPlayer()
{
    TargetPlayer = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
}

void AEnemyActor::PlayFlipbookByName(FName FlipbookKey, float PlayRate, bool bLoop)
{
    if (!FlipbookComponent || !FlipbookMap.Contains(FlipbookKey) || !FlipbookMap[FlipbookKey])
    {
        UE_LOG(LogTemp, Warning, TEXT("无法播放动画：未找到 FlipbookKey [%s]"), *FlipbookKey.ToString());
        return;
    }

    FlipbookComponent->SetFlipbook(FlipbookMap[FlipbookKey]);
    FlipbookComponent->SetPlayRate(PlayRate);
    FlipbookComponent->SetLooping(bLoop);
    FlipbookComponent->Play();
}

