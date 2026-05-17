#include "DropActor.h"
#include "PlayerCharacter.h"
#include "InventoryManager.h"
//#include "InventoryManager.h"

ADropActor::ADropActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // 可选：初始化可视化组件（例如加个StaticMesh或Billboard）
    // StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    // RootComponent = StaticMeshComponent;
    SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));
    RootComponent = SpriteComponent; // ✅ 将 Sprite 设置为根组件

    SpriteComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SpriteComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    SpriteComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);  // 启用与视距的点击响应
    SpriteComponent->SetGenerateOverlapEvents(false);
}

void ADropActor::BeginPlay()
{
    Super::BeginPlay();

    SetActorScale3D(FVector(0.2f));
}

void ADropActor::InitializeDrop(EDropItemType InType, FName InID, const FText& InName, EMaterialGrade InGrade, UPaperSprite* InSprite, const FText& InDescription)
{
    DropType = InType;
    DropID = InID;
    DropGrade = InGrade;
    DropName = InName;

    // 处理描述
    if (DropType == EDropItemType::MainMaterial)
    {
        // 如果是主材料，前面加上材料等级
        FText GradeText = StaticEnum<EMaterialGrade>()->GetDisplayNameTextByValue((int64)DropGrade);

        // 合成新的描述，比如：“优秀的章鱼触手”
        DropDescription = FText::Format(FText::FromString(TEXT("{0}的{1}")), GradeText, InDescription);
    }
    else
    {
        DropDescription = InDescription;
    }

    if (InSprite && SpriteComponent)
    {
        SpriteComponent->SetSprite(InSprite);
    }
}

void ADropActor::Pickup()
{
    if (UInventoryManager* Inventory = GetGameInstance()->GetSubsystem<UInventoryManager>())
    {
        UPaperSprite* Sprite = SpriteComponent ? SpriteComponent->GetSprite() : nullptr;
        UTexture2D* Icon = Sprite ? Sprite->GetBakedTexture() : nullptr;

        Inventory->AddItem(DropID, DropType, DropName, DropGrade, Icon, DropDescription);
    }

    Destroy();
}
