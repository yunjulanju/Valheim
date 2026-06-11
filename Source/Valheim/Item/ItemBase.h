// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/ItemDataStruct.h"
#include "ItemBase.generated.h"


UCLASS()
class VALHEIM_API AItemBase : public AActor
{
	GENERATED_BODY()

    //------------------------------Function ////
public:	
	// Sets default values for this actor's properties
	AItemBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

    //------------------------------Property ////

    //Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDataTableRowHandle ItemDataRowHandle;
    FItemBaseRow* ItemData = nullptr;

    //Item
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    int32 Quantity = 1;

    //Component
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    UStaticMeshComponent* ItemMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    class UBoxComponent* BoxCollision;


};
