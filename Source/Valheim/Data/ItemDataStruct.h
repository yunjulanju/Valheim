#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemDataStruct.generated.h" 

class AItemBase;

UENUM(BlueprintType)
enum class EItemCategory : uint8
{
    Weapon      UMETA(DisplayName = "Weapon"),
    Consumable  UMETA(DisplayName = "Consumable"),
    Etc         UMETA(DisplayName = "Etc")
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
    TSubclassOf<AItemBase> ItemClass;
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
    EItemCategory ItemCategory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FItemAssetData AssetData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FItemNumericData NumericData;
};