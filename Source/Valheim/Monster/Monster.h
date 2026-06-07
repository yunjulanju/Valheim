// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Monster.generated.h"

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	Idle,
	Moving,
	Attacking
};

UCLASS()
class VALHEIM_API AMonster : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMonster();

	class AMonsterAIController* MonsterAIController;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	//Animation
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void CallAttack();

	//Attack Collision
	void CallAttackCollision();

	//Death
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void CallDeathAnimation();

	void EndDeath();
	UFUNCTION(Server, Reliable)
	void Server_EndDeath();

protected:
	UFUNCTION(Server, Reliable)
	void ServerCallAttackCollision();

public:
	FORCEINLINE EMonsterState GetCurrentState() const { return CurrentState; }
	FORCEINLINE void SetCurrentState(EMonsterState NewState) { CurrentState = NewState; }

///////////////////////////////////////////////////PRPOPERTY
protected:
	UPROPERTY(BlueprintReadWrite, Category = "State")
	EMonsterState CurrentState = EMonsterState::Idle;

	float MaxHP = 20.0f;

	float HP;

};
