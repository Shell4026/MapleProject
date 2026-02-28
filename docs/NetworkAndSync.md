# 네트워크 및 동기화
## 개요
네트워크는 용도에 따라 UDP와 TCP를 분리합니다.

- UDP: 고빈도 상태/입력(손실 허용, 저지연)
- TCP: 접속/월드 이동/인벤토리 등 신뢰 전송 필요 이벤트

## 연결 과정
1. 클라이언트가 `PlayerJoinPacket`(UDP)을 전송합니다.
2. 서버는 임시 토큰(`PlayerTokenPacket`)을 UDP로 반환합니다.
3. 클라이언트는 TCP 연결 후 동일 토큰을 TCP로 전송합니다.
4. 서버 `UserManager`가 UDP 엔드포인트와 TCP 소켓을 매칭해 유저 세션을 확정합니다.
5. 서버는 `InventorySyncPacket`과 `ChangeWorldPacket`(TCP)을 전송합니다.
6. 클라이언트는 해당 월드에 `PlayerJoinWorldPacket`(TCP)으로 입장 요청합니다.

## 패킷 분류
| 영역 | 패킷 예시 | 채널 |
|---|---|---|
| 세션 | `PlayerJoinPacket`, `PlayerTokenPacket`, `PlayerLeavePacket` | UDP + TCP |
| 월드 | `PlayerJoinWorldPacket`, `ChangeWorldPacket` | TCP |
| 플레이어 이동 | `PlayerInputPacket`, `PlayerStatePacket` | UDP |
| 생존 감시 | `HeartbeatPacket` | UDP |
| 전투 | `SkillUsingPacket`, `SkillStatePacket`, `MobHitPacket` | UDP |
| 오브젝트 | `PlayerSpawnPacket`, `PlayerDespawnPacket`, `MobSpawnPacket`, `MobStatePacket`, `ItemDropPacket` | UDP |
| 인벤토리 | `InventorySlotSwapPacket`, `InventorySyncPacket` | TCP |

## 플레이어 동기화 모델
### 클라이언트
- 입력 변경 시점에만 `PlayerInputPacket`을 전송해 대역폭을 절약합니다.
- 즉시 로컬 시뮬레이션을 수행하여 반응성을 확보합니다.
- `PlayerStatePacket` 수신 시 과거 틱 히스토리를 기준으로 재조정합니다.

### 서버
- 입력 패킷을 도착 즉시 적용하지 않고, 클라-서버 틱 오프셋을 추정해 미래 서버 틱에 예약 적용합니다.
- 주기적으로 `PlayerStatePacket`을 브로드캐스트합니다.
- 패킷에는 `lastProcessedInputSeq`, `clientTickAtState`가 포함되어 클라이언트 재조정 기준점이 됩니다.

## 오프셋 보정
- 서버는 `offset = tick - packet.tick + 5`를 초기 추정값으로 사용합니다.
- 이후 이동 평균 형태(`offset = (offset * 9 + newOffset) / 10`)로 완만하게 보정합니다.
- 이동과 스킬 모두 동일한 틱을 사용합니다.

## 원격 플레이어 보간
- 로컬 플레이어가 아닌 경우 서버 위치를 목표값으로 저장하고, 프레임마다 선형 보간합니다.
- 서버 권한 상태를 유지하면서 시각적 떨림을 완화합니다.

## 장애 대응
- 하트비트 기반으로 비정상 종료 유저를 감지합니다.
- 서버는 하트비트가 0이 된 유저를 정리하고 퇴장 이벤트를 발행합니다.

## 관련 상세 문서
- [플레이어 예측-재조정 상세](../PlayerPrediction.md)
