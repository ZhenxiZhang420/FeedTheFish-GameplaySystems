#include "FingerClickActor.h"
#include "Components/WidgetComponent.h"

AFingerClickActor::AFingerClickActor()
{
    PrimaryActorTick.bCanEverTick = false;

    WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
    RootComponent = WidgetComponent;

    //WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
    //WidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFingerClickActor::BeginPlay()
{
    Super::BeginPlay();
}
