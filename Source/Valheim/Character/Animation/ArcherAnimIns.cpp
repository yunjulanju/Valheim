// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Animation/ArcherAnimIns.h"
#include "Character/Archer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UArcherAnimIns::NativeInitializeAnimation()
{
	CharacterRef = Cast<AArcher>(TryGetPawnOwner());
	if(CharacterRef)
	{
		CharacterMovementRef = CharacterRef->GetCharacterMovement();
	}
}

void UArcherAnimIns::NativeUpdateAnimation(float DeltaTimeX)
{
	if (!CharacterMovementRef || !CharacterRef)
	{
		return;
	}

	Velocity = CharacterRef->GetVelocity();
	Speed = Velocity.Size2D();

	if (CharacterMovementRef->bOrientRotationToMovement)
	{
		float PreDirection = CalculateDirection(Velocity, CharacterRef->GetBaseAimRotation());
		Direction = FMath::Clamp(PreDirection, -45.0f, 45.0f);
	}
	else
	{
		Direction = CalculateDirection(Velocity, CharacterRef->GetBaseAimRotation());
	}
	
	FVector CurrentAcceleration = CharacterMovementRef->GetCurrentAcceleration();

	bShoudMove = (Speed > 0.01) && !CurrentAcceleration.IsNearlyZero();
	
	bIsFalling = CharacterMovementRef->IsFalling();
	bIsCrouching = CharacterMovementRef->IsCrouching();
	bIsDrawingBow = CharacterRef->GetIsDrawing();

}

void UArcherAnimIns::AnimNotify_AttackStart()
{
	if (CharacterRef)
	{
		CharacterRef->SetIsAttacking(true);
	}
}

void UArcherAnimIns::AnimNotify_AttackEnd()
{
	if (CharacterRef)
	{
		CharacterRef->SetIsAttacking(false);
	}
}

void UArcherAnimIns::AnimNotify_Attack()
{
	if (CharacterRef)
	{
		CharacterRef->CallAttackCollision();
	}
}

void UArcherAnimIns::AnimNotify_Drawing()
{
	if (CharacterRef)
	{
		CharacterRef->SetIsRecoiling(true);
	}
}

void UArcherAnimIns::AnimNotify_Recoil()
{
	if (CharacterRef)
	{
		CharacterRef->SetIsRecoiling(false);
	}
}
