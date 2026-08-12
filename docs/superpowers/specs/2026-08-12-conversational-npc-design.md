# 대화형 AI NPC (언리얼 클라이언트) 설계

이 문서는 `docs/UNREAL_INTEGRATION.md`에 정의된 프로토콜을 사용해, 언리얼 프로젝트 안에 "AI와 대화하는 NPC" 기능을 추가하는 설계다. AI NPC 서버 자체는 이 저장소 밖에 별도로 존재하며(사용자가 직접 실행), 여기서는 그 서버에 WebSocket으로 접속하는 클라이언트 쪽만 다룬다.

## 범위

- **포함**: 리슨서버 호스트가 직접 AI NPC와 대화하는 기능 (싱글/호스트 플레이 기준).
- **제외 (v1)**: 다른 클라이언트(호스트가 아닌 플레이어)의 대화 요청을 호스트가 RPC로 중계하는 로직. `docs/UNREAL_INTEGRATION.md`에 설계는 되어 있으나, 이번 버전에서는 구현하지 않고 추후 별도 작업으로 진행한다.
- 대화 기록 저장/관리는 AI NPC 서버가 전적으로 담당한다 (`(player_id, npc_id)` 단위 영구 저장). 언리얼 쪽은 별도 히스토리 관리를 하지 않는다.

## 아키텍처

```
ANPC (bAIEnabled = true)
  └─ Interact() → 분기: bAIEnabled면 AI 대화창, 아니면 기존 퀘스트 로직
       └─ UNPCDialogueWidget (신규)
              └─ UAINpcSubsystem (신규, GameInstanceSubsystem)
                     └─ IWebSocket → AI NPC 서버 (ws://<host>:8765/ws)
```

### 모듈 의존성

`Valheim.Build.cs`에 다음 모듈 추가 (현재 없음):
- `WebSockets` — 언리얼 내장 WebSocket 클라이언트
- `Json`, `JsonUtilities` — 요청/응답 JSON 직렬화

### `UAINpcSubsystem` (신규, `UGameInstanceSubsystem`)

- AI NPC 서버와의 WebSocket 연결을 단일 소유. 연결은 최초 사용 시 1회 맺고 유지 (요청마다 재연결하지 않음 — 문서 권장 사항).
- `RequestChat(FString PlayerId, FString NpcId, FString Message)`: 요청 JSON(`{"type":"chat", "player_id":..., "npc_id":..., "message":...}`)을 만들어 전송.
- 델리게이트:
  - `OnAIChatReply(FString PlayerId, FString NpcId, FString Reply)`
  - `OnAIChatError(FString PlayerId, FString NpcId, FString ErrorCode)`
- 수신 메시지를 파싱해 `type`이 `chat_reply`면 `OnAIChatReply`, `error`면 `OnAIChatError`를 브로드캐스트. `player_id`+`npc_id`로 어떤 요청에 대한 응답인지 매칭 (문서에 명시된 대로 응답 순서 보장 안 됨을 전제).
- v1은 host-only이므로 재연결/큐잉/다중 연결 로직은 넣지 않는다. 연결 실패 시 로그만 남기고, 호출부(`UNPCDialogueWidget`)가 연결 상태를 확인해 대체 대사를 표시하게 한다.

### `ANPC` 확장

- `UPROPERTY(EditAnywhere) bool bAIEnabled` 필드 추가.
- 기존 `Interact()`에 분기 추가: `bAIEnabled == true`면 새 `OpenAIDialogue(APawn* Interactor)`를 호출하고 기존 퀘스트 로직(`HandleQuestInteraction`)은 건너뛴다. `bAIEnabled == false`(기본값)면 기존 동작 그대로 — 퀘스트 NPC는 전혀 영향받지 않는다.
- 기존 `NPCID`(FName) 필드를 그대로 `npc_id`로 재사용한다 (서버의 `npcs/<npc_id>.json`과 일치해야 함, 예: `guard`).

### `UNPCDialogueWidget` (신규)

- 입력창 + 대화 표시 영역으로 구성. UI 패턴은 기존 `ChatBox`/`ChatEntry`를 참고해 새로 만든다 (전역 채팅과는 별개 채널이므로 별도 위젯 클래스).
- 제출 시:
  - `player_id` = `UValheimGI::SavedNickname` (게임 인스턴스에 이미 저장되어 있는 닉네임 재사용)
  - `npc_id` = 상호작용한 `ANPC`의 `NPCID`
  - `UAINpcSubsystem::RequestChat(player_id, npc_id, message)` 호출
- 응답 대기 중에는 입력창을 잠가 중복 전송을 막는다 (한 번에 하나의 요청/응답만 처리, 매칭 로직은 있지만 동시 다발 요청 자체를 방지).
- `OnAIChatReply`/`OnAIChatError` 구독 후, 자신이 열려 있는 NPC의 `npc_id`와 일치하는 응답만 받아 대사로 표시.

## 데이터 흐름

```
플레이어(호스트) NPC 상호작용
  → ANPC::Interact() → bAIEnabled=true → OpenAIDialogue()
  → UNPCDialogueWidget 표시, 입력창 포커스, 입력창 활성화
  → 플레이어 메시지 입력/제출 → 입력창 잠금
  → UAINpcSubsystem::RequestChat(SavedNickname, NPCID, Message)
  → JSON 직렬화 → WebSocket 전송
  → (서버가 chat_reply 또는 error로 응답, 순서 보장 없음)
  → UAINpcSubsystem이 파싱 → (player_id, npc_id) 매칭 → 델리게이트 브로드캐스트
  → UNPCDialogueWidget이 자신의 npc_id와 일치하는 응답만 받아 대사로 표시 → 입력창 잠금 해제
```

## 에러 처리

`docs/UNREAL_INTEGRATION.md`의 권장 처리를 그대로 따른다. 서버 에러 코드별로 UI 분기를 세분화하지 않고 모두 대체 대사 하나로 통일 처리한다 (v1은 재시도 로직 없음).

| 상황 | 처리 |
|---|---|
| `npc_not_found`, `bad_request` | 대체 대사("지금은 대답할 수 없다") 표시 + 로그 (개발 단계 버그로 간주) |
| `llm_unavailable`, `storage_error` | 대체 대사 표시 (일시적 서버 문제로 간주) |
| WebSocket 연결 안 됨/끊김 | 대화창을 열 때 연결 상태 확인, 연결 안 되어 있으면 즉시 대체 대사 |
| 응답 타임아웃 (응답 자체가 안 옴) | 클라이언트 타이머(10초)로 자체 타임아웃 처리 → 대체 대사 표시 + 입력창 잠금 해제 |

연결 자체는 서버 에러가 나도 끊기지 않으므로(문서 명시), 재연결 로직은 v1에서 구현하지 않는다.

## 테스트 계획

1. AI NPC 서버를 직접 실행(`run.py`) 후 `scripts/manual_chat_client.py`로 서버 단독 동작을 먼저 확인한다 (언리얼 코드를 건드리기 전).
2. `Valheim.Build.cs`에 `WebSockets`/`Json`/`JsonUtilities` 추가 후 빌드 확인.
3. `UAINpcSubsystem` 구현 후, 연결/전송/수신이 되는지 로그로 확인.
4. `ANPC`에 `bAIEnabled` 필드 추가, 레벨의 NPC 하나에 `bAIEnabled=true`, `NPCID="guard"` 설정.
5. `UNPCDialogueWidget` 연결 후 PIE(리슨서버, 싱글)로 실제 대화 왕복 테스트.
6. 서버를 대화 도중 종료해 에러 경로(대체 대사 표시, 크래시 없음)를 확인한다.
7. 기존 퀘스트 NPC(`bAIEnabled=false`)가 기존과 동일하게 동작하는지 회귀 확인한다.
