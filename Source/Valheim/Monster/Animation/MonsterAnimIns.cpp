// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Animation/MonsterAnimIns.h"
#include "Monster/Monster.h"

void UMonsterAnimIns::NativeInitializeAnimation()
{
	MonsterRef = Cast<AMonster>(TryGetPawnOwner());
}

void UMonsterAnimIns::NativeUpdateAnimation(float DeltaTimeX)
{
	if (!MonsterRef)
	{
		return;
	}

	Velocity = MonsterRef->GetVelocity();
	Speed = Velocity.Size2D();
}

void UMonsterAnimIns::AnimNotify_MonsterAttack()
{
	if (!MonsterRef)
	{
		return;
	}
	MonsterRef->CallAttackCollision();
}
