#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemDataStruct.generated.h" 

class UItemDataBase;

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
    Defense       UMETA(DisplayName = "Defense")
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
    int MaxStackSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsStackable;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<UItemDataBase> ItemDataClass;
};