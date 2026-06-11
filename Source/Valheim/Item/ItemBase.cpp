// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemBase.h"
#include "Components/BoxComponent.h"
#include "ItemDataBase.h"

// Sets default values
AItemBase::AItemBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	SetRootComponent(BoxCollision);

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(BoxCollision);
}

// Called when the game starts or when spawned
void AItemBase::BeginPlay()
{
	Super::BeginPlay();

	ItemData = ItemDataRowHandle.GetRow<FItemBaseRow>(TEXT("ItemBase"));
}

// Called every frame
void AItemBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

