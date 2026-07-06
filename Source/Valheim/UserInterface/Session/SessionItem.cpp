// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/Session/SessionItem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void USessionItem::SetInfo(FBlueprintSessionResult InSessionResult)
{
	Result = InSessionResult;
	RefreshUI();
}

void USessionItem::RefreshUI()
{
	// 방이름 담아줄 변수
	FString SessionName;
	// 세션세팅에 넣어놨던 방이름 가져오기
	Result.OnlineResult.Session.SessionSettings.Get("SESSION_NAME", OUT SessionName);
	// 방이름 설정하기
	Txt_SessionName->SetText(FText::FromString(SessionName));

	// 방이름 담아줄 변수
	FString MapName;
	// 세션세팅에 넣어놨던 방이름 가져오기
	Result.OnlineResult.Session.SessionSettings.Get("MAP_NAME", OUT MapName);
	// 방이름 설정하기
	Txt_MapName->SetText(FText::FromString(MapName));

	// 현재 참여인원 구하기
	const int32 currentPlayers = Result.OnlineResult.Session.SessionSettings.NumPublicConnections - Result.OnlineResult.Session.NumOpenPublicConnections;
	// 맥스 플레이어 구하기
	const int32 MaxPlayers = Result.OnlineResult.Session.SessionSettings.NumPublicConnections;
	// 텍스트 합쳐주기 
	FString PlayerCountString = FString::Printf(TEXT("(%d / %d)"), currentPlayers, MaxPlayers);
	// 플레이어카운트 텍스트블록 채워주기
	Txt_PlayerCount->SetText(FText::FromString(PlayerCountString)); // (1/10)
}