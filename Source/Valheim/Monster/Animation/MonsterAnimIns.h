// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MonsterAnimIns.generated.h"

/**
 * 
 */
UCLASS()
class VALHEIM_API UMonsterAnimIns : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTimeX) override;

	class AMonster* MonsterRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	FVector Velocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float Speed;

};
