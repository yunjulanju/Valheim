# 대화형 AI NPC (언리얼 클라이언트) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 리슨서버 호스트가 AI NPC 서버(`ws://<host>:8765/ws`)와 WebSocket으로 대화할 수 있는 언리얼 클라이언트 기능을 추가한다 (v1: host-only, 클라이언트 릴레이 없음).

**Architecture:** `UAINpcSubsystem`(GameInstanceSubsystem)이 WebSocket 연결을 단일 소유하고 JSON 요청/응답을 처리한다. `ANPC`에 `bAIEnabled` 플래그를 추가해 `Interact()`에서 AI 대화 분기를 타면 `UNPCDialogueWidget`(신규 UMG C++ 베이스, 기존 `UChatEntry` 재사용)을 띄운다. 기존 퀘스트 NPC 로직·채팅 시스템은 건드리지 않는다.

**Tech Stack:** Unreal Engine 5.7, C++, WebSockets/Json/JsonUtilities 모듈, Unreal Automation Test 프레임워크(신규 도입, 순수 로직 함수만 커버).

## Global Constraints

- 설계 문서: `docs/superpowers/specs/2026-08-12-conversational-npc-design.md` — 모든 태스크는 이 문서의 요구사항을 따른다.
- v1 범위는 host-only. 다른 클라이언트의 RPC 중계는 이 플랜에 포함하지 않는다.
- 대화 기록은 서버가 관리한다. 클라이언트는 별도 히스토리 저장을 하지 않는다.
- 서버 에러는 코드별 분기 없이 전부 동일한 대체 대사로 처리한다.
- AI NPC 서버 기본 접속 정보: host `127.0.0.1`, port `8765`, path `/ws`.
- 엔진 경로: `C:/Program Files/Epic Games/UE_5.7/Engine`. 프로젝트: `D:/Projects/Valheim/Valheim.uproject`. 빌드 타겟명: `ValheimEditor` (Win64, Development) — `Binaries/Win64/ValheimEditor.target`로 확인됨.
- 컴파일 확인 명령 (모든 C++ 태스크 공통):
  ```
  "C:/Program Files/Epic Games/UE_5.7/Engine/Build/BatchFiles/Build.bat" ValheimEditor Win64 Development -Project="D:/Projects/Valheim/Valheim.uproject" -WaitMutex
  ```
- 이 프로젝트에는 기존 자동화 테스트가 전혀 없다. 이 플랜에서 순수 로직(JSON 빌드/파싱)에 한해 Unreal Automation Test를 처음 도입한다. 실행 명령:
  ```
  "C:/Program Files/Epic Games/UE_5.7/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "D:/Projects/Valheim/Valheim.uproject" -ExecCmds="Automation RunTests Valheim.AINpc; Quit" -unattended -nopause -nosplash -log
  ```
  WebSocket 송수신, NPC 위젯 흐름 등 실제 I/O·에디터 자산이 필요한 부분은 이 프레임워크로 커버하지 않고, 태스크별 "수동 검증" 절차로 확인한다.
- UMG 위젯의 시각적 레이아웃(.uasset 블루프린트 생성/배치)은 에이전트가 CLI로 만들 수 없다 — 언리얼 에디터에서 사람이 직접 해야 하는 작업이며, 해당 태스크에 명시했다.

---

### Task 1: `UAINpcSubsystem` — AI NPC 서버 WebSocket 클라이언트

**Files:**
- Modify: `Source/Valheim/Valheim.Build.cs`
- Create: `Source/Valheim/AINPC/AINpcSubsystem.h`
- Create: `Source/Valheim/AINPC/AINpcSubsystem.cpp`
- Test: `Source/Valheim/AINPC/AINpcSubsystemTests.cpp`

**Interfaces:**
- Produces (다음 태스크들이 사용):
  - `bool UAINpcSubsystem::Connect(const FString& Host, int32 Port)`
  - `bool UAINpcSubsystem::IsAIServerConnected() const`
  - `void UAINpcSubsystem::RequestChat(const FString& PlayerId, const FString& NpcId, const FString& Message)`
  - `FOnAIChatReply OnAIChatReply` — `(const FString& PlayerId, const FString& NpcId, const FString& Reply)`
  - `FOnAIChatError OnAIChatError` — `(const FString& PlayerId, const FString& NpcId, const FString& ErrorCode)`
  - `static FString UAINpcSubsystem::BuildChatRequestJson(const FString& PlayerId, const FString& NpcId, const FString& Message)`
  - `static bool UAINpcSubsystem::ParseServerMessage(const FString& JsonText, FString& OutType, FString& OutPlayerId, FString& OutNpcId, FString& OutPayload)`

- [ ] **Step 1: 모듈 의존성 추가**

`Source/Valheim/Valheim.Build.cs`의 `PublicDependencyModuleNames.AddRange` 목록에 `"WebSockets"`, `"Json"`, `"JsonUtilities"` 추가:

```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "Core",
    "CoreUObject",
    "Engine",
    "InputCore",
    "EnhancedInput",
    "AIModule",
    "StateTreeModule",
    "GameplayStateTreeModule",
    "UMG","Sockets","Networking",
    "Slate","SlateCore","AIModule","GameplayTasks","NetCore", "OnlineSubsystem", "OnlineSubsystemUtils",
    "WebSockets", "Json", "JsonUtilities"
});
```

- [ ] **Step 2: 헤더 작성**

`Source/Valheim/AINPC/AINpcSubsystem.h`:

```cpp
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AINpcSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAIChatReply, const FString&, PlayerId, const FString&, NpcId, const FString&, Reply);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAIChatError, const FString&, PlayerId, const FString&, NpcId, const FString&, ErrorCode);

class IWebSocket;

UCLASS()
class VALHEIM_API UAINpcSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "AINpc")
	bool Connect(const FString& Host, int32 Port);

	UFUNCTION(BlueprintCallable, Category = "AINpc")
	bool IsAIServerConnected() const;

	UFUNCTION(BlueprintCallable, Category = "AINpc")
	void RequestChat(const FString& PlayerId, const FString& NpcId, const FString& Message);

	UPROPERTY(BlueprintAssignable, Category = "AINpc")
	FOnAIChatReply OnAIChatReply;

	UPROPERTY(BlueprintAssignable, Category = "AINpc")
	FOnAIChatError OnAIChatError;

	static FString BuildChatRequestJson(const FString& PlayerId, const FString& NpcId, const FString& Message);
	static bool ParseServerMessage(const FString& JsonText, FString& OutType, FString& OutPlayerId, FString& OutNpcId, FString& OutPayload);

private:
	TSharedPtr<IWebSocket> Socket;

	void HandleMessage(const FString& MessageString);
};
```

- [ ] **Step 3: cpp 스켈레톤 작성 (순수 함수는 스텁, 나머지는 실제 구현)**

`Source/Valheim/AINPC/AINpcSubsystem.cpp`:

```cpp
// Fill out your copyright notice in the Description page of Project Settings.

#include "AINPC/AINpcSubsystem.h"
#include "WebSocketsModule.h"
#include "IWebSocket.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

FString UAINpcSubsystem::BuildChatRequestJson(const FString& PlayerId, const FString& NpcId, const FString& Message)
{
	return TEXT("");
}

bool UAINpcSubsystem::ParseServerMessage(const FString& JsonText, FString& OutType, FString& OutPlayerId, FString& OutNpcId, FString& OutPayload)
{
	return false;
}

void UAINpcSubsystem::Deinitialize()
{
	if (Socket.IsValid() && Socket->IsConnected())
	{
		Socket->Close();
	}
	Socket.Reset();
	Super::Deinitialize();
}

bool UAINpcSubsystem::Connect(const FString& Host, int32 Port)
{
	if (Socket.IsValid() && Socket->IsConnected())
	{
		return true;
	}

	if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
	{
		FModuleManager::Get().LoadModule("WebSockets");
	}

	const FString Url = FString::Printf(TEXT("ws://%s:%d/ws"), *Host, Port);
	Socket = FWebSocketsModule::Get().CreateWebSocket(Url);

	Socket->OnMessage().AddUObject(this, &UAINpcSubsystem::HandleMessage);
	Socket->OnConnectionError().AddLambda([](const FString& Error)
	{
		UE_LOG(LogTemp, Warning, TEXT("UAINpcSubsystem: connection error: %s"), *Error);
	});

	Socket->Connect();
	return true;
}

bool UAINpcSubsystem::IsAIServerConnected() const
{
	return Socket.IsValid() && Socket->IsConnected();
}

void UAINpcSubsystem::RequestChat(const FString& PlayerId, const FString& NpcId, const FString& Message)
{
	if (!IsAIServerConnected())
	{
		OnAIChatError.Broadcast(PlayerId, NpcId, TEXT("not_connected"));
		return;
	}

	Socket->Send(BuildChatRequestJson(PlayerId, NpcId, Message));
}

void UAINpcSubsystem::HandleMessage(const FString& MessageString)
{
	FString Type, PlayerId, NpcId, Payload;
	if (!ParseServerMessage(MessageString, Type, PlayerId, NpcId, Payload))
	{
		UE_LOG(LogTemp, Warning, TEXT("UAINpcSubsystem: failed to parse message: %s"), *MessageString);
		return;
	}

	if (Type == TEXT("chat_reply"))
	{
		OnAIChatReply.Broadcast(PlayerId, NpcId, Payload);
	}
	else if (Type == TEXT("error"))
	{
		OnAIChatError.Broadcast(PlayerId, NpcId, Payload);
	}
}
```

- [ ] **Step 4: 실패하는 자동화 테스트 작성**

`Source/Valheim/AINPC/AINpcSubsystemTests.cpp`:

```cpp
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
```

- [ ] **Step 5: 컴파일 후 테스트 실행 → 실패 확인**

```
"C:/Program Files/Epic Games/UE_5.7/Engine/Build/BatchFiles/Build.bat" ValheimEditor Win64 Development -Project="D:/Projects/Valheim/Valheim.uproject" -WaitMutex
"C:/Program Files/Epic Games/UE_5.7/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "D:/Projects/Valheim/Valheim.uproject" -ExecCmds="Automation RunTests Valheim.AINpc; Quit" -unattended -nopause -nosplash -log
```

Expected: 컴파일 성공, 세 테스트 모두 FAIL (스텁이 빈 문자열/false를 반환하므로).

- [ ] **Step 6: `BuildChatRequestJson`/`ParseServerMessage` 실제 구현**

`AINpcSubsystem.cpp`의 두 스텁을 아래로 교체:

```cpp
FString UAINpcSubsystem::BuildChatRequestJson(const FString& PlayerId, const FString& NpcId, const FString& Message)
{
	TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("type"), TEXT("chat"));
	Json->SetStringField(TEXT("player_id"), PlayerId);
	Json->SetStringField(TEXT("npc_id"), NpcId);
	Json->SetStringField(TEXT("message"), Message);

	FString Output;
	TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Output);
	FJsonSerializer::Serialize(Json, Writer);
	return Output;
}

bool UAINpcSubsystem::ParseServerMessage(const FString& JsonText, FString& OutType, FString& OutPlayerId, FString& OutNpcId, FString& OutPayload)
{
	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		return false;
	}

	if (!Json->TryGetStringField(TEXT("type"), OutType))
	{
		return false;
	}

	Json->TryGetStringField(TEXT("player_id"), OutPlayerId);
	Json->TryGetStringField(TEXT("npc_id"), OutNpcId);

	if (OutType == TEXT("chat_reply"))
	{
		return Json->TryGetStringField(TEXT("reply"), OutPayload);
	}
	if (OutType == TEXT("error"))
	{
		return Json->TryGetStringField(TEXT("error"), OutPayload);
	}
	return false;
}
```

- [ ] **Step 7: 컴파일 후 테스트 재실행 → 통과 확인**

동일한 두 명령을 다시 실행. Expected: 컴파일 성공, 세 테스트 모두 PASS.

- [ ] **Step 8: 서버 대상 수동 스모크 테스트 (WebSocket 송수신)**

AI NPC 서버 저장소에서:
```
.venv\Scripts\python run.py
```
언리얼 에디터를 열고 (PIE 불필요), 레벨 블루프린트나 에디터 유틸리티 위젯에서 임시로 `UAINpcSubsystem::Connect("127.0.0.1", 8765)` → `RequestChat("test", "guard", "안녕")`을 호출해보고, Output Log에서 `OnAIChatReply`/`OnAIChatError`가 정상적으로 브로드캐스트되는지 확인 (임시 호출 코드/BP 노드는 검증 후 제거). `scripts/manual_chat_client.py`로 서버가 보내는 응답과 형식이 같은지 비교.

- [ ] **Step 9: 커밋**

```bash
git add Source/Valheim/Valheim.Build.cs Source/Valheim/AINPC/AINpcSubsystem.h Source/Valheim/AINPC/AINpcSubsystem.cpp Source/Valheim/AINPC/AINpcSubsystemTests.cpp
git commit -m "feat: AI NPC 서버 WebSocket 클라이언트 서브시스템 추가"
```

---

### Task 2: `UNPCDialogueWidget` — 대화 UI

**Files:**
- Create: `Source/Valheim/UserInterface/AINpc/NPCDialogueWidget.h`
- Create: `Source/Valheim/UserInterface/AINpc/NPCDialogueWidget.cpp`

**Interfaces:**
- Consumes:
  - `UAINpcSubsystem` (Task 1): `RequestChat`, `IsAIServerConnected`, `OnAIChatReply`, `OnAIChatError`
  - `UChatEntry` (기존, `Source/Valheim/UserInterface/Chat/ChatEntry.h`): `SetChatEntry(const FString& InUserId, const FString& InMessage)` — 신규 위젯을 만들지 않고 그대로 재사용
  - `UValheimGI::SavedNickname` (기존, `Source/Valheim/Core/ValheimGI.h`)
- Produces (Task 3이 사용):
  - `void UNPCDialogueWidget::OpenDialogue(const FString& InNpcId, const FString& InNpcDisplayName)`
  - `void UNPCDialogueWidget::CloseDialogue()`

- [ ] **Step 1: 헤더 작성**

`Source/Valheim/UserInterface/AINpc/NPCDialogueWidget.h`:

```cpp
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NPCDialogueWidget.generated.h"

class UEditableText;
class UScrollBox;
class UChatEntry;

UCLASS()
class VALHEIM_API UNPCDialogueWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> DialogueInputBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> DialogueScrollBox;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AINpc")
	TSubclassOf<UChatEntry> DialogueEntryWidgetClass;

	UFUNCTION(BlueprintCallable, Category = "AINpc")
	void OpenDialogue(const FString& InNpcId, const FString& InNpcDisplayName);

	UFUNCTION(BlueprintCallable, Category = "AINpc")
	void CloseDialogue();

	UFUNCTION(BlueprintCallable, Category = "AINpc")
	void SendDialogueMessage();

protected:
	FString NpcId;
	FString NpcDisplayName;
	bool bWaitingForReply = false;
	FTimerHandle TimeoutTimerHandle;

	UFUNCTION()
	void OnDialogueInputCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void HandleAIChatReply(const FString& PlayerId, const FString& InNpcId, const FString& Reply);

	UFUNCTION()
	void HandleAIChatError(const FString& PlayerId, const FString& InNpcId, const FString& ErrorCode);

	void HandleTimeout();
	void AddDialogueEntry(const FString& Speaker, const FString& Message);
	void SetWaitingForReply(bool bWaiting);
};
```

- [ ] **Step 2: cpp 작성**

`Source/Valheim/UserInterface/AINpc/NPCDialogueWidget.cpp`:

```cpp
// Fill out your copyright notice in the Description page of Project Settings.

#include "UserInterface/AINpc/NPCDialogueWidget.h"
#include "UserInterface/Chat/ChatEntry.h"
#include "AINPC/AINpcSubsystem.h"
#include "Core/ValheimGI.h"

#include "Components/EditableText.h"
#include "Components/ScrollBox.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

static constexpr float NPCDialogueTimeoutSeconds = 10.f;

void UNPCDialogueWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (DialogueInputBox)
	{
		DialogueInputBox->OnTextCommitted.AddDynamic(this, &UNPCDialogueWidget::OnDialogueInputCommitted);
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UAINpcSubsystem* Subsystem = GameInstance->GetSubsystem<UAINpcSubsystem>())
		{
			Subsystem->OnAIChatReply.AddDynamic(this, &UNPCDialogueWidget::HandleAIChatReply);
			Subsystem->OnAIChatError.AddDynamic(this, &UNPCDialogueWidget::HandleAIChatError);
		}
	}
}

void UNPCDialogueWidget::NativeDestruct()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UAINpcSubsystem* Subsystem = GameInstance->GetSubsystem<UAINpcSubsystem>())
		{
			Subsystem->OnAIChatReply.RemoveDynamic(this, &UNPCDialogueWidget::HandleAIChatReply);
			Subsystem->OnAIChatError.RemoveDynamic(this, &UNPCDialogueWidget::HandleAIChatError);
		}
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimeoutTimerHandle);
	}

	Super::NativeDestruct();
}

void UNPCDialogueWidget::OpenDialogue(const FString& InNpcId, const FString& InNpcDisplayName)
{
	NpcId = InNpcId;
	NpcDisplayName = InNpcDisplayName;
	SetWaitingForReply(false);

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UAINpcSubsystem* Subsystem = GameInstance->GetSubsystem<UAINpcSubsystem>())
		{
			if (UValheimGI* VGI = Cast<UValheimGI>(GameInstance))
			{
				Subsystem->Connect(VGI->AIServerHost, VGI->AIServerPort);
			}
		}
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeUIOnly InputMode;
		if (DialogueInputBox)
		{
			InputMode.SetWidgetToFocus(DialogueInputBox->TakeWidget());
		}
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
	}

	if (DialogueInputBox)
	{
		DialogueInputBox->SetKeyboardFocus();
	}
}

void UNPCDialogueWidget::CloseDialogue()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimeoutTimerHandle);
	}
	SetWaitingForReply(false);

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeGameOnly());
	}
	UWidgetBlueprintLibrary::SetFocusToGameViewport();

	RemoveFromParent();
}

void UNPCDialogueWidget::SendDialogueMessage()
{
	if (!DialogueInputBox || bWaitingForReply)
	{
		return;
	}

	const FString Message = DialogueInputBox->GetText().ToString().TrimStartAndEnd();
	if (Message.IsEmpty())
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	UAINpcSubsystem* Subsystem = GameInstance->GetSubsystem<UAINpcSubsystem>();
	UValheimGI* VGI = Cast<UValheimGI>(GameInstance);
	if (!Subsystem || !VGI)
	{
		return;
	}

	if (!Subsystem->IsAIServerConnected())
	{
		AddDialogueEntry(NpcDisplayName, TEXT("지금은 대답할 수 없다."));
		DialogueInputBox->SetText(FText::GetEmpty());
		return;
	}

	const FString PlayerId = VGI->SavedNickname;
	AddDialogueEntry(PlayerId, Message);
	DialogueInputBox->SetText(FText::GetEmpty());

	Subsystem->RequestChat(PlayerId, NpcId, Message);
	SetWaitingForReply(true);

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(TimeoutTimerHandle, FTimerDelegate::CreateUObject(this, &UNPCDialogueWidget::HandleTimeout), NPCDialogueTimeoutSeconds, false);
	}
}

void UNPCDialogueWidget::OnDialogueInputCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod != ETextCommit::OnEnter)
	{
		return;
	}

	SendDialogueMessage();

	if (DialogueInputBox)
	{
		DialogueInputBox->SetKeyboardFocus();
	}
}

void UNPCDialogueWidget::HandleAIChatReply(const FString& PlayerId, const FString& InNpcId, const FString& Reply)
{
	if (InNpcId != NpcId || !bWaitingForReply)
	{
		return;
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimeoutTimerHandle);
	}

	AddDialogueEntry(NpcDisplayName, Reply);
	SetWaitingForReply(false);
}

void UNPCDialogueWidget::HandleAIChatError(const FString& PlayerId, const FString& InNpcId, const FString& ErrorCode)
{
	if (InNpcId != NpcId || !bWaitingForReply)
	{
		return;
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimeoutTimerHandle);
	}

	UE_LOG(LogTemp, Warning, TEXT("UNPCDialogueWidget: AI chat error [%s]"), *ErrorCode);
	AddDialogueEntry(NpcDisplayName, TEXT("지금은 대답할 수 없다."));
	SetWaitingForReply(false);
}

void UNPCDialogueWidget::HandleTimeout()
{
	if (!bWaitingForReply)
	{
		return;
	}

	AddDialogueEntry(NpcDisplayName, TEXT("지금은 대답할 수 없다."));
	SetWaitingForReply(false);
}

void UNPCDialogueWidget::AddDialogueEntry(const FString& Speaker, const FString& Message)
{
	if (!DialogueScrollBox || !DialogueEntryWidgetClass)
	{
		return;
	}

	UChatEntry* NewEntry = CreateWidget<UChatEntry>(this, DialogueEntryWidgetClass);
	if (!NewEntry)
	{
		return;
	}

	NewEntry->SetChatEntry(Speaker, Message);
	DialogueScrollBox->AddChild(NewEntry);
	DialogueScrollBox->ScrollToEnd();
}

void UNPCDialogueWidget::SetWaitingForReply(bool bWaiting)
{
	bWaitingForReply = bWaiting;
	if (DialogueInputBox)
	{
		DialogueInputBox->SetIsEnabled(!bWaiting);
	}
}
```

- [ ] **Step 3: 컴파일 확인**

```
"C:/Program Files/Epic Games/UE_5.7/Engine/Build/BatchFiles/Build.bat" ValheimEditor Win64 Development -Project="D:/Projects/Valheim/Valheim.uproject" -WaitMutex
```

Expected: 컴파일 성공 (에러 없음). 이 태스크는 UMG 자산 없이는 PIE로 직접 검증할 수 없으므로, 실사용 검증은 Task 4에서 진행한다.

- [ ] **Step 4: 커밋**

```bash
git add Source/Valheim/UserInterface/AINpc/NPCDialogueWidget.h Source/Valheim/UserInterface/AINpc/NPCDialogueWidget.cpp
git commit -m "feat: AI NPC 대화 위젯(NPCDialogueWidget) 추가"
```

---

### Task 3: `ANPC` 확장 + `ValheimGI` 설정 필드

**Files:**
- Modify: `Source/Valheim/Core/ValheimGI.h`
- Modify: `Source/Valheim/NPC/NPC.h`
- Modify: `Source/Valheim/NPC/NPC.cpp`

**Interfaces:**
- Consumes: `UNPCDialogueWidget::OpenDialogue` (Task 2), `UValheimGI::AIServerHost`/`AIServerPort` (본 태스크에서 추가)
- Produces (Task 4가 사용): `ANPC::bAIEnabled` (EditAnywhere, 레벨의 NPC 인스턴스에서 켬), `ANPC::AIDialogueWidgetClass` (EditAnywhere, BP에서 위젯 클래스 지정)

- [ ] **Step 1: `ValheimGI`에 AI 서버 접속 설정 추가**

`Source/Valheim/Core/ValheimGI.h`의 기존 `UPROPERTY(BlueprintReadWrite, Category = "Player") FString SavedNickname;` 아래에 추가:

```cpp
	UPROPERTY(EditDefaultsOnly, Category = "AINpc")
	FString AIServerHost = TEXT("127.0.0.1");

	UPROPERTY(EditDefaultsOnly, Category = "AINpc")
	int32 AIServerPort = 8765;
```

- [ ] **Step 2: `ANPC.h`에 AI 대화 관련 필드/함수 추가**

`Source/Valheim/NPC/NPC.h`에서 `NPCID` 선언부 아래, `Interact` 선언 위에 추가:

```cpp
	UPROPERTY(EditAnywhere, Category = "AINpc")
	bool bAIEnabled = false;

	UPROPERTY(EditAnywhere, Category = "AINpc")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, Category = "AINpc")
	TSubclassOf<class UNPCDialogueWidget> AIDialogueWidgetClass;
```

`protected:` 섹션의 `HandleQuestInteraction` 선언 아래에 추가:

```cpp
	void OpenAIDialogue(APawn* Interactor);
```

- [ ] **Step 3: `ANPC.cpp`의 `Interact()`에 분기 추가**

기존 `Interact()`를 아래로 교체 (`HandleQuestInteraction(QuestPlayerState);` 호출 부분만 분기로 변경, 나머지 null 체크는 그대로 유지):

```cpp
void ANPC::Interact(APawn* Interactor)
{
	if (!Interactor)
	{
		UE_LOG(LogTemp, Warning, TEXT("ANPC::Interact !Interactor"))
		return;
	}

	if (bAIEnabled)
	{
		OpenAIDialogue(Interactor);
		return;
	}

	AArcher* Taker = Cast<AArcher>(Interactor);
	if (!Taker)
	{
		UE_LOG(LogTemp, Warning, TEXT("ANPC::Interact !Taker"))
		return;
	}

	AArcherPS* QuestPlayerState = Taker->GetPlayerState<AArcherPS>();
	if (!QuestPlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("ANPC::Interact !QuestPlayerState"))
		return;
	}

	HandleQuestInteraction(QuestPlayerState);
}
```

파일 상단 include에 추가:

```cpp
#include "UserInterface/AINpc/NPCDialogueWidget.h"
```

`HandleQuestInteraction` 구현부 아래에 새 함수 추가:

```cpp
void ANPC::OpenAIDialogue(APawn* Interactor)
{
	if (!AIDialogueWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ANPC::OpenAIDialogue !AIDialogueWidgetClass"))
		return;
	}

	APlayerController* PC = Cast<APlayerController>(Interactor->GetController());
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	UNPCDialogueWidget* DialogueWidget = CreateWidget<UNPCDialogueWidget>(PC, AIDialogueWidgetClass);
	if (!DialogueWidget)
	{
		return;
	}

	const FString ResolvedNpcId = NPCID.ToString();
	const FString ResolvedDisplayName = DisplayName.IsEmpty() ? ResolvedNpcId : DisplayName.ToString();

	DialogueWidget->AddToViewport();
	DialogueWidget->OpenDialogue(ResolvedNpcId, ResolvedDisplayName);
}
```

- [ ] **Step 4: 컴파일 확인**

```
"C:/Program Files/Epic Games/UE_5.7/Engine/Build/BatchFiles/Build.bat" ValheimEditor Win64 Development -Project="D:/Projects/Valheim/Valheim.uproject" -WaitMutex
```

Expected: 컴파일 성공.

- [ ] **Step 5: 커밋**

```bash
git add Source/Valheim/Core/ValheimGI.h Source/Valheim/NPC/NPC.h Source/Valheim/NPC/NPC.cpp
git commit -m "feat: ANPC에 AI 대화 분기(bAIEnabled) 추가"
```

---

### Task 4: 에디터 자산 생성 + 엔드투엔드 수동 검증 (사람이 직접 진행)

이 태스크는 언리얼 에디터 GUI 작업이 필요해 에이전트가 CLI로 대신할 수 없다. 아래 체크리스트를 사람이 직접 수행한다.

**Files (에디터에서 생성/수정, 코드 변경 없음):**
- Create: `Content/UserInterface/AINpc/WBP_NPCDialogue.uasset`
- Modify: `Content/Blueprints/NPC/BP_NPC.uasset` (또는 테스트용 NPC 인스턴스 하나)

- [ ] **Step 1: `WBP_NPCDialogue` 블루프린트 위젯 생성**

`Content/UserInterface/AINpc/` 폴더에 `WBP_NPCDialogue` 생성, 부모 클래스는 `NPCDialogueWidget`. `Content/UserInterface/Chat/WBP_ChatBox.uasset`의 레이아웃(입력창 + 스크롤박스)을 참고해 `EditableText`를 `DialogueInputBox`로, `ScrollBox`를 `DialogueScrollBox`로 이름 지정해 바인딩. Class Defaults에서 `Dialogue Entry Widget Class`를 기존 `WBP_ChatEntry`로 지정 (신규 엔트리 위젯을 만들지 않고 재사용).

- [ ] **Step 2: 테스트용 NPC에 AI 대화 활성화**

레벨의 NPC 인스턴스(또는 `BP_NPC`의 테스트용 배치본) 하나를 골라 Details 패널에서:
- `AI Npc` 카테고리의 `bAIEnabled` 체크
- `NPCID`를 `guard`로 설정 (AI NPC 서버의 `npcs/guard.json`과 일치)
- `Display Name`을 원하는 NPC 이름으로 설정 (예: "경비병")
- `AI Dialogue Widget Class`를 `WBP_NPCDialogue`로 설정

- [ ] **Step 3: AI NPC 서버 실행**

AI NPC 서버 저장소에서:
```
.venv\Scripts\python run.py
```

- [ ] **Step 4: PIE로 엔드투엔드 확인**

리슨서버(싱글 플레이 OK)로 PIE 시작 → `bAIEnabled` NPC에게 다가가 상호작용 키로 대화창 열기 → 메시지 입력/엔터 → 서버 응답이 대화창에 표시되는지 확인. `docs/superpowers/specs/2026-08-12-conversational-npc-design.md`의 테스트 계획 6, 7번 항목도 함께 확인:
- 대화 도중 서버(`run.py`)를 종료해 에러 경로(대체 대사 "지금은 대답할 수 없다." 표시, 크래시 없음, 입력창 다시 활성화) 확인.
- `bAIEnabled=false`인 기존 퀘스트 NPC에게 상호작용해 기존 퀘스트 로직이 그대로 동작하는지 회귀 확인 (아무 것도 안 변해야 함).

- [ ] **Step 5: 커밋**

```bash
git add Content/UserInterface/AINpc/WBP_NPCDialogue.uasset Content/Blueprints/NPC/BP_NPC.uasset
git commit -m "feat: AI NPC 대화 위젯 블루프린트 및 테스트 NPC 설정 추가"
```

(레벨 내 NPC 인스턴스 값만 바꾼 경우, 해당 레벨 `.umap` 파일도 함께 `git add`.)
