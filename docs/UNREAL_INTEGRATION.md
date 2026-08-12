# 언리얼 프로젝트 연동 가이드

이 문서는 언리얼 프로젝트 쪽(리슨서버 호스트)에서 AI NPC 서버와 통신하는 코드를 짤 때 참고하는 프로토콜 레퍼런스다. 서버 쪽 구현 세부사항은 `docs/superpowers/specs/2026-08-12-ai-npc-server-design.md`에 있고, 여기서는 언리얼 쪽에서 알아야 할 것만 정리한다.

## 연결 모델

**리슨서버 호스트만** 이 서버에 WebSocket으로 연결한다. 다른 플레이어(클라이언트)의 NPC 대화 요청은 언리얼 자체 네트워킹(RPC)으로 먼저 호스트에 모은 뒤, 호스트가 대신 이 서버에 요청하고 응답을 다시 리플리케이션으로 해당 클라이언트에 돌려줘야 한다 — 이 중계 로직은 언리얼 프로젝트 쪽에서 구현해야 하며, AI NPC 서버는 이걸 모른다. 서버 입장에서는 그냥 "호스트라는 WebSocket 클라이언트 하나가 여러 (player_id, npc_id) 조합의 요청을 보낸다"로만 취급한다.

연결은 리슨서버 시작 시 한 번 맺고 게임이 끝날 때까지 유지하는 것을 권장한다 (요청마다 새로 연결하지 않는다).

## 접속 주소

```
ws://<AI NPC 서버 호스트>:8765/ws
```

- 기본 포트는 `8765` (서버 쪽 `SERVER_PORT` 환경변수로 변경 가능).
- 로컬에서 같은 PC에 띄운 경우 `ws://localhost:8765/ws`.

## 메시지 프로토콜

JSON 텍스트 메시지를 주고받는다. 하나의 연결 위에서 여러 요청을 동시에 보낼 수 있고, **응답 순서는 보장되지 않으므로** `player_id` + `npc_id`로 어떤 요청에 대한 응답인지 매칭해야 한다.

### 요청 (언리얼 → 서버)

```json
{
  "type": "chat",
  "player_id": "playerA",
  "npc_id": "guard",
  "message": "안녕"
}
```

| 필드 | 타입 | 설명 |
|---|---|---|
| `type` | `"chat"` 고정 | |
| `player_id` | string | 대화 중인 플레이어를 구분하는 ID (언리얼 쪽에서 정하는 값, 예: 플레이어 컨트롤러 고유 ID) |
| `npc_id` | string | 대화 상대 NPC ID. 서버의 `npcs/<npc_id>.json` 파일과 일치해야 함 (아래 "사용 가능한 NPC" 참고) |
| `message` | string | 플레이어가 입력한 텍스트 |

### 성공 응답 (서버 → 언리얼)

```json
{
  "type": "chat_reply",
  "player_id": "playerA",
  "npc_id": "guard",
  "reply": "어서 오게, 여행자."
}
```

`reply`를 그대로 NPC 대사로 표시하면 된다.

### 에러 응답 (서버 → 언리얼)

```json
{
  "type": "error",
  "player_id": "playerA",
  "npc_id": "guard",
  "error": "npc_not_found"
}
```

`player_id`/`npc_id`는 요청이 아예 파싱되지 않은 경우(`bad_request`) `null`일 수 있다.

| `error` 값 | 의미 | 언리얼 쪽 권장 처리 |
|---|---|---|
| `npc_not_found` | 요청한 `npc_id`가 서버에 등록되지 않음 | 개발 단계 버그로 취급 (오타 등). 플레이어에게는 일반적인 "지금은 대답할 수 없다" 정도로 표시 |
| `llm_unavailable` | 로컬 LLM(LM Studio/llama.cpp) 호출 실패 또는 타임아웃 | 재시도하거나, NPC가 "지금은 대답하기 곤란하다"는 식의 대체 대사를 표시 |
| `bad_request` | 요청 JSON 형식이 잘못됨(필드 누락 등) | 언리얼 쪽 요청 생성 로직 버그. 재발 시 로그 확인 |
| `storage_error` | 서버의 대화 기록 조회 실패 | `llm_unavailable`과 동일하게 처리 (일시적 서버 문제) |

연결 자체는 에러가 나도 끊기지 않으니, 에러를 받아도 같은 WebSocket으로 계속 요청을 보내면 된다.

## 사용 가능한 NPC

서버 시작 시 `npcs/*.json` 파일들을 읽어서 NPC 목록을 구성한다. 현재 등록된 예시:

- `guard` — 경비병

새 NPC를 추가하려면 서버 쪽 `npcs/<npc_id>.json`에 `npc_id`, `name`, `system_prompt` 필드를 채운 파일을 추가하고 서버를 재시작해야 한다 (언리얼 쪽에서 임의의 `npc_id`를 보낸다고 서버가 자동으로 NPC를 만들어주지 않는다).

## 로컬 테스트

언리얼 통합 코드를 짜기 전에, 서버가 정상 동작하는지 먼저 확인하려면:

```
.venv\Scripts\python run.py
.venv\Scripts\python scripts\manual_chat_client.py playerA guard 안녕
```

이 스크립트가 보내는 요청/응답 형식이 위 프로토콜과 동일하므로, 언리얼 쪽 WebSocket 클라이언트 구현의 참고 예시로 쓸 수 있다.

## 참고

- 대화 기록은 서버가 `(player_id, npc_id)` 쌍 단위로 영구 저장하므로, 같은 `player_id`로 같은 NPC에게 다시 말을 걸면 이전 대화 맥락이 이어진다. 언리얼 쪽에서 별도로 히스토리를 관리할 필요는 없다.
- 이 서버는 텍스트 대화만 지원한다 (음성/애니메이션 트리거 없음).
