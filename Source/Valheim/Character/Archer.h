// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Archer.generated.h"

UENUM(BlueprintType)
enum class EEquipType : uint8
{
	None    UMETA(DisplayName = "None"),
	Sword   UMETA(DisplayName = "Sword"),
	Bow     UMETA(DisplayName = "Bow")
};

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

	void EquipWeapon(UItemDataBase* Weapon);
	void UnequipAllWeapon();

public:
	void CallAttackCollision();

protected:
	UFUNCTION(Server, Reliable)
	void ServerCallAttackCollision();

	void SelectHotbar1(); void SelectHotbar2(); void SelectHotbar3();
	void SelectHotbar4(); void SelectHotbar5(); void SelectHotbar6();
	void SelectHotbar7(); void SelectHotbar8(); void SelectHotbar9();

	void SetActiveHotbarIndex(int32 NewIndex);

	void RefreshActiveHotbarEquip();

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

	//Weapon
	FORCEINLINE EEquipType GetEquipType() const { return CurrentEquipType; }
	FORCEINLINE void SetEquipType(EEquipType NewEquipType) { CurrentEquipType = NewEquipType; }


	//---------------Property

	UPROPERTY(BlueprintAssignable)
	FOnHPChanged OnHPChanged;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* SwordMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* BowMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UInventoryComponent* PlayerInventory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hotbar")
	int32 ActiveHotbarIndex = 0;

	float MaxHP = 100;
	float HP;
	float DefaultDamage = 10.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equip")
	EEquipType CurrentEquipType = EEquipType::None;

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

	/** Attack HotBar Action */
	UPROPERTY(EditAnywhere, Category = "Input|Hotbar")
	TArray<UInputAction*> HotbarActions;
};
