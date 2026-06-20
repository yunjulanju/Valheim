// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Archer.h"
#include "Character/ArcherPC.h"
#include "GameFramework/Controller.h"
#include "Valheim.h"
#include "Net/UnrealNetwork.h" 

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "Kismet/GameplayStatics.h"
#include <Monster/Monster.h>
#include <Item/ItemBase.h>
#include <Item/Sword.h>
#include "Inventory/InventoryComponent.h"
#include "UserInterface/ArcherHUD.h"
#include "Item/ItemDataBase.h"


// Sets default values
AArcher::AArcher()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->SocketOffset = FVector(0,0,100);
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bDoCollisionTest = false;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	SwordMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwordMesh"));
	SwordMesh->SetupAttachment(GetMesh(), FName("RightHandSocket"));
	SwordMesh->SetVisibility(false);

	HP = MaxHP;

	PlayerInventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
	PlayerInventory->SetSlotsCapacity(20);

}

// Called when the game starts or when spawned
void AArcher::BeginPlay()
{
	Super::BeginPlay();
	
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PlayerController = Cast<AArcherPC>(PC);
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			// IMC null 체크 필수
			if (Subsystem)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
		HUD = Cast<AArcherHUD>(PC->GetHUD());
	}
}

// Called every frame
void AArcher::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
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

		// Interacting
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AArcher::Interaction);

		// MenuToggle
		EnhancedInputComponent->BindAction(MenuAction
			, ETriggerEvent::Started, this, &AArcher::ToggleMenuWidget);
		
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

void AArcher::Interaction()
{
	if (bEquipWeapon)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	float InteractDistance = 1000.0f;
	FVector CameraLocation;
	FRotator CameraRotation;
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	FVector Start = CameraLocation;
	FVector End = Start + CameraRotation.Vector() * InteractDistance;

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this); // 자기 자신 무시

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_WorldDynamic
	);

	DrawDebugLine(
		GetWorld(),Start,End,FColor::Green,false,1.f,0,1.f);

	if (bHit)
	{
		AItemBase* HitItem = Cast<AItemBase>(HitResult.GetActor());
		if (HitItem)
		{
			HitItem->TakePickUp(this);
		}

		//DebugLine
		DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 10.f, FColor::Red, false, 1.f);
		DrawDebugLine(GetWorld(), Start, HitResult.ImpactPoint, FColor::Red, false, 1.f, 0, 1.f);
	}

}

void AArcher::AttachWeapon(ASword* Sword)
{
	if (!Sword || !GetMesh())
	{
		return;
	}

	/*Sword->ItemMesh->SetSimulatePhysics(false);
	Sword->ItemMesh->AttachToComponent(GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		FName("RightHandSocket"));*/


	Sword->PickItem();
}

void AArcher::CallAttackCollision()
{
	ServerCallAttackCollision();
}

void AArcher::DropItem(UItemDataBase* ItemToDrop, const int32 QuantityToDrop)
{
	if (PlayerInventory->FindMatchingItem(ItemToDrop))
	{
		FActorSpawnParameters SpawnParam;
		SpawnParam.Owner = this;
		SpawnParam.bNoFail = true;
		SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		const FVector SpawnLocation{ GetActorLocation() + (GetActorForwardVector() * 50) };
		const FTransform SpawnTransform(GetActorRotation(), SpawnLocation);

		const int32 RemovedQuantity = PlayerInventory->RemoveAmountOfItem(ItemToDrop, QuantityToDrop);

		AItemBase* PickUp = GetWorld()->SpawnActor<AItemBase>(AItemBase::StaticClass(), SpawnTransform, SpawnParam);

		if (PickUp)
		{
			PickUp->InitiallizeDrop(ItemToDrop, RemovedQuantity);
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Item to drop was FindMatchingItem Null"));
	}
}

void AArcher::SetHP(float NewHP)
{
	HP = FMath::Clamp((HP+NewHP), 0, MaxHP);
	OnHPChanged.Broadcast();
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
					float Damage;
					if (bEquipWeapon)
					{
						Damage = DefaultDamage * 2;
					}
					else
					{
						Damage = DefaultDamage;
					}
					//UE_LOG(LogTemp, Warning, TEXT("Hit Damage: %f"), Damage);
					UGameplayStatics::ApplyDamage(
						Monster,
						Damage,
						GetController(),
						this,
						UDamageType::StaticClass() // 데미지 타입
					);
				}
			}
		}
	}

	DrawDebugSphere(GetWorld(), SpawnLocation, SphereRadius, 12, FColor::Red, false, 1.f);
}

void AArcher::CallAttack()
{
	if (bIsAttacking)
	{
		return;
	}
	ServerAttack();
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


void AArcher::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(AArcher, bIsAttacking);
}

float AArcher::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HP -= DamageAmount;

	return DamageAmount;
}
