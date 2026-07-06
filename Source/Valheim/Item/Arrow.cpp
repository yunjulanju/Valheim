// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Arrow.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Monster/Monster.h"

AArrow::AArrow()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetBoxExtent(FVector(5.f, 5.f, 5.f));
	CollisionBox->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionBox->SetNotifyRigidBodyCollision(true);
	SetRootComponent(CollisionBox);

	ArrowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowMesh"));
	ArrowMesh->SetupAttachment(CollisionBox);
	ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArrowMesh->SetIsReplicated(true);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionBox;
	ProjectileMovement->InitialSpeed = 6000;
	ProjectileMovement->MaxSpeed = 6000;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 1.0f;

	InitialLifeSpan = 10.0f;
}

void AArrow::InitializeArrow(float InDamage, AController* InInstigator, AActor* InDamageCauser)
{
	Damage = InDamage;
	InstigatorController = InInstigator;
	DamageCauserActor = InDamageCauser;
}

void AArrow::BeginPlay()
{
	Super::BeginPlay();
	CollisionBox->OnComponentHit.AddDynamic(this, &AArrow::OnHit);
}

void AArrow::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bHasHit || !OtherActor || OtherActor == this)
	{
		return;
	}

	bHasHit = true;

	if (AMonster* Monster = Cast<AMonster>(OtherActor))
	{
		UGameplayStatics::ApplyDamage(
			Monster,
			Damage,
			InstigatorController,
			DamageCauserActor,
			UDamageType::StaticClass()
		);
	}
	
	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->ProjectileGravityScale = 0.f;
	CollisionBox->SetSimulatePhysics(false);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SetLifeSpan(LifeSpanAfterHit);
}

void AArrow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

