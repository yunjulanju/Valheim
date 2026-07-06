// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/Session/SessionList.h"
#include "UserInterface/Session/SessionItem.h"
#include "Components/WrapBox.h"
#include "Components/Button.h"

bool USessionList::Initialize()
{
	if (Super::Initialize() == false)
		return false;

	SetInfo();

	return true;
}

void USessionList::SetInfo()
{
	// Child UI 만들어진거 비우기 
	Sessions.Empty();

	// Child UI 미리 틀만 10개 만들어 두기
	for (int i = 0; i < 10; i++)
	{
		// 생성할 UI 클래스 레퍼런스가 없으면 넘기기
		if (SessionItemClass == nullptr)
			continue;

		// Child UI 생성
		USessionItem* ChildWidget = CreateWidget<USessionItem>(GetWorld(), SessionItemClass);

		// 제대로 생성됬는지 검증
		if (ChildWidget == nullptr)
			continue;

		// WrapBox(UI)에 차일드로 붙이기
		SessionList->AddChildToWrapBox(ChildWidget);

		// 데이터로(인스턴스)써 만들어진 차일드를 들고 있기
		Sessions.Add(ChildWidget);
	}

	// UI 갱신
	RefreshUI();
}

void USessionList::RefreshUI()
{
	// 세션(방)이 몇개 있는지 확인
	const int32 SessionLength = SessionInfos.Num();

	for (int32 i = 0; i < Sessions.Num(); i++)
	{
		const int32 index = i;

		// 차일드 UI 켜주기
		if (index < SessionLength)
		{
			Sessions[index]->SetVisibility(ESlateVisibility::Visible);
			FBlueprintSessionResult Result = SessionInfos[index];
			// 세션정보 넘겨줘서 차일드 위젯에 데이터 채우기
			Sessions[index]->SetInfo(Result);

		}
		// 차일드 UI 꺼주기
		else
		{
			Sessions[index]->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

}