// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FingerClickActor.generated.h"

class UWidgetComponent;

UCLASS()
class FEEDTHEFISH_API AFingerClickActor : public AActor
{
	GENERATED_BODY()
	
public:
    AFingerClickActor();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
        UWidgetComponent* WidgetComponent;
};
