#include "FollowCameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"


void AFollowCameraActor::UpdateToLocation(const FVector& TargetLocation)
{
    float HalfWidth = OrthoWidth / 2.f;
    float HalfHeight = OrthoHeight / 2.f;

    float ClampX = FMath::Clamp(TargetLocation.X, MapLeft + HalfWidth, MapRight - HalfWidth);
    float ClampZ = FMath::Clamp(TargetLocation.Z, MapBottom + HalfHeight, MapTop - HalfHeight);

    SetActorLocation(FVector(ClampX, -100.f, ClampZ + VerticalOffset));
}