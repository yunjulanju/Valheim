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
    EItemCategory ItemCategory;

    //Consumable type
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EItemType ItemType;

    //type Value
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Value;
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
    UTexture2D* ItemImage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* ItemMesh;
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

    // 위젯이 그릴 아이콘/이름/스택가능여부 등은 전부 이걸로 조회
    UPROPERTY(Transient, NotReplicated)
    TObjectPtr<UItemPrimaryDataAsset> CachedItemData = nullptr;

    bool IsValidItem() const { return ItemID != NAME_None; }
    bool IsStackable() const { return CachedItemData && CachedItemData->NumericData.bIsStackable; }
    bool IsFullStack() const { return CachedItemData && Quantity >= CachedItemData->NumericData.MaxStackSize; }

    // FastArraySerializer가 요구하는 콜백 (클라에서 항목이 바뀔 때 실행됨)
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