# DB
## 개요
DB는 SQLite 기반이며, 게임 루프와 DB I/O를 분리하기 위해 전용 스레드(`DBThread`)를 사용합니다.

## 주요 컴포넌트
| 컴포넌트 | 역할 |
|---|---|
| `Database` | SQLite 래퍼, Prepared SQL, Query/Execute |
| `DBThread` | DB 작업 큐와 단일 워커 스레드 관리 |
| `UserManager` | 유저 생성/입퇴장/인벤토리 DB 반영 |
| `Inventory` | 슬롯 데이터 모델, 직렬화/부분 직렬화 |

## 사용자 세션과 DB 연동
### 가입/로그인
1. 세션 확정 후 `UserManager::ProcessPlayerJoinTcp`에서 DB 작업을 비동기로 예약합니다.
2. 닉네임 조회 결과가 없으면 사용자 레코드를 생성합니다.
3. 기존 사용자면 마지막 접속 IP/Port를 갱신합니다.
4. 인벤토리를 로드해 `PendingJoin`으로 메인 루프에 전달합니다.
5. 메인 루프(`Tick`)에서 `future` 완료를 확인한 뒤 `User` 객체를 최종 생성합니다.

### 로그아웃/퇴장
1. `ProcessPlayerLeave`에서 인벤토리 DB 반영 작업을 예약합니다.
2. 유저 제거 전 이벤트를 발행하고, 컨테이너/메모리풀에서 정리합니다.

## 인벤토리 저장
- `Inventory`는 변경된 슬롯 인덱스를 `dirtySlots`로 추적합니다.
- DB 반영 시 더티 슬롯만 순회하여 `UPSERT` 또는 `DELETE`를 수행합니다.
- 트랜잭션(`BEGIN -> COMMIT`, 실패 시 `ROLLBACK`)으로 원자성을 보장합니다.
- DB 반영 성공 시 `ClearDirtySlots()`로 상태를 확정합니다.

## 비동기 설계 포인트
- 게임 스레드에서 DB 쿼리 대기를 피하기 위해 `std::future` 기반 비동기 완료 확인을 사용합니다.
- `pendingJoins`, `pendingInventory` 큐를 틱에서 폴링해 결과를 안전하게 반영합니다.
- 종료 시 남은 future를 정리해 리소스 누수를 방지합니다.

## 스키마
- 유저: `Users`
  - id nickname lastIP lastPort lastWorld
- 유저 인벤토리: `UserInventory`
  - ownerId + slotIdx 기준 저장
  - 슬롯 단위 업데이트에 최적화

## 참고
- DB 파일 경로는 `DBThread::Init("users.db")`로 초기화됩니다.
- 서버 실행 환경에서 DB 파일 쓰기 권한이 필요합니다.
