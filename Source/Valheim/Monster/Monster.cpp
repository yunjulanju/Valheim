// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Monster.h"
#include "Kismet/GameplayStatics.h"
#include "Character/Archer.h"
#include "Monster/MonsterAIController.h"
#include <Character/ArcherPS.h>


AMonster::AMonster()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMonster::BeginPlay()
{
	Super::BeginPlay();

	HP = MaxHP;

	MonsterAIController = Cast<AMonsterAIController>(GetController());
	
}

void AMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMonster::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

float AMonster::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	HP -= DamageAmount;
	//UE_LOG(LogTemp, Warning, TEXT("Monster HP: %f"), HP);

	if (HP <= 0)
	{	
		AArcher* Causer = Cast<AArcher>(DamageCauser);
		if (Causer)
		{
			AArcherPS* ArcherPS = Causer->GetPlayerState<AArcherPS>();
			if (ArcherPS)
			{
				ArcherPS->UpdateQuestProgressByEvent(EQuestType::Kill, MonsterID, 1);
			}
		}

		if (!MonsterAIController)
		{
			return DamageAmount;
		}
		MonsterAIController->StopAI();
		CallDeathAnimation();
	}

	return DamageAmount;
}

void AMonster::CallAttackCollision()
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
void AMonster::CallAttack_Implementation()
{
}

void AMonster::EndDeath()
{
	Server_EndDeath();
}

void AMonster::Server_EndDeath_Implementation()
{
	Destroy();
}