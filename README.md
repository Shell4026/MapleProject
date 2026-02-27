# MapleProject

> ShellEngine 기반으로 구현한 **2D MMORPG 스타일 멀티플레이 게임 프로토타입**입니다.  
> 단순 기능 나열이 아니라, **서버 권한 모델 / 예측-재조정 이동 / 이벤트 기반 라우팅 / 비동기 DB 저장**까지 포함한 기술 데모 성격의 프로젝트입니다.

---

## 1) 프로젝트 개요

### 프로젝트 성격
- 장르: 2D 횡스크롤 MMORPG 모작/프로토타입
- 아키텍처: 서버-클라이언트 분리 + 공통 로직 공유(`Source/common`)
- 핵심 목표:
  1. 네트워크 지연 환경에서도 조작감을 유지하는 플레이어 이동
  2. 서버 권한(authoritative server) 기반 동기화
  3. 상태 저장(인벤토리)과 월드 동기화를 포함한 실전형 멀티플레이 루프 구현

### 기술 스택
- **Language**: C++17
- **Engine**: [ShellEngine](https://github.com/Shell4026/ShellEngine)
- **Build**: CMake + Ninja (Preset 제공)
- **Network**: UDP + TCP 혼합 모델
- **Data**: SQLite3 (유저/인벤토리 영속화)
- **Library**: `fmt`, `glm`, `nlohmann_json`, `sqlite3`

### 지원 플랫폼
- Windows / Linux (CMakePresets 기반)

---

## 2) 데모 & 참고 문서

- 시연 영상: https://youtu.be/-ZEik9VYwIc
- 플레이어 예측-재조정 상세: [`PlayerPrediction.md`](./PlayerPrediction.md)

<img width="1913" height="1029" alt="엔진2" src="https://github.com/user-attachments/assets/edf87c9e-08ed-45e9-8344-39c894548087" />
윈도우
<img width="1027" height="796" alt="리눅스" src="https://github.com/user-attachments/assets/fbb6d71c-f228-483b-97e1-7e53c458fdc9" />
리눅스
<img width="1070" height="849" alt="스크린샷 2025-09-15 155939" src="https://github.com/user-attachments/assets/32fd5787-f8d8-4a80-8e09-69b818bb705c" />
멀티플레이 테스팅

---

## 3) 시스템 아키텍처

## 3-1. 빌드 타겟 분리 전략
이 프로젝트는 클라이언트와 서버를 물리적으로 분리하되, 공통 코드는 재사용하는 구조입니다.

- `ShellEngineUser`(클라이언트 DLL): `Source/client + Source/common`
- `ShellEngineUserServer`(서버 DLL): `Source/server + Source/common`
- `SH_SERVER` 컴파일 매크로로 서버 전용 로직 분기

즉, 동일한 도메인 모델(Player/Item/Mob/World)을 공유하면서도 네트워크 및 시뮬레이션 책임은 타겟별로 분리합니다.

## 3-2. 런타임 컴포넌트 구성
- **MapleServer**: UDP 수신/브로드캐스트, TCP 리스닝, 사용자 세션/토큰 핸드셰이크, 월드 로딩
- **MapleClient**: UDP 송수신, TCP 연결, 패킷 디스패치, 로컬 유저 상태 보관
- **MapleWorld (서버/클라 분기)**:
  - 서버: 플레이어 스폰/디스폰, 아이템 드롭/회수, 월드 Tick 관리
  - 클라: 스폰/디스폰 패킷 수신 후 오브젝트 생성, 카메라 로컬 플레이어 바인딩

## 3-3. 이벤트 기반 결합도 완화
패킷은 수신 즉시 단일 컴포넌트가 직접 처리하지 않고 `EventBus`에 게시됩니다.

- 장점:
  - 네트워크 레이어와 게임 로직의 참조 분리
  - 기능 추가 시 구독자만 확장하면 되어 유지보수 용이
- 성능 경로:
  - 플레이어/엔티티 동기화는 `EntityRouter`를 통해 라우팅하여 처리량 확보

---

## 4) 네트워크 설계 (UDP + TCP 하이브리드)

### 왜 하이브리드인가?
- **UDP**: 위치/상태처럼 손실 허용 가능한 고빈도 데이터 전송
- **TCP**: 월드 이동, 인벤토리 동기화 같은 신뢰성 필수 데이터 전송

### 접속 핸드셰이크 시퀀스
1. 클라이언트가 UDP `PlayerJoinPacket` 송신
2. 서버가 토큰(UUID) 발급 후 `PlayerTokenPacket`(UDP) 반환
3. 클라이언트가 TCP 연결 후 동일 토큰 송신
4. 서버가 pending 소켓과 토큰을 매칭해 최종 세션 확정

이 과정을 통해 UDP endpoint와 TCP 세션을 안전하게 귀속시킵니다.

### 연결 생존성 관리
- 클라: 1초마다 Heartbeat 전송
- 서버: heartbeat 카운트 감소 및 0 도달 시 유저 정리
- 비정상 종료에도 유저 메모리/월드 엔티티 누수 없이 정리되도록 설계

---

## 5) 월드/엔티티 동기화

### 멀티 월드
- 서버는 `loadedWorlds` 목록을 순회하며 월드를 additive 모드로 로드
- 유저 입장 시 첫 월드 UUID를 할당하고 `ChangeWorldPacket`으로 클라에 통지

### 플레이어 스폰 동기화
유저가 월드에 진입하면 서버는 아래 순서로 처리합니다.
1. 신규 유저에게 기존 플레이어 목록(`PlayerSpawnPacket`) 선전송
2. 신규 유저 플레이어를 서버 월드에 생성
3. 기존 유저들에겐 `bLocal=false` 상태로 브로드캐스트
4. 신규 유저 본인에게는 `bLocal=true`로 별도 송신 (카메라 바인딩 트리거)

### 아이템 라이프사이클
- 드롭 시: 서버가 아이템 엔티티를 활성화하고 `ItemDropPacket` 브로드캐스트
- 획득/소멸 시: 비활성화 후 sleep queue 보관
- 일정 Tick 이상 미사용 시 sleep queue를 청소하여 객체 완전 제거

> 즉시 파괴 대신 재사용 풀링(sleepItems)을 사용해 빈번한 생성/삭제 비용을 줄였습니다.

---

## 6) 플레이어 이동 품질: Prediction & Reconciliation

본 프로젝트의 핵심 기술 포인트는 **입력 예측 + 서버 재조정**입니다.

- 클라: 입력 시 즉시 로컬 시뮬레이션 수행(지연 체감 최소화)
- 서버: 권한 있는 정답 상태를 주기적으로 전송
- 클라: 서버 상태와 과거 입력 버퍼를 기준으로 오차 보정 후 재시뮬레이션

추가로, 기존 물리기반 지형 대신 **foothold(선 기반 발판)** 시스템을 도입해 결정적 재시뮬레이션 안정성을 강화했습니다.  
구체 메커니즘/비교 실험은 [`PlayerPrediction.md`](./PlayerPrediction.md)에 정리되어 있습니다.

---

## 7) 데이터 저장(인벤토리/유저)

### DB 처리 방식
- SQLite 사용 (`users.db`)
- DB 작업은 `DBThread` 단일 워커 스레드에서 비동기 처리
- 로그인 시:
  - 유저 조회 → 없으면 생성 / 있으면 접속 정보 업데이트
  - 유저 인벤토리 로드 후 세션 객체에 주입

### 인벤토리 동기화 전략
- 슬롯 기반(기본 32칸)
- Dirty Slot 추적(`dirtySlots`, `dirtyMask`)
- 전체 동기화 대신 변경 슬롯만 `SerializeDirtySlots()`로 전송/저장
- DB 저장 시 트랜잭션(`BEGIN/COMMIT/ROLLBACK`) 사용

이 구조로 네트워크와 DB 양쪽 모두 불필요한 쓰기를 줄였습니다.

---

## 8) 디렉토리 구조

```text
.
├─ Source/
│  ├─ common/      # 클라/서버 공통 도메인 및 인프라
│  ├─ client/      # 클라이언트 전용 구현
│  ├─ server/      # 서버 전용 구현
│  └─ include/     # 공용 헤더
├─ EditorSource/   # 에디터 인스펙터 확장
├─ External/SQLite # SQLite 소스 포함
├─ Assets/         # 프리팹, 월드, 셰이더, 텍스처 등
├─ PlayerPrediction.md
└─ README.md
```

---

## 9) 빌드 및 실행

## 9-1. 사전 준비
1. ShellEngine 바이너리/헤더가 준비되어 있어야 합니다.
2. 루트 `CMakeLists.txt`의 `ENGINE_DIR`를 로컬 환경에 맞게 수정합니다.

```cmake
set(ENGINE_DIR C:/ShellEngine/out/build/x64-release/bin)
```

## 9-2. Configure & Build 예시

### Linux
```bash
cmake --preset linux-debug
cmake --build out/build/linux-debug
```

### Windows
```bash
cmake --preset x64-debug
cmake --build out/build/x64-debug
```

## 9-3. 실행 개요
- 서버 타겟(`ShellEngineUserServer`) 먼저 실행
- 클라이언트 타겟(`ShellEngineUser`) 실행 후 접속
- 기본 월드/시작 월드는 `ProjectSetting.json` 기준으로 로딩

---

## 10) 구현 포인트 요약 (포트폴리오용)

- **서버 권한 + 클라 예측 재조정**으로 네트워크 지연 체감 완화
- **UDP/TCP 분리 설계**로 실시간성/신뢰성 균형 확보
- **EventBus + Router**를 통한 로직 결합도 감소
- **비동기 DB 파이프라인**으로 런타임 프레임 드랍 리스크 완화
- **Dirty Slot 기반 인벤토리 동기화**로 대역폭/DB 쓰기 최적화
- **오브젝트 재사용(sleep queue)**으로 월드 아이템 생성/파괴 비용 절감

---

## 11) 향후 개선 아이디어

- 월드 샤딩/채널 서버 확장
- 치트 대응(서버 검증 룰 강화, 속도핵/텔레포트 탐지)
- 인벤토리/아이템 트랜잭션에 대한 장애 복구 시나리오 고도화
- 리플레이/로그 기반 디버깅 툴링 추가
- CI 파이프라인(정적 분석 + 포맷 + 단위 테스트) 자동화

---

### Appendix
이 문서는 코드베이스 구조를 분석해 기술 문서 톤으로 재작성한 README입니다.  
프로젝트의 핵심 강점은 “게임 기능 구현” 그 자체보다, **실시간 멀티플레이 환경에서의 아키텍처 의사결정**에 있습니다.
