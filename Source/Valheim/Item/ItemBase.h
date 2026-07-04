// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/ItemDataStruct.h"
#include "Interface/Interactable.h"
#include "ItemBase.generated.h"

class AArcher;

UCLASS()
class VALHEIM_API AItemBase : public AActor, public IInteractable
{
    GENERATED_BODY()

    //------------------------------Function ////
public:
    // Sets default values for this actor's properties
    AItemBase();

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    void InitializeItem(const int32 InQuantity);

    // 필드에서 드롭될 때 호출: 어떤 아이템인지는 이제 UObject가 아니라 ItemID로 전달받음
    void InitiallizeDrop(FName InItemID, const int32 InQuantity);

    FORCEINLINE FName GetItemID() const { return ItemID; }
    FORCEINLINE int32 GetQuantity() const { return Quantity; }

    UFUNCTION()
    void TakePickUp(const AArcher* Taker);

    virtual void Interact(APawn* Interactor) override;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // 서버에서 CurrentMeshAsset이 바뀔 때마다 클라에도 알려서 실제 SetStaticMesh를 호출시킴
    // (SetStaticMesh()는 함수 호출이라 그 자체는 리플리케이트되지 않기 때문)
    UFUNCTION()
    void OnRep_CurrentMeshAsset();

    // ItemID로 인벤토리에 넣기를 시도하고, 성공하면 true (호출부에서 Destroy 여부 결정)
    bool TryAddToInventory(AArcher* Taker);

    //------------------------------Property ////

protected:

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UItemPrimaryDataAsset* DesiredItemAsset;

    UPROPERTY(Replicated, BlueprintReadOnly)
    FName ItemID = NAME_None;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Replicated)
    int32 Quantity = 1;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentMeshAsset)
    class UStaticMesh* CurrentMeshAsset = nullptr;

    //Component
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    UStaticMeshComponent* ItemMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    class UBoxComponent* BoxCollision;
};