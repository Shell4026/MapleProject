# 시스템 아키텍처 기술 문서

## 개요
이 프로젝트는 **UDP + TCP 혼합 통신**과 **서버 권한(authoritative) 시뮬레이션**을 기반으로 구성된 멀티플레이 게임 구조입니다.  
핵심 목표는 다음 3가지입니다.

- 조작감(입력 반응성) 확보
- 서버 일관성(치트/오차 억제)
- 기능 확장 시 결합도 최소화

---

## 실행/빌드 구조

| 구분 | 설명 |
|---|---|
| 단일 코드베이스 | `Source/common`을 client/server가 공유 |
| 분기 빌드 | `SH_SERVER` 매크로로 서버 전용 로직 분기 |
| 타겟 | 클라이언트 DLL, 서버 DLL 분리 빌드 |
| 외부 의존성 | `fmt`, `glm`, `nlohmann_json`, `sqlite3` |

서버/클라이언트가 같은 데이터 타입과 패킷 정의를 공유하므로, 네트워크 동기화 시 타입 불일치 리스크를 줄였습니다.

---

## 월드/서버 구성

### 1) MapleServer
`MapleServer`는 UDP/TCP를 동시에 운용하며 다음 역할을 담당합니다.

- UDP 수신(입력/하트비트/실시간 상태 계열)
- TCP 수신(로그인 토큰 확정, 인벤토리 동기화/교환 등)
- 수신 패킷을 Event Bus로 발행
- 접속/종료 이벤트 처리 및 유저 매니저 틱 실행

### 2) MapleWorld
`MapleWorld`는 월드 단위 시뮬레이션 책임을 가집니다.

- 플레이어 스폰/디스폰
- 아이템 드롭/소멸
- 월드별 라우터(`EntityRouter`) 연동
- 서버에서는 월드 UUID 기준 입장 패킷을 받아 해당 월드로 라우팅

### 3) EntityRouter
`EntityRouter`는 고빈도 패킷을 이벤트 버스 전체 브로드캐스트 대신 **엔티티 직접 라우팅**합니다.

- `PlayerInputPacket` → 해당 플레이어 `PlayerMovement`
- `KeyPacket`(줍기 입력) → `PlayerPickup`
- `SkillUsingPacket` → `SkillManager`

이 구조는 이벤트 기반의 느슨한 결합을 유지하면서, 입력 핫패스는 빠르게 처리하도록 설계된 절충안입니다.

---

## 패킷 설계 원칙

모든 패킷은 `network::Packet`을 상속하며,

- 클래스별 `Serialize/Deserialize` 구현
- `TypeTraits::GetTypeHash<T>()` 기반 ID 자동 생성

을 사용합니다. 즉, 중앙 enum 관리 없이 패킷 타입 자체로 식별자를 안정적으로 생성하는 구조입니다.

---

## 라이프사이클 요약

1. 클라이언트 UDP Join 요청
2. 서버가 토큰 발급 후 UDP로 전달
3. 클라이언트 TCP 연결 후 토큰 재전송
4. 서버가 DB 로드/유저 객체 생성
5. `ChangeWorldPacket` 전달
6. 클라이언트가 월드 입장 패킷 전송
7. 서버가 월드 내 스폰 상태 브로드캐스트

---

## 설계 포인트 요약

- **공유 코드 + 매크로 분기**로 유지보수성 확보
- **Event Bus + Router 혼합**으로 확장성과 성능 균형
- **Authoritative server** 원칙 유지
- **월드 단위 책임 분리**로 멀티월드 확장 가능
