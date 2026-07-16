// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Archer.h"
#include "Character/ArcherPC.h"
#include "GameFramework/Controller.h"
#include "Valheim.h"
#include "Net/UnrealNetwork.h" 
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "Kismet/GameplayStatics.h"
#include <Monster/Monster.h>
#include <Item/ItemBase.h>
#include "Inventory/InventoryComponent.h"
#include "Item/ItemSubsystem.h"
#include "Data/ItemPrimaryDataAsset.h"
#include "UserInterface/ArcherHUD.h"
#include "Item/ItemDataBase.h"
#include <Item/Arrow.h>
#include "Interface/Interactable.h"
#include "Character/ArcherPS.h"


AArcher::AArcher()
{
	bReplicates = true;
	bReplicateUsingRegisteredSubObjectList = true;

	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->SocketOffset = FVector(0, 0, 100);
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bDoCollisionTest = false;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	SwordMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwordMesh"));
	SwordMesh->SetupAttachment(GetMesh(), FName("RightHandSocket"));
	SwordMesh->SetVisibility(false);
	SwordMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	BowMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BowMesh"));
	BowMesh->SetupAttachment(GetMesh(), FName("LeftHandSocket"));
	BowMesh->SetVisibility(false);
	BowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ArrowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowMesh"));
	ArrowMesh->SetupAttachment(GetMesh(), FName("ArrowSocket"));
	ArrowMesh->SetVisibility(false);
	ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HP = MaxHP;

	PlayerInventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
	PlayerInventory->SetSlotsCapacity(20);

}

void AArcher::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PlayerController = Cast<AArcherPC>(PC);

		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}

		HUD = Cast<AArcherHUD>(PC->GetHUD());
	}
	if (PlayerInventory)
	{
		PlayerInventory->OnInventoryUpdated.AddUObject(this, &AArcher::RefreshActiveHotbarEquip);
	}
	SetActiveHotbarIndex(0);
	SetEquipType(EEquipType::None);
}

void AArcher::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AArcher::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AArcher::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AArcher::Look);

		//Crouching
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AArcher::StartCrouch);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AArcher::StopCrouch);

		// Attacking
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AArcher::CallAttack);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Completed, this, &AArcher::CallAttackRelease);

		// Interacting
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AArcher::Interaction);

		// MenuToggle
		EnhancedInputComponent->BindAction(MenuAction, ETriggerEvent::Started, this, &AArcher::ToggleMenuWidget);

		// Hotbar Using
		if (HotbarActions.IsValidIndex(0) && HotbarActions[0])
			EnhancedInputComponent->BindAction(HotbarActions[0], ETriggerEvent::Started, this, &AArcher::SelectHotbar1);
		if (HotbarActions.IsValidIndex(1) && HotbarActions[1])
			EnhancedInputComponent->BindAction(HotbarActions[1], ETriggerEvent::Started, this, &AArcher::SelectHotbar2);
		if (HotbarActions.IsValidIndex(2) && HotbarActions[2])
			EnhancedInputComponent->BindAction(HotbarActions[2], ETriggerEvent::Started, this, &AArcher::SelectHotbar3);
		if (HotbarActions.IsValidIndex(3) && HotbarActions[3])
			EnhancedInputComponent->BindAction(HotbarActions[3], ETriggerEvent::Started, this, &AArcher::SelectHotbar4);
		if (HotbarActions.IsValidIndex(4) && HotbarActions[4])
			EnhancedInputComponent->BindAction(HotbarActions[4], ETriggerEvent::Started, this, &AArcher::SelectHotbar5);
		if (HotbarActions.IsValidIndex(5) && HotbarActions[5])
			EnhancedInputComponent->BindAction(HotbarActions[5], ETriggerEvent::Started, this, &AArcher::SelectHotbar6);
		if (HotbarActions.IsValidIndex(6) && HotbarActions[6])
			EnhancedInputComponent->BindAction(HotbarActions[6], ETriggerEvent::Started, this, &AArcher::SelectHotbar7);
		if (HotbarActions.IsValidIndex(7) && HotbarActions[7])
			EnhancedInputComponent->BindAction(HotbarActions[7], ETriggerEvent::Started, this, &AArcher::SelectHotbar8);
		if (HotbarActions.IsValidIndex(8) && HotbarActions[8])
			EnhancedInputComponent->BindAction(HotbarActions[8], ETriggerEvent::Started, this, &AArcher::SelectHotbar9);

		// Hotbar Scroll (Mouse Wheel)
		if (HotbarScrollAction)
			EnhancedInputComponent->BindAction(HotbarScrollAction, ETriggerEvent::Triggered, this, &AArcher::ScrollHotbar);
	}
	else
	{
		UE_LOG(LogValheim, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AArcher::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (PlayerController)
	{
		PlayerController->AddYawInput(LookAxisVector.X);
		PlayerController->AddPitchInput((-1) * LookAxisVector.Y);
	}
}

void AArcher::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();
	FRotator ControlRotation = GetControlRotation();

	// Get Forward Vector
	FRotator ForwardRotation = FRotator(0.f, ControlRotation.Yaw, ControlRotation.Roll);
	FVector ForwardVector = FRotationMatrix(ForwardRotation).GetUnitAxis(EAxis::X);

	// Get Right Vector
	FRotator RightRotation = FRotator(0.f, ControlRotation.Yaw, ControlRotation.Roll);
	FVector RightVector = FRotationMatrix(RightRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardVector, MovementVector.X);
	AddMovementInput(RightVector, MovementVector.Y);
}

void AArcher::StartCrouch()
{
	Crouch();
}

void AArcher::StopCrouch()
{
	UnCrouch();
}

void AArcher::ToggleMenuWidget()
{
	HUD->ToggleMainWidget();
}

void AArcher::ToggleChatWidget()
{
	HUD->ToggleChatWidget();
}

void AArcher::Interaction()
{
	if (!IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("AArcher::Interaction() !IsLocallyControlled()"));
		return;
	}
	ServerInteraction();
}

void AArcher::ServerInteraction_Implementation()
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PlayerController = Cast<AArcherPC>(PC);
	}
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("AArcher::ServerInteraction() !PlayerController"));
		return;
	}

	float InteractDistance = 1000.0f;
	float InteractRadius = 30.0f;

	FVector CameraLocation;
	FRotator CameraRotation;
	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

	FVector Start = CameraLocation;
	FVector End = Start + CameraRotation.Vector() * InteractDistance;

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FCollisionShape SphereShape = FCollisionShape::MakeSphere(InteractRadius);

	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		Start,
		End,
		FQuat::Identity,
		ECC_WorldDynamic,
		SphereShape,
		Params
	);
	//DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1.f, 0, 1.f);

	if (bHit)
	{
		IInteractable* HitObject = Cast<IInteractable>(HitResult.GetActor());
		if (HitObject)
		{
			HitObject->Interact(this);
			//UE_LOG(LogTemp, Warning, TEXT("Interaction HitObject->Interact(this)"));
		}

		//DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, InteractRadius, 12, FColor::Red, false, 1.f);
		//DrawDebugLine(GetWorld(), Start, HitResult.ImpactPoint, FColor::Red, false, 1.f, 0, 1.f);
	}
}

void AArcher::DropItem(int32 InventoryIndex, const int32 QuantityToDrop)
{
	if (!IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("AArcher::Interaction() !IsLocallyControlled()"));
		return;
	}
	ServerDropItem(InventoryIndex, QuantityToDrop);
}

void AArcher::ServerDropItem_Implementation(int32 InventoryIndex, const int32 QuantityToDrop)
{
	if (!PlayerInventory)
	{
		return;
	}

	const FInventoryItemInstance ItemToDrop = PlayerInventory->GetInventoryItem(InventoryIndex);
	if (!ItemToDrop.IsValidItem())
	{
		UE_LOG(LogTemp, Warning, TEXT("ServerDropItem_Implementation: InventoryIndex %d is empty"), InventoryIndex);
		return;
	}

	const int32 RemovedQuantity = PlayerInventory->RemoveAmountOfItem(InventoryIndex, QuantityToDrop);
	if (RemovedQuantity <= 0)
	{
		return;
	}

	FActorSpawnParameters SpawnParam;
	SpawnParam.Owner = this;
	SpawnParam.bNoFail = true;
	SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	const FVector SpawnLocation{ GetActorLocation() + (GetActorForwardVector() * 50) };
	const FTransform SpawnTransform(GetActorRotation(), SpawnLocation);

	AItemBase* PickUp = GetWorld()->SpawnActor<AItemBase>(AItemBase::StaticClass(), SpawnTransform, SpawnParam);

	if (PickUp)
	{
		PickUp->InitiallizeDrop(ItemToDrop.ItemID, RemovedQuantity);
	}
}

void AArcher::UseItem(int32 InventoryIndex)
{
	if (!IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("AArcher::UseItem() !IsLocallyControlled()"));
		return;
	}
	ServerUseItem(InventoryIndex);
}

void AArcher::UseHotbarItem(int32 HotbarIndex)
{
	if (!IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("AArcher::UseHotbarItem() !IsLocallyControlled()"));
		return;
	}
	ServerUseHotbarItem(HotbarIndex);
}

void AArcher::SetEquipType(EEquipType NewEquipType)
{
	if (!HasAuthority() && !IsLocallyControlled())
	{
		return;
	}
	ServerSetEquipType(NewEquipType);
}

void AArcher::ServerSetEquipType_Implementation(EEquipType NewEquipType)
{
	CurrentEquipType = NewEquipType;
}

void AArcher::ServerUseItem_Implementation(int32 InventoryIndex)
{
	if (!PlayerInventory)
	{
		return;
	}

	const FInventoryItemInstance ItemToUse = PlayerInventory->GetInventoryItem(InventoryIndex);
	if (!ItemToUse.IsValidItem())
	{
		return;
	}

	UItemPrimaryDataAsset* ItemData = nullptr;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UItemSubsystem* Subsystem = GI->GetSubsystem<UItemSubsystem>())
		{
			Subsystem->GetItemData(ItemToUse.ItemID, ItemData);
		}
	}

	if (!ItemData || ItemData->ItemCategory.ItemCategory != EItemCategory::Consumable)
	{
		return;
	}

	if (GetCurrentHP() <= 0)
	{
		return;
	}

	if (AArcherPS* ArcherPS = GetPlayerState<AArcherPS>())
	{
		ArcherPS->UpdateQuestProgressByEvent(EQuestType::UseItem, ItemToUse.ItemID, 1);
	}

	switch (ItemData->ItemCategory.ItemType)
	{
	case EItemType::Heal:
		AddHP(ItemData->ItemCategory.Value);
		break;
	case EItemType::Damage:
		AddHP(-1 * ItemData->ItemCategory.Value);
		break;
	default:
		break;
	}

	PlayerInventory->RemoveAmountOfItem(InventoryIndex, 1);
}

void AArcher::ServerUseHotbarItem_Implementation(int32 HotbarIndex)
{
	if (!PlayerInventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("AArcher::ServerUseHotbarItem !PlayerInventory"));
		return;
	}

	const FInventoryItemInstance ItemToUse = PlayerInventory->GetHotbarItem(HotbarIndex);
	if (!ItemToUse.IsValidItem())
	{
		UE_LOG(LogTemp, Warning, TEXT("AArcher::ServerUseHotbarItem !ItemToUse.IsValidItem()"));
		return;
	}

	UItemPrimaryDataAsset* ItemData = nullptr;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UItemSubsystem* Subsystem = GI->GetSubsystem<UItemSubsystem>())
		{
			Subsystem->GetItemData(ItemToUse.ItemID, ItemData);
		}
	}

	if (!ItemData || ItemData->ItemCategory.ItemCategory != EItemCategory::Consumable)
	{
		return;
	}

	if (GetCurrentHP() <= 0)
	{
		return;
	}

	if (AArcherPS* ArcherPS = GetPlayerState<AArcherPS>())
	{
		ArcherPS->UpdateQuestProgressByEvent(EQuestType::UseItem, ItemToUse.ItemID, 1);
	}

	switch (ItemData->ItemCategory.ItemType)
	{
	case EItemType::Heal:
		AddHP(ItemData->ItemCategory.Value);
		break;
	case EItemType::Damage:
		AddHP(-1 * ItemData->ItemCategory.Value);
		break;
	default:
		break;
	}

	PlayerInventory->RemoveAmountOfHotbarItem(HotbarIndex, 1);
}

void AArcher::SetHP(float NewHP)
{
	HP = FMath::Clamp((HP + NewHP), 0, MaxHP);
	OnHPChanged.Broadcast();
}

void AArcher::CallAttackCollision()
{
	if (!IsLocallyControlled())
	{
		return;
	}
	ServerCallAttackCollision();
}

void AArcher::ServerCallAttackCollision_Implementation()
{
	TArray<AActor*> HitActors;
	TArray<FHitResult> OutHits;
	FVector SpawnLocation = GetActorLocation() + (GetActorForwardVector() * 110.f);
	float SphereRadius = 50.0f;
	FCollisionShape MySphere = FCollisionShape::MakeSphere(SphereRadius);
	bool bIsHit = GetWorld()->SweepMultiByChannel(
		OutHits,
		SpawnLocation,
		SpawnLocation,
		FQuat::Identity,
		ECC_WorldDynamic,
		MySphere);

	if (bIsHit)
	{
		for (const FHitResult& Hit : OutHits)
		{
			AActor* HitActor = Hit.GetActor();
			if (AMonster* Monster = Cast<AMonster>(HitActor))
			{
				if (!HitActors.Contains(HitActor))
				{
					HitActors.Add(HitActor);
					//float Damage;

					UGameplayStatics::ApplyDamage(
						Monster,
						GetDamageValue(),
						GetController(),
						this,
						UDamageType::StaticClass()
					);
				}
			}
		}
	}

	//DrawDebugSphere(GetWorld(), SpawnLocation, SphereRadius, 12, FColor::Red, false, 1.f);
}

void AArcher::CallAttack()
{
	if (bIsAttacking)
	{
		return;
	}

	switch (GetEquipType())
	{
	case EEquipType::Bow:
		StartDrawBow();
		break;
	case EEquipType::Sword:
		ServerAttack();
		break;
	case EEquipType::None:
		ServerAttack();
		break;
	}
}

void AArcher::ServerAttack_Implementation()
{
	MultiAttack();
}

void AArcher::MultiAttack_Implementation()
{
	if (AttackMontage)
	{
		PlayAnimMontage(AttackMontage);
	}
}

void AArcher::CallAttackRelease()
{
	if (GetEquipType() == EEquipType::Bow && GetIsDrawing())
	{
		ReleaseDrawBow();
	}
}

void AArcher::StartDrawBow()
{
	if (GetInventory()->FindInventoryIndexByID("Arrow") < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("AArcher::StartDrawBow GetInventory()->FindInventoryIndexByID Null"));
		return;
	}

	SetIsDrawing(true);
	GetCharacterMovement()->MaxWalkSpeed = DrawingWalkSpeed;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = true;
}

void AArcher::ReleaseDrawBow()
{
	if (GetIsDrawing())
	{
		ServerRecoil();
	}
	SetIsDrawing(false);
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
}

void AArcher::ServerRecoil_Implementation()
{
	if (!PlayerController) return;

	FVector CameraLocation;
	FRotator CameraRotation;
	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

	FVector TraceStart = CameraLocation;
	FVector TraceEnd = TraceStart + CameraRotation.Vector() * 10000.f;

	FHitResult HitResult;
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(this);

	FVector AimPoint = TraceEnd;
	if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, TraceParams))
	{
		AimPoint = HitResult.ImpactPoint;
	}

	FTransform SocketTransform = BowMesh->GetSocketTransform(TEXT("LeftHandSocket"));
	FVector SocketLocation = SocketTransform.GetLocation();

	FVector FireDirection = (AimPoint - SocketLocation).GetSafeNormal();
	FRotator SpawnRotation = FireDirection.Rotation();
	FVector SpawnLocation = SocketLocation + FireDirection * 50.f;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	AArrow* Arrow = GetWorld()->SpawnActor<AArrow>(ArrowClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (Arrow)
	{
		Arrow->InitializeArrow(GetDamageValue(), GetController(), this);
	}
	;
	PlayerInventory->RemoveAmountOfItem(PlayerInventory->FindInventoryIndexByID("Arrow"), 1);
	MultiRecoil();
}

void AArcher::MultiRecoil_Implementation()
{
	if (RecoilMontage)
	{
		PlayAnimMontage(RecoilMontage);
	}
}

void AArcher::EquipWeapon(EItemType WeaponType)
{
	UnequipAllWeapon();

	if (WeaponType == EItemType::Sword)
	{
		SetEquipType(EEquipType::Sword);
		SetVisiblityMesh(EMeshType::Sword, true);
	}
	else if (WeaponType == EItemType::Bow)
	{
		SetEquipType(EEquipType::Bow);
		SetVisiblityMesh(EMeshType::Bow, true);
	}
}

void AArcher::UnequipAllWeapon()
{
	SetVisiblityMesh(EMeshType::Sword, false);
	SetVisiblityMesh(EMeshType::Bow, false);
	SetEquipType(EEquipType::None);
}

void AArcher::SetVisiblityMesh(EMeshType MeshType, bool OnOff)
{
	if (!IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("AArcher::SetVisiblityMesh !IsLocallyControlled()"));
		return;
	}
	ServerSetVisiblityMesh(MeshType, OnOff);
}

void AArcher::ServerSetVisiblityMesh_Implementation(EMeshType MeshType, bool OnOff)
{
	MultiSetVisiblityMesh(MeshType, OnOff);
}

void AArcher::MultiSetVisiblityMesh_Implementation(EMeshType MeshType, bool OnOff)
{
	switch (MeshType)
	{
	case EMeshType::Arrow:
		ArrowMesh->SetVisibility(OnOff);
		break;
	case EMeshType::Sword:
		SwordMesh->SetVisibility(OnOff);
		break;
	case EMeshType::Bow:
		BowMesh->SetVisibility(OnOff);
		break;
	}
}

void AArcher::OnRep_HP()
{
	OnHPChanged.Broadcast();
}

float AArcher::GetDamageValue()
{
	switch (CurrentEquipType)
	{
	case EEquipType::None:
		return 5;
		break;
	case EEquipType::Sword:
		return 10;
		break;
	case EEquipType::Bow:
		return 15;
		break;
	default:
		return 5;
		break;
	}
}

void AArcher::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AArcher, bIsAttacking);
	DOREPLIFETIME(AArcher, HP);
	DOREPLIFETIME(AArcher, CurrentEquipType);
}

float AArcher::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	float InDamage = (-1) * DamageAmount;
	SetHP(InDamage);
	return DamageAmount;
}

void AArcher::SelectHotbar1() { SetActiveHotbarIndex(0); }
void AArcher::SelectHotbar2() { SetActiveHotbarIndex(1); }
void AArcher::SelectHotbar3() { SetActiveHotbarIndex(2); }
void AArcher::SelectHotbar4() { SetActiveHotbarIndex(3); }
void AArcher::SelectHotbar5() { SetActiveHotbarIndex(4); }
void AArcher::SelectHotbar6() { SetActiveHotbarIndex(5); }
void AArcher::SelectHotbar7() { SetActiveHotbarIndex(6); }
void AArcher::SelectHotbar8() { SetActiveHotbarIndex(7); }
void AArcher::SelectHotbar9() { SetActiveHotbarIndex(8); }

void AArcher::ScrollHotbar(const FInputActionValue& Value)
{
	const float ScrollAxis = Value.Get<float>();
	if (FMath::IsNearlyZero(ScrollAxis))
	{
		return;
	}

	constexpr int32 SlotCount = 9;

	const int32 Direction = ScrollAxis > 0.f ? 1 : -1;
	const int32 NewIndex = (ActiveHotbarIndex + Direction + SlotCount) % SlotCount;

	SetActiveHotbarIndex(NewIndex);
}

void AArcher::SetActiveHotbarIndex(int32 NewIndex)
{
	ActiveHotbarIndex = NewIndex;
	RefreshActiveHotbarEquip();
	OnActiveHotbarChanged.Broadcast();

	if (!HasAuthority() && IsLocallyControlled())
	{
		ServerSetActiveHotbarIndex(NewIndex);
	}
}

void AArcher::ServerSetActiveHotbarIndex_Implementation(int32 NewIndex)
{
	ActiveHotbarIndex = NewIndex;
	RefreshActiveHotbarEquip();
}

void AArcher::RefreshActiveHotbarEquip()
{
	if (!PlayerInventory)
	{
		return;
	}

	if (!HasAuthority() && !IsLocallyControlled())
	{
		return;
	}

	const FInventoryItemInstance Item = PlayerInventory->GetHotbarItem(ActiveHotbarIndex);
	if (!Item.IsValidItem())
	{
		UnequipAllWeapon();
		return;
	}

	UItemPrimaryDataAsset* ItemData = nullptr;

	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const UItemSubsystem* ItemSubsystem = GI->GetSubsystem<UItemSubsystem>())
		{
			ItemSubsystem->GetItemData(Item.ItemID, ItemData);
		}
	}

	if (ItemData && ItemData->ItemCategory.ItemCategory == EItemCategory::Weapon)
	{
		EquipWeapon(ItemData->ItemCategory.ItemType);
	}
	else
	{
		UnequipAllWeapon();
	}
}