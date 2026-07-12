// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ArcherPC.generated.h"

/**
 * 
 */
class UTCPClientSubsystem;
UCLASS()
class VALHEIM_API AArcherPC : public APlayerController
{
	GENERATED_BODY()

public:
	AArcherPC();

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	UTCPClientSubsystem* GetTCP();

	UFUNCTION()
	void HandleConnected();

	UFUNCTION()
	void HandleDisconnect();

};
