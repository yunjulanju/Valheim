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

	if (Speed > 1.0f)
	{
		StopAllMontages(0.3);
		MonsterRef->SetCurrentState(EMonsterState::Moving);
		CurrentState = EMonsterState::Moving;
	}
	else if (MonsterRef->GetCurrentState() != EMonsterState::Attacking)
	{
		MonsterRef->SetCurrentState(EMonsterState::Idle);
		CurrentState = EMonsterState::Idle;
		
	}else
	{
		CurrentState = EMonsterState::Attacking;
	}

}

void UMonsterAnimIns::AnimNotify_MonsterAttack()
{
	if (!MonsterRef)
	{
		return;
	}
	MonsterRef->CallAttackCollision();
}
void UMonsterAnimIns::AnimNotify_MonsterAttackEnd()
{
	if (!MonsterRef)
	{
		return;
	}
	MonsterRef->SetCurrentState(EMonsterState::Idle);
}
void UMonsterAnimIns::AnimNotify_MonsterDeath()
{
	if (!MonsterRef)
	{
		return;
	}
	MonsterRef->EndDeath();
}
