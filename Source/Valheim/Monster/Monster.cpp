// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Monster.h"
#include "Kismet/GameplayStatics.h"
#include "Character/Archer.h"
#include "Monster/MonsterAIController.h"

// Sets default values
AMonster::AMonster()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMonster::BeginPlay()
{
	Super::BeginPlay();

	HP = MaxHP;

	MonsterAIController = Cast<AMonsterAIController>(GetController());
	
}

// Called every frame
void AMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMonster::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

float AMonster::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	HP -= DamageAmount;
	UE_LOG(LogTemp, Warning, TEXT("Monster HP: %f"), HP);

	if (HP <= 0)
	{
		if (!MonsterAIController)
		{
			return DamageAmount;
		}
		UE_LOG(LogTemp, Warning, TEXT("MonsterAIController"));
		MonsterAIController->StopAI();
		CallDeathAnimation();
	}

	return DamageAmount;
}

void AMonster::CallAttack_Implementation()
{
}

void AMonster::CallAttackCollision()
{
	ServerCallAttackCollision();
}

void AMonster::EndDeath()
{
	Server_EndDeath();
}

void AMonster::Server_EndDeath_Implementation()
{
	Destroy();
}

void AMonster::ServerCallAttackCollision_Implementation()
{
	TArray<AActor*> HitActors;
	TArray<FHitResult> OutHits;
	FVector SpawnLocation = GetActorLocation() + (GetActorForwardVector() * 150.f);
	float SphereRadius = 60.0f;
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
			if (AArcher* Archer = Cast<AArcher>(HitActor))
			{
				if (!HitActors.Contains(HitActor))
				{
					HitActors.Add(HitActor);
					UGameplayStatics::ApplyDamage(
						Archer,
						10.f,
						GetController(),
						this,
						UDamageType::StaticClass() // 데미지 타입
					);
				}
			}
		}
	}

	DrawDebugSphere(GetWorld(), SpawnLocation, SphereRadius, 12, FColor::Magenta, false, 1.f);
}

void AMonster::CallDeathAnimation_Implementation()
{
}