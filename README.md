# MapleProject

메이플스토리 스타일 게임플레이를 ShellEngine 기반으로 구현한 **서버-클라이언트 멀티플레이 프로젝트**입니다.

## 핵심 요약
- C++17 / CMake 기반
- UDP + TCP 혼합 네트워크 구조
- 서버 권한(Authoritative) 시뮬레이션
- 멀티월드, 플레이어 예측/재조정, 몬스터 AI(FSM), 스킬/히트, 아이템/인벤토리(DB)

## 기술 문서 모음
- [플레이어 예측/재조정](./PlayerPrediction.md)
- [시스템 아키텍처](./docs/ArchitectureOverview.md)
- [네트워크 세션/데이터 흐름](./docs/NetworkSessionAndDataFlow.md)
- [전투/몬스터/아이템 루프](./docs/GameplayLoop_MobSkillItem.md)

## 빌드 개요
- 루트 `CMakeLists.txt` 기준으로 `Source`, `EditorSource`, `External/SQLite`를 함께 빌드합니다.
- 클라이언트/서버는 공용 코드(`Source/common`)를 공유하고, `SH_SERVER` 매크로로 런타임 동작을 분기합니다.

## 참고
- 에셋은 저장소에 일부만 포함되어 있으며, 실행 환경에 따라 별도 리소스가 필요할 수 있습니다.
