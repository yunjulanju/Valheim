// Fill out your copyright notice in the Description page of Project Settings.

#include "Misc/AutomationTest.h"
#include "AINPC/AINpcSubsystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAINpcSubsystemBuildRequestTest, "Valheim.AINpc.BuildChatRequestJson", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAINpcSubsystemBuildRequestTest::RunTest(const FString& Parameters)
{
	const FString Json = UAINpcSubsystem::BuildChatRequestJson(TEXT("playerA"), TEXT("guard"), TEXT("hello"));

	TestTrue(TEXT("contains type chat"), Json.Contains(TEXT("\"type\":\"chat\"")));
	TestTrue(TEXT("contains player_id"), Json.Contains(TEXT("\"player_id\":\"playerA\"")));
	TestTrue(TEXT("contains npc_id"), Json.Contains(TEXT("\"npc_id\":\"guard\"")));
	TestTrue(TEXT("contains message"), Json.Contains(TEXT("\"message\":\"hello\"")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAINpcSubsystemParseReplyTest, "Valheim.AINpc.ParseServerMessage.ChatReply", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAINpcSubsystemParseReplyTest::RunTest(const FString& Parameters)
{
	const FString Input = TEXT("{\"type\":\"chat_reply\",\"player_id\":\"playerA\",\"npc_id\":\"guard\",\"reply\":\"welcome\"}");

	FString OutType, OutPlayerId, OutNpcId, OutPayload;
	const bool bOk = UAINpcSubsystem::ParseServerMessage(Input, OutType, OutPlayerId, OutNpcId, OutPayload);

	TestTrue(TEXT("parse succeeds"), bOk);
	TestEqual(TEXT("type"), OutType, FString(TEXT("chat_reply")));
	TestEqual(TEXT("player_id"), OutPlayerId, FString(TEXT("playerA")));
	TestEqual(TEXT("npc_id"), OutNpcId, FString(TEXT("guard")));
	TestEqual(TEXT("reply payload"), OutPayload, FString(TEXT("welcome")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAINpcSubsystemParseErrorTest, "Valheim.AINpc.ParseServerMessage.Error", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAINpcSubsystemParseErrorTest::RunTest(const FString& Parameters)
{
	const FString Input = TEXT("{\"type\":\"error\",\"player_id\":\"playerA\",\"npc_id\":\"guard\",\"error\":\"npc_not_found\"}");

	FString OutType, OutPlayerId, OutNpcId, OutPayload;
	const bool bOk = UAINpcSubsystem::ParseServerMessage(Input, OutType, OutPlayerId, OutNpcId, OutPayload);

	TestTrue(TEXT("parse succeeds"), bOk);
	TestEqual(TEXT("type"), OutType, FString(TEXT("error")));
	TestEqual(TEXT("error payload"), OutPayload, FString(TEXT("npc_not_found")));

	return true;
}
