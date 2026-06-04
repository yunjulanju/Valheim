// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ArcherAnimIns.generated.h"

/**
 * 
 */
UCLASS()
class VALHEIM_API UArcherAnimIns : public UAnimInstance
{
    GENERATED_BODY()

public:

    virtual void NativeInitializeAnimation() override;

    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    UFUNCTION()
    void AnimNotify_AttackStart();
    UFUNCTION()
    void AnimNotify_AttackEnd();
    UFUNCTION()
    void AnimNotify_Attack();

//PROPERTY

    class AArcher* CharacterRef;
    class UCharacterMovementComponent* CharacterMovementRef;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bShoudMove;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsFalling;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsCrouching;


};
