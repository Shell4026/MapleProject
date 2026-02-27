<img width="1913" height="1029" alt="엔진2" src="https://github.com/user-attachments/assets/edf87c9e-08ed-45e9-8344-39c894548087" />
윈도우
<img width="1027" height="796" alt="리눅스" src="https://github.com/user-attachments/assets/fbb6d71c-f228-483b-97e1-7e53c458fdc9" />
리눅스
<img width="1070" height="849" alt="스크린샷 2025-09-15 155939" src="https://github.com/user-attachments/assets/32fd5787-f8d8-4a80-8e09-69b818bb705c" />
멀티플레이 테스팅

https://youtu.be/-ZEik9VYwIc

# 개요
유명한 MMORPG 메이플스토리를 ShellEngine을 이용해 모작 해본 프로젝트입니다.</br>
서버-클라이언트 구조로, 클라이언트가 입력 패킷을 보내고 서버가 처리 후 계산하여 상태 패킷을 클라이언트에게 보냅니다.</br>
매크로와 CMake 타겟을 통해 서버 사이드와 클라이언트 사이드 코드를 분리하여 관리합니다.</br>

- 개발 언어: C++17
- 엔진: [ShellEngine](https://github.com/Shell4026/ShellEngine) (직접 개발, CMake 빌드 관리)
- 네트워크: ASIO 기반 UDP, TCP
- 지원 플랫폼: Windows, Linux
- 구현 기능: 멀티월드, 플레이어 이동/보간, NPC 대화창, 인벤토리(서버와 DB연결), FSM기반 몬스터 AI, 스킬/히트 시스템, 아이템

에셋은 포함 돼 있지 않습니다

# 상세
## 주요 시스템
- **멀티 월드 서버**
  - 기존 ShellEngine이 단일 월드만 업데이트하던 구조를 개선해 여러 월드를 동시에 시뮬레이션 가능하도록 확장
  - 서버에서는 렌더링 관련 컴포넌트를 비활성화하여 리소스를 절약
- **낮은 결합도 - Event Bus, Router**
  - 중앙 Event Bus를 도입하여 네트워크 레이어와 게임 로직 컴포넌트간의 직접 참조를 제거
  - 수신된 패킷을 PacketEvent로 발행하면 필요한 컴포넌트만 구독해서 처리
  - 성능이 중요한 곳은 수신한 패킷을 Router가 대신 받아서 필요한 곳에 전달
- **패킷 ID 자동 부여**
  - 클래스명 기반 컴파일타임 해시를 사용해 패킷 ID를 자동으로 생성(런타임 오버헤드 없음)
- **AI: 전략 패턴 + FSM**
  - AIStrategy 인터페이스로 행동 패턴을 추상화하여 확장성을 확보
  - 서버에서 FSM 기반으로 AI를 실행하고, 클라이언트에는 상태/위치만 전송 후 선형 보간
- **동적 폰트 생성**
  - 채팅/이름표 등 런타임에 등장하는 유니코드 문자에 대응하기 위해 필요 시 폰트 아틀라스(이미지)를 동적으로 생성하고 교체
  - shared_ptr을 통해 폰트 객체를 전역적으로 관리하고 모든 사용처가 사라질 시 제거
- **애니메이터와 애니메이션**
  - FSM기반 애니메이터
  - 조건에 따라 애니메이션을 수행
  
## 플레이어
- **예측-재조정(Prediction - Reconciliation) 기반 움직임** [상세](https://github.com/Shell4026/MapleProject/blob/master/PlayerPrediction.md)
  - 100ms 지연 상황에서도 끊김 없는 플레이어 움직임
- **대역폭 최적화**
  - 매 프레임 패킷을 전송하는 대신 입력 상태가 변경될 때 위주로 PlayerInputPacket을 전송
- **바닥 감지 foothold system**
- **플레이어 입장 / 퇴장**
  - 플레이어가 접속하면 서버측에서 모든 유저에게 통일된 UUID를 부여
  - 비정상 종료 대비: 클라이언트는 1초마다 하트비트 전송, 서버는 주기적으로 하트비트를 감소/증가 관리
- **스킬 & 히트 시스템**
  - 클라이언트는 스킬 사용 시 우선 애니메이션을 재생하고 SkillUsingPacket을 서버로 전송
  - 서버에서 스킬 발동을 처리 후 SkillStatePacket을 브로드캐스트하며, 히트박스 충돌이 발생하면 MobHitPacket을 전송
  - 몹에 가해진 충격(넉백, 경직) 등은 서버에서 물리적으로 처리
