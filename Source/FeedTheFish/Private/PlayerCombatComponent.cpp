#include "PlayerCombatComponent.h"
#include "EnemyActor.h"
#include "CharacterAttributeComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "CombatSystem.h"
#include "SkillManager.h"
#include "PlayerCharacter.h"
#include "PaperFlipbook.h"
#include <PlayerGameController.h>

UPlayerCombatComponent::UPlayerCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerCombatComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerActor = GetOwner();
    AttributeComponent = OwnerActor->FindComponentByClass<UCharacterAttributeComponent>();
}

void UPlayerCombatComponent::Initialize()
{
    if (AttributeComponent)
    {
        AttributeComponent->OnDeath.AddDynamic(this, &UPlayerCombatComponent::StopAutoAttack);
    }
}

void UPlayerCombatComponent::HandleAttackOverlap(AActor* OtherActor)
{
    if (AEnemyActor* Enemy = Cast<AEnemyActor>(OtherActor))
    {
        if (APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwner()))
        {
            if (Player->TargetEnemy != nullptr && Enemy == Player->TargetEnemy) 
            {
                Player->bIsMoving = false; // ✅ 停止玩家移动
                StartAutoAttack(Enemy);
            }
        }
    }
}

void UPlayerCombatComponent::StartAutoAttack(AEnemyActor* Enemy)
{
    if (!Enemy || bIsAttacking || !AttributeComponent || !AttributeComponent->IsAlive()) return;

    UE_LOG(LogTemp, Warning, TEXT("StartAutoAttack"));

    bIsAttacking = true;

    if (APlayerCharacter* Player = Cast<APlayerCharacter>(OwnerActor))
    {
        // ✅ 自动翻转朝向
        FVector Scale = Player->FlipbookComponent->GetRelativeScale3D();
        float BaseScaleX = FMath::Abs(Scale.X);
        float NewScaleX = (Enemy->GetActorLocation().X < Player->GetActorLocation().X) ? -BaseScaleX : BaseScaleX;
        Player->FlipbookComponent->SetRelativeScale3D(FVector(NewScaleX, Scale.Y, Scale.Z));

        // ✅ 禁用点击（让玩家不能再点击其他目标）
        if (APlayerController* PC = Cast<APlayerController>(Player->GetController()))
        {
            // 你自定义的 PlayerController 类，替换成你实际类名
            if (APlayerGameController* MyPC = Cast<APlayerGameController>(PC))
            {
                MyPC->bEnableClick = false;
            }
        }
    }

    PerformAttack(Enemy);

    float Interval = 1.f / FMath::Max(AttributeComponent->AttackSpeed, 0.01f);

    PlayAttackAnimation();

    GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, [this, Enemy]()
        {
            if (!Enemy || !IsValid(Enemy) || !Enemy->IsAlive())
            {
                StopAutoAttack();
                return;
            }

            PerformAttack(Enemy);

        }, Interval, true);
}


void UPlayerCombatComponent::PerformAttack(AEnemyActor* Enemy)
{
    BasicAttack(Enemy);

    if (!Enemy->IsAlive())
    {
        StopAutoAttack();

        if (APlayerCharacter* Player = Cast<APlayerCharacter>(OwnerActor))
        {
            // ✅ 回满血
            if (UCharacterAttributeComponent* Attr = Player->AttributeComponent)
            {
                Attr->RestoreFullHealth(true);
            }

            // ✅ 恢复点击
            if (APlayerController* PC = Cast<APlayerController>(Player->GetController()))
            {
                if (APlayerGameController* MyPC = Cast<APlayerGameController>(PC))
                {
                    MyPC->bEnableClick = true;
                }
            }

            Player->ClearTargetEnemy();
        }

    }
}


void UPlayerCombatComponent::BasicAttack(AActor* Target)
{
    if (!Target || !AttributeComponent) return;

    float BaseDamage = AttributeComponent->GetTotalAttack();

    if (DefaultHitEffect && OwnerActor)
    {
        APlayerCharacter* Player = Cast<APlayerCharacter>(OwnerActor);
        if (Player && Player->FlipbookComponent)
        {
            float FacingDir = FMath::Sign(Player->FlipbookComponent->GetComponentScale().X);

            FVector Origin = Player->GetActorLocation();
            FVector SpawnLocation = Origin + FVector(130.f * FacingDir, 0.f, 0.f); // 只往前，不抬高

            UE_LOG(LogTemp, Warning, TEXT("击中特效位置：%s"), *SpawnLocation.ToString());

            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                DefaultHitEffect,
                SpawnLocation,
                FRotator::ZeroRotator,
                FVector(1.f)
            );
        }
    }


    UCombatSystem::ApplyDamage(OwnerActor, Target, BaseDamage, FSkillData());
}

void UPlayerCombatComponent::PlayAttackAnimation()
{
    if (!OwnerActor) return;

    if (APlayerCharacter* Player = Cast<APlayerCharacter>(OwnerActor))
    {
        if (Player->FlipbookComponent && Player->BasicAttackFlipbook)
        {
            Player->FlipbookComponent->SetFlipbook(Player->BasicAttackFlipbook);

            float PlayRate = Player->BasicAttackFlipbook->GetTotalDuration() * AttributeComponent->AttackSpeed;
            Player->FlipbookComponent->SetPlayRate(PlayRate);
            Player->FlipbookComponent->Play();
        }
    }
}


void UPlayerCombatComponent::StopAutoAttack()
{
    bIsAttacking = false;
    GetWorld()->GetTimerManager().ClearTimer(AttackTimerHandle);

    if (AttributeComponent->IsAlive()) 
    {
        if (APlayerCharacter* Player = Cast<APlayerCharacter>(OwnerActor))
        {
            if (Player->FlipbookComponent && Player->IdleFlipbook)
            {
                Player->FlipbookComponent->SetFlipbook(Player->IdleFlipbook);
                Player->FlipbookComponent->SetPlayRate(1.5f);
                Player->FlipbookComponent->Play();
            }
        }
    }

}

void UPlayerCombatComponent::LockSkillInput(float Duration)
{
    StopAutoAttack(); // 停止当前自动攻击

    GetWorld()->GetTimerManager().SetTimer(SkillLockTimerHandle, [this]()
        {
            if (APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwner()))
            {
                if (AEnemyActor* Enemy = Cast<AEnemyActor>(Player->TargetEnemy))
                {
                    if (Enemy->IsAlive())
                    {
                        StartAutoAttack(Enemy);
                    }
                }
            }
        }, Duration, false);
}

void UPlayerCombatComponent::RestartAutoAttack()
{
    StopAutoAttack();
    if (APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwner()))
    {
        if (AEnemyActor* Enemy = Cast<AEnemyActor>(Player->TargetEnemy))
        {
            if (Enemy->IsAlive())
            {
                StartAutoAttack(Enemy);
            }
        }
    }
}






