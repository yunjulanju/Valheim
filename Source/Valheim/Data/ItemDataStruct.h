#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Net/Serialization/FastArraySerializer.h" 
#include "ItemDataStruct.generated.h" 

class UItemPrimaryDataAsset;


UENUM(BlueprintType)
enum class EItemCategory : uint8
{
    Weapon      UMETA(DisplayName = "Weapon"),
    Consumable  UMETA(DisplayName = "Consumable"),
    Etc         UMETA(DisplayName = "Etc")
};

UENUM(BlueprintType)
enum class EItemType : uint8
{
    Heal      UMETA(DisplayName = "Heal"),
    Damage  UMETA(DisplayName = "Damage"),
    Defense       UMETA(DisplayName = "Defense"),
    Sword  UMETA(DisplayName = "Sword"),
    Bow  UMETA(DisplayName = "Bow"),
    Arrow  UMETA(DisplayName = "Arrow")
};

USTRUCT(BlueprintType)
struct FItemCategory : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EItemCategory ItemCategory = EItemCategory::Etc;

    //Consumable type
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EItemType ItemType = EItemType::Heal;

    //type Value
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Value = 0.f;
};

USTRUCT(BlueprintType)
struct FItemTextData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;
};

USTRUCT(BlueprintType)
struct FItemAssetData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* ItemImage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* ItemMesh = nullptr;
};

USTRUCT(BlueprintType)
struct FItemNumericData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int MaxStackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsStackable = false;
};

USTRUCT(BlueprintType)
struct FItemBaseRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName  ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FItemTextData TextData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FItemCategory ItemCategory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FItemAssetData AssetData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FItemNumericData NumericData;
};


USTRUCT(BlueprintType)
struct FInventoryItemInstance : public FFastArraySerializerItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quantity = 0;

    UPROPERTY(Transient, NotReplicated)
    TObjectPtr<UItemPrimaryDataAsset> CachedItemData = nullptr;

    bool IsValidItem() const { return ItemID != NAME_None; }

    // 선언만 (구현은 .cpp로 이동)
    bool IsStackable() const;
    bool IsFullStack() const;

    void PostReplicatedAdd(const struct FInventoryList& InArraySerializer);
    void PostReplicatedChange(const struct FInventoryList& InArraySerializer);
    void PreReplicatedRemove(const struct FInventoryList& InArraySerializer);
};

USTRUCT(BlueprintType)
struct FInventoryList : public FFastArraySerializer
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<FInventoryItemInstance> Items;

    // 리플리케이트 안 되는 로컬 전용 포인터. 콜백 안에서 GetWorld() 등에 접근하려고 둠.
    class UInventoryComponent* OwningComponent = nullptr;

    bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
    {
        return FastArrayDeltaSerialize<FInventoryItemInstance>(Items, DeltaParms, *this);
    }
};

template<>
struct TStructOpsTypeTraits<FInventoryList> : public TStructOpsTypeTraitsBase2<FInventoryList>
{
    enum { WithNetDeltaSerializer = true };
};