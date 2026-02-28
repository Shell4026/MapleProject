# MapleProject
<img width="1070" height="849" alt="스크린샷 2025-09-15 155939" src="https://github.com/user-attachments/assets/32fd5787-f8d8-4a80-8e09-69b818bb705c" />

ShellEngine 기반 C++17 멀티플레이 메이플스토리 모작 프로젝트입니다.  
클라이언트-서버 분리 구조에서 플레이어/몹/아이템/월드 상태를 동기화하고, 예측-재조정 기반 이동을 적용합니다.

## 요약
- 언어: C++17
- 엔진: [ShellEngine](https://github.com/Shell4026/ShellEngine)
- 네트워크: UDP(실시간 상태) + TCP(세션/신뢰 전송)
- DB: SQLite, 비동기 DB 작업 스레드
- 플랫폼: Windows, Linux

## 핵심 특징
- 멀티 월드 서버 시뮬레이션과 월드 간 포털 이동
- EventBus + Router 기반 패킷 라우팅(낮은 결합도)
- 플레이어 입력 예측/재조정(Prediction/Reconciliation)
- FSM 기반 몹 AI, 스킬/히트박스 처리, 아이템 드롭/획득
- 인벤토리 변경 슬롯 기반 동기화 + DB 반영

## 구조
| 경로 | 역할 |
|---|---|
| `Source/client` | 클라이언트 전용 로직 |
| `Source/server` | 서버 전용 로직 |
| `Source/common` | 클라/서버 공통 로직 |
| `Source/include` | 공유 헤더(도메인/패킷 정의) |
| `Assets` | 월드/프리팹/리소스(저장소 기준 일부만 포함) |

## 상세 문서
- [아키텍처](docs/Architecture.md)
- [네트워크 및 동기화](docs/NetworkAndSync.md)
- [게임플레이 시스템](docs/GameplaySystems.md)
- [DB](docs/DataPersistence.md)
- [플레이어 예측-재조정 상세](docs/PlayerPrediction.md)

## 빌드 참고
- 루트 `CMakeLists.txt`에서 `Source`, `EditorSource`, `External/SQLite`를 함께 구성합니다.
- `CMakePresets.json` 기준 주요 프리셋:
  - `x64-debug`, `x64-release`
  - `linux-debug`, `linux-release`

## 데모
- 테스트 영상: https://youtu.be/-ZEik9VYwIc
