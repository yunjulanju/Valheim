// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/ItemDataStruct.h"
#include "ItemBase.generated.h"


class UItemDataBase;
class AArcher;
UCLASS()
class VALHEIM_API AItemBase : public AActor
{
	GENERATED_BODY()

    //------------------------------Function ////
public:	
	// Sets default values for this actor's properties
	AItemBase();

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    void InitializeItem(const int32 InQuantity);

    void InitiallizeDrop(UItemDataBase* ItemToDrop, const int32 InQuantity);

    FORCEINLINE UItemDataBase* GetItemData() { return ItemReference; };

    UFUNCTION()
    void TakePickUp(const AArcher* Taker);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif #if WITH_EDITOR

    //------------------------------Property ////

protected:

    //Data
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
    UDataTable* ItemDataTable;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
    FName DesiredItemID;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
    class UItemDataBase* ItemReference;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
    int32 Quantity = 1;

    //Component
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    UStaticMeshComponent* ItemMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    class UBoxComponent* BoxCollision;


};
