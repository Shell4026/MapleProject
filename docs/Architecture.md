# 아키텍처
## 개요
해당 프로젝트는 `SH_SERVER` 전처리 매크로와 CMake 타겟 분리를 통해 클라이언트/서버 실행 경계를 나눕니다.  
`Source/common`은 양측에서 공유되고, `Source/client`, `Source/server`는 각 실행 환경에만 포함됩니다.

## 모듈 경계
| 계층 | 주요 타입 | 역할 |
|---|---|---|
| 세션/전송 | `MapleClient`, `MapleServer` | UDP/TCP 소켓 관리, 패킷 수신 루프, EventBus 발행 |
| 월드 단위 | `MapleWorld` | 월드별 플레이어/아이템/포털/몹 생명주기 관리 |
| 엔티티 제어 | `EntityRouter` | 패킷을 플레이어 컴포넌트로 라우팅 |
| 도메인 로직 | `Player*`, `Mob*`, `Skill*`, `Item*` | 플레이어 이동, AI, 스킬, 아이템 처리 |
| 데이터 | `UserManager`, `Inventory`, `DBThread`, `Database` | 유저 세션/인벤토리/DB 동기화 |

## 서버 구성
- `MapleServer`는 UDP 서버와 TCP 리스너를 동시에 운용합니다.
- `loadedWorlds`에 등록된 월드를 Additive로 로드하여 로드된 모든 월드에 대해 시뮬레이션을 수행합니다.
- 수신 패킷은 `network::PacketEvent`로 `bus`에 발행되고, 월드/라우터/기타 컴포넌트가 구독해 처리합니다.
- 유저 관리는 `UserManager`가 담당하며, 하트비트/입퇴장/DB 반영 흐름을 포함합니다.

## 클라이언트 구성
- `MapleClient`는 UDP 클라이언트 + 별도 TCP 소켓 스레드로 구성됩니다.
- 패킷 수신 후 EventBus에 발행하고, 월드/플레이어/UI가 필요한 이벤트만 구독합니다.
- 월드 진입은 `PlayerJoinWorldPacket`(TCP) 기반으로 시작됩니다.

## 공통 Tick 모델
- `PlayerTickController`가 `TickBegin -> TickFixed -> TickUpdate` 순서를 보장합니다.
- 클라이언트 예측/서버 권한 상태 계산 모두 해당 틱 기반으로 동작합니다.
- 결정적 처리 단위를 유지해 재조정(Reconciliation) 시 오차를 줄입니다.

## 패킷 라우팅 설계
### EventBus
- 네트워크 계층과 게임 로직의 직접 결합을 줄이기 위한 기본 이벤트 채널입니다.
- 월드/엔티티/시스템 컴포넌트는 필요한 패킷 이벤트만 구독합니다.

### EntityRouter
- 플레이어 단위 입력(`PlayerInputPacket`, `KeyPacket`, `SkillUsingPacket`)을 플레이어 객체로 직접 전달합니다.
- 서버 월드(`MapleWorld`)에서 플레이어 Spawn/Despawn 시 라우터 등록/해제를 동기화합니다.

## 월드/엔티티 라이프사이클
1. 유저 접속 후 서버가 월드를 지정합니다.
2. 클라이언트는 대상 월드에 `PlayerJoinWorldPacket`을 전송합니다.
3. 서버 `MapleWorld`가 플레이어를 스폰하고 기존 엔티티 스냅샷을 동기화합니다.
4. 런타임 중 패킷과 Tick을 통해 상태가 갱신됩니다.
5. 퇴장/월드 이동 시 Despawn 후 대상 월드에서 새로 스폰 됩니다.

## 참조 경로
| 경로 | 설명 |
|---|---|
| `Source/server/World` | 서버 월드/라우팅/포털/브로드캐스트 |
| `Source/client/World` | 클라이언트 월드 스폰/동기화 |
| `Source/common/Player` | 공통 플레이어/틱/인벤토리 기본 로직 |
| `Source/server/Player` | 서버 유저 세션/검증/DB 연동 |
| `Source/include/Packet` | 네트워크 패킷들 |
