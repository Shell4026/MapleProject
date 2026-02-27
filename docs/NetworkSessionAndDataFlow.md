# 네트워크 세션/데이터 흐름 기술 문서

## 개요
본 문서는 로그인 이후 실제 플레이까지 이어지는 **세션 수립**, **유저 상태 유지**, **인벤토리 영속화** 경로를 다룹니다.

---

## 전송 채널 분리 전략

| 채널 | 용도 | 이유 |
|---|---|---|
| UDP | 입력/상태/하트비트 | 지연 최소화, 손실 허용 가능 |
| TCP | 토큰 확정, 인벤토리 동기화, 종료 처리 | 신뢰성 필요 |

핵심은 “빠른 데이터는 UDP, 정확해야 하는 데이터는 TCP”입니다.

---

## 접속 핸드셰이크

### 단계별 흐름
1. 클라이언트가 `PlayerJoinPacket`을 UDP로 전송
2. 서버가 임시 토큰(UUID) 발급 (`PlayerTokenPacket`)
3. 클라이언트가 같은 토큰을 TCP로 다시 전송
4. 서버가 `pendingUsers`와 대조 후 세션 확정
5. DB에서 유저/인벤토리 로드 비동기 작업 등록
6. 완료 시 실제 `User` 객체 생성 및 Join 이벤트 발행

### 왜 2단계인가?
- UDP endpoint와 TCP endpoint를 동일 사용자로 묶어야 함
- 토큰 기반 재확인으로 임의 TCP 연결 오염 가능성 완화

---

## UserManager 내부 상태 머신

`UserManager`는 대략 다음 큐/맵으로 운영됩니다.

- `pendingUsers`: UDP 등록만 끝난 임시 유저
- `pendingJoins`: DB 로딩 future 목록
- `users` + `userUUIDs`: 활성 유저 인덱스
- `userUdpEndpoints`: endpoint ↔ user UUID 매핑
- `pendingInventory`: 인벤토리 DB 반영 비동기 큐

### Tick에서 처리되는 것
- 완료된 Join future를 회수해 유저 활성화
- 인벤토리 업데이트 future 완료 처리(성공 시 dirty clear)
- 하트비트 기반 타임아웃 유저 정리

---

## 하트비트/접속 종료

- 클라이언트: 1초마다 `HeartbeatPacket` 송신
- 서버: 하트비트 카운터 감소, 패킷 수신 시 리셋
- 카운터 0 도달 시 강제 정리(leave 처리)

비정상 종료(프로세스 kill, 네트워크 단절) 상황에서도 세션 청소가 가능합니다.

---

## 인벤토리 동기화와 DB 영속화

### 동기화
- 접속 직후 전체 인벤토리 JSON 송신 (`InventorySyncPacket`)
- 슬롯 변경 시 dirty slot만 직렬화하여 부분 동기화

### 영속화
- `DBThread` 단일 작업 스레드에서 SQL 비동기 실행
- 사용자 종료 시 dirty slot 중심 upsert/delete 반영
- DB 업데이트 성공 시 dirty 상태 clear

### 장점
- 메인 게임 루프에서 DB 블로킹 최소화
- 네트워크 대역폭 절감(부분 동기화)

---

## 장애/경계 상황 대응

- 중복 UDP endpoint 접속 차단
- 유효하지 않은 토큰 TCP 인증 거절
- Join 처리 중 유저 이탈 시 pending 결과 폐기
- 유저 삭제 시 swap-pop 인덱스 재배치로 O(1) 가까운 정리

---

## 요약

이 프로젝트의 세션 계층은

- **토큰 기반 UDP/TCP 결합 인증**
- **비동기 DB 파이프라인**
- **하트비트 기반 자동 정리**

를 조합해 실시간성과 신뢰성을 동시에 확보합니다.
