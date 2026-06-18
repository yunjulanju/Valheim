// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Archer.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;
class UAnimMontage;
class ASword;
class UStaticMeshComponent;
class UInventoryComponent;
class UItemDataBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHPChanged);

UCLASS()
class VALHEIM_API AArcher : public ACharacter
{
	GENERATED_BODY()

protected:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** Sword Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* SwordMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UInventoryComponent* PlayerInventory;

public:
	AArcher();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;

	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void StartCrouch();

	void StopCrouch();

	void ToggleMenuWidget();

	void CallAttack();
	UFUNCTION(Server, Reliable)
	void ServerAttack();
	UFUNCTION(NetMulticast, Reliable)
	void MultiAttack();

	void Interaction();

	void AttachWeapon(ASword* Sword);

public:
	void CallAttackCollision();

protected:
	UFUNCTION(Server, Reliable)
	void ServerCallAttackCollision();

public:
	FORCEINLINE bool IsAttacking() const { return bIsAttacking; }
	FORCEINLINE void SetIsAttacking(bool bNewAttacking) { bIsAttacking = bNewAttacking; }

	FORCEINLINE UInventoryComponent* GetInventory() const { return PlayerInventory; }

	//Inventory
	void DropItem(UItemDataBase* ItemToDrop, const int32 QuantityToDrop);

	void SetHP(float NewHP);
	FORCEINLINE void AddHP(float HealValue) { SetHP(HealValue); }
	FORCEINLINE float GetCurrentHP() { return HP; }
	FORCEINLINE float GetCurrentPercentHP() { return HP/MaxHP; }

	//---------------Property

	UPROPERTY(BlueprintAssignable)
	FOnHPChanged OnHPChanged;

protected:

	float MaxHP = 100;
	float HP;
	float DefaultDamage = 10.0f;

	bool bEquipWeapon = false;

	class AArcherPC* PlayerController;

	class AArcherHUD* HUD;

	//Animation
	UPROPERTY(EditAnywhere, Category = "Anim")
	TObjectPtr<UAnimMontage> AttackMontage;

	//Input
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	/** Crouch Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* CrouchAction;

	/** Attack Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* AttackAction;

	UPROPERTY(Replicated)
	bool bIsAttacking = false;

	/** Interaction Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* InteractAction;

	/** Attack Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MenuAction;
};
