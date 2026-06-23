// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Animation/BowAnimInstance.h"
#include "Character/Archer.h"
#include "Components/SkeletalMeshComponent.h"

void UBowAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwningArcher)
    {
        if (USkeletalMeshComponent* OwnerComp = GetSkelMeshComponent())
        {
            OwningArcher = Cast<AArcher>(OwnerComp->GetOwner());
        }
    }

    if (!OwningArcher)
    {
        return;
    }

    bIsRecoiling = OwningArcher->GetIsRecoiling();

    if (bIsRecoiling)
    {
        DrawHandLocation = OwningArcher->GetMesh()->GetSocketLocation(FName("RightHandSocket"));
    }
}
