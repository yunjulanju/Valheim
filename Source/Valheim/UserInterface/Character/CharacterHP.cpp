// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/Character/CharacterHP.h"
#include "Character/Archer.h"
#include "Components/TextBlock.h"
#include "Components/Progressbar.h"


void UCharacterHP::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	PlayerCharacter = Cast<AArcher>(GetOwningPlayerPawn());
	if (PlayerCharacter)
	{
		if (HPText)
		{
			HPText->SetText(FText::AsNumber(PlayerCharacter->GetCurrentHP()));
		}
		if (HPBar)
		{
			HPBar->SetPercent(PlayerCharacter->GetCurrentPercentHP());
		}
	}
}
