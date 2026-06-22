// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BowAnimInstance.generated.h"

/**
 * 
 */
class AArcher;
UCLASS()
class VALHEIM_API UBowAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
    UPROPERTY(BlueprintReadOnly, Category = "Bow")
    FVector DrawHandLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Bow")
    bool bIsDrawingBow = false;

protected:
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    UPROPERTY()
    AArcher* OwningArcher;
};
