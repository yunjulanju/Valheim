// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Sword.h"

void ASword::PickItem()
{
	UE_LOG(LogTemp, Warning, TEXT("PickItem"));
	Destroy();
}
