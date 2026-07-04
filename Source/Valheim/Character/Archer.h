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

UENUM(BlueprintType)
enum class EMeshType : uint8
{
	Arrow    UMETA(DisplayName = "Arrow"),
	Sword   UMETA(DisplayName = "Sword"),
	Bow     UMETA(DisplayName = "Bow")
};

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class USkeletalMeshComponent;
struct FInputActionValue;
class UAnimMontage;
class UStaticMeshComponent;
class UInventoryComponent;
class UItemDataBase;
class AArrow;

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

	void StartDrawBow();
	void ReleaseDrawBow();



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
	void ServerAttack_Implementation();
	UFUNCTION(NetMulticast, Reliable)
	void MultiAttack();
	void MultiAttack_Implementation();

	void Interaction();
	UFUNCTION(Server, Reliable)
	void ServerInteraction();
	void ServerInteraction_Implementation();

	UFUNCTION(Server, Reliable)
	void ServerDropItem(int32 InventoryIndex, const int32 QuantityToDrop);
	void ServerDropItem_Implementation(int32 InventoryIndex, const int32 QuantityToDrop);

	UFUNCTION(Server, Reliable)
	void ServerUseItem(int32 InventoryIndex);
	void ServerUseItem_Implementation(int32 InventoryIndex);

	UFUNCTION(Server, Reliable)
	void ServerRecoil();
	void ServerRecoil_Implementation();

	UFUNCTION(NetMulticast, Reliable)
	void MultiRecoil();
	void MultiRecoil_Implementation();

	void EquipWeapon(FName WeaponItemID);
	void UnequipAllWeapon();

	void CallAttackRelease();

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

	void SetHP(float NewHP);
	FORCEINLINE void AddHP(float HealValue) { SetHP(HealValue); }
	FORCEINLINE float GetCurrentHP() { return HP; }
	FORCEINLINE float GetCurrentPercentHP() { return HP / MaxHP; }

	void DropItem(int32 InventoryIndex, const int32 QuantityToDrop);

	void UseItem(int32 InventoryIndex);

	//Weapon
	FORCEINLINE EEquipType GetEquipType() const { return CurrentEquipType; }
	FORCEINLINE void SetEquipType(EEquipType NewEquipType) { CurrentEquipType = NewEquipType; }

	FORCEINLINE bool GetIsDrawing() const { return bIsDrawingBow; }
	FORCEINLINE void SetIsDrawing(bool Drawing) { bIsDrawingBow = Drawing; }

	FORCEINLINE bool GetIsRecoiling() const { return bIsRecoiling; }
	FORCEINLINE void SetIsRecoiling(bool Recoiling) { bIsRecoiling = Recoiling; SetVisiblityMesh(EMeshType::Arrow, Recoiling); }

	void SetVisiblityMesh(EMeshType MeshType, bool OnOff);
	UFUNCTION(Server, Reliable)
	void ServerSetVisiblityMesh(EMeshType MeshType, bool OnOff);
	void ServerSetVisiblityMesh_Implementation(EMeshType MeshType, bool OnOff);
	UFUNCTION(NetMulticast, Reliable)
	void MultiSetVisiblityMesh(EMeshType MeshType, bool OnOff);
	void MultiSetVisiblityMesh_Implementation(EMeshType MeshType, bool OnOff);

	UFUNCTION()
	void OnRep_HP();

	float GetDamageValue();

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
	USkeletalMeshComponent* BowMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* ArrowMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UInventoryComponent* PlayerInventory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hotbar")
	int32 ActiveHotbarIndex = 0;

	float MaxHP = 100;
	UPROPERTY(ReplicatedUsing = OnRep_HP)
	float HP;
	float DefaultDamage = 10.0f;
	
	UPROPERTY(VisibleAnywhere, Replicated, Category = "Equip")
	EEquipType CurrentEquipType = EEquipType::None;

	bool bIsDrawingBow = false;
	bool bIsRecoiling = false;

	float DrawingWalkSpeed = 400;
	float DefaultWalkSpeed = 600;

	class AArcherPC* PlayerController;

	class AArcherHUD* HUD;

	//Arrow Actor
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arrow")
	TSubclassOf<AArrow> ArrowClass;

	//Animation
	UPROPERTY(EditAnywhere, Category = "Anim")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditAnywhere, Category = "Anim")
	TObjectPtr<UAnimMontage> RecoilMontage;

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