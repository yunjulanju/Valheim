// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Monster.generated.h"

UCLASS()
class VALHEIM_API AMonster : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMonster();

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

///////////////////////////////////////////////////PRPOPERTY
protected:
	float MaxHP = 50.0f;

	float HP;

};
