# MapleProject

ShellEngine 기반으로 제작한 2D 멀티플레이 RPG 프로토타입입니다.  
핵심 목표는 **서버 권한 동기화 + 클라이언트 조작감 확보**이며, 이를 위해 Prediction/Reconciliation 구조를 중심으로 설계했습니다.

## 빠른 소개
- **Language**: C++17
- **Engine**: ShellEngine
- **Network**: UDP + TCP
- **Platform**: Windows, Linux
- **핵심 기능**: 멀티월드, 플레이어 예측 이동, 스킬/히트, 아이템 드랍/획득, 인벤토리 DB 동기화

## 문서 가이드
README는 핵심만 간단히 소개하고, 상세 기술 문서는 아래로 분리했습니다.

- 플레이어 예측/재조정 상세: [`PlayerPrediction.md`](./PlayerPrediction.md)
- 시스템 아키텍처/네트워크/DB 설계: [`TechnicalOverview.md`](./TechnicalOverview.md)

## 데모
- 영상: https://youtu.be/-ZEik9VYwIc

<img width="1070" height="849" alt="멀티플레이 테스트" src="https://github.com/user-attachments/assets/32fd5787-f8d8-4a80-8e09-69b818bb705c" />

## 빌드 요약
1. `CMakeLists.txt`의 `ENGINE_DIR`를 로컬 ShellEngine 경로로 설정
2. CMake preset으로 configure/build

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

---
더 자세한 기술 설명은 `TechnicalOverview.md`를 참고해주세요.
