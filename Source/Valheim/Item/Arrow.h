// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Arrow.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
UCLASS()
class VALHEIM_API AArrow : public AActor
{
	GENERATED_BODY()
	
public:	
	AArrow();

	void InitializeArrow(float InDamage, AController* InInstigator, AActor* InDamageCauser);
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

public:	
	virtual void Tick(float DeltaTime) override;

	//--------------PROPERTY
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* ArrowMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere, Category = "Arrow")
	float Damage = 15.0f;

	UPROPERTY(EditAnywhere, Category = "Arrow")
	float LifeSpanAfterHit = 5.0f;

	UPROPERTY()
	AController* InstigatorController;

	UPROPERTY()
	AActor* DamageCauserActor;

	bool bHasHit = false;
};
