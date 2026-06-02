// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Archer.h"
#include "Character/ArcherPC.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Valheim.h"
#include "Kismet/KismetMathLibrary.h"

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
	CameraBoom->TargetArmLength = 300.0f;
	CameraBoom->SocketOffset = FVector(0,0,80);
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

// Called when the game starts or when spawned
void AArcher::BeginPlay()
{
	Super::BeginPlay();
	
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PlayerController = Cast<AArcherPC>(PC);
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
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
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AArcher::CallAttack);
		
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

void AArcher::CallAttack()
{
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