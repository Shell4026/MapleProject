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
매크로를 통해 서버 사이드와 클라이언트 사이드 코드를 분리하며 CMake를 통해 빌드를 관리합니다.</br>

- 개발 언어: C++17
- 엔진: [ShellEngine](https://github.com/Shell4026/ShellEngine) (직접 개발, CMake 빌드 관리)
- 네트워크: ASIO 기반 UDP
- 지원 플랫폼: Windows, Linux
- 구현 기능: 멀티월드, 플레이어 이동/보간, FSM AI, 스킬/히트 시스템, 이벤트 버스

에셋은 포함 돼 있지 않습니다

# 상세
## 서버
서버는 여러 맵을 전부 업데이트 해야합니다. 따라서 시작 시 모든 월드를 로드 후 업데이트 합니다.</br>
여담으로, 처음 만들 당시 엔진은 월드를 하나만 불러올 수 있는 구조여서 엔진의 개량이 필요했습니다.</br>

<details><summary>코드 펼치기</summary>
  
```c++
SH_USER_API void MapleServer::Start()
{
  // 유니티로 치면 LoadScene에 Addtive모드로 씬을 하나 더 불러오는 기능
  // 불러온 월드들이 같이 업데이트 루프를 돌게 됩니다.
  Super::Start();
  for (auto world : loadedWorlds)
  {
    SH_INFO_FORMAT("loading other world...({})", world->GetUUID().ToString());
    world->SubscribeEvent(componentSubscriber); // 서버는 렌더러 같은 컴포넌트가 필요 없으므로 컴포넌트 추가 이벤트가 들어오면 캔슬 시키는데 씁니다.
    if (world != nullptr)
      GameManager::GetInstance()->LoadWorld(world->GetUUID(), GameManager::LoadMode::Additive, true);
  }
}
```

</details>

## 플레이어 조작
PlayerMovement2D 컴포넌트가 역할을 수행합니다.</br>
처음 개발 할 땐 매 프레임 클라이언트측에서 어느 키를 누르고 있는지 서버로 전송했습니다.</br>
하지만 상당히 비효율적인 구조라는 것을 깨닫고 키의 변화가 있었을 때만 서버로 패킷을 전송하는 구조로 변경하였습니다.</br>
<details><summary>코드 펼치기</summary>

```c++
// 클라이언트 측
bool bInputChanged = false;
bInputChanged = (xInput != lastSent.inputX) || (bJump != lastSent.bJump || (bProne != lastSent.bProne));

if (bInputChanged)
{
  PlayerInputPacket packet{};
  packet.inputX = xInput;
  packet.bJump = bJump;
  packet.seq = inputSeqCounter++;
  packet.playerUUID = client->GetUser().GetUserUUID().ToString();
  packet.timestamp = tick;
  packet.bProne = bProne;

  lastSent = std::move(packet);

  client->SendPacket(lastSent);
}
```
</details>

패킷을 받은 서버는 상태를 키 눌림 상태를 변화 시키며 클라이언트측과 동일한 환경에서 플레이어 캐릭터의 물리를 계산합니다.</br>
서버는 특정 조건(1초에 30번 또는 중요 이벤트 발생)이 되면 클라이언트들에게 패킷을 브로드 캐스팅합니다.
<details><summary>코드 펼치기</summary>

```c++
// 플레이어 입력 패킷을 받아 현재 상태를 바꾸는 코드
void PlayerMovement2D::ProcessInputPacket(const PlayerInputPacket& packet, const Endpoint& endpoint)
{
  if (!core::IsValid(rigidBody))
    return;
  if (packet.playerUUID != player->GetUserUUID().ToString())
    return;
  if (lastInput.seq >= packet.seq) // 과거 패킷임
    return;

  if (lastInput.xMove != packet.inputX)
    lastInput.xMove = packet.inputX;
  if (lastInput.bJump != packet.bJump)
    lastInput.bJump = packet.bJump;
  if (lastInput.bProne != packet.bProne)
    lastInput.bProne = packet.bProne;
  lastInput.tick = packet.timestamp;
  lastInput.seq = packet.seq;
}
```
```C++
// 서버에서 움직임 시뮬레이션
void PlayerMovement2D::ProcessInput()
{
  yVelocity = rigidBody->GetLinearVelocity().y;
  if (bGround)
  {
    if (!lastInput.bProne)
    {
      if (lastInput.xMove == 0 || bLock)
        xVelocity = 0;
      else
        xVelocity = std::clamp(lastInput.xMove * speed, -speed, speed);

      if (lastInput.bJump && !bLock)
      {
        yVelocity = jumpSpeed;
        bGround = false;
      }
    }
    else
    {
      xVelocity = 0.0f;
    }
  }
  else
  {
    float airSpeed = 0.1f;
    if (lastInput.xMove != 0 && !bLock)
    {
      float targetSpeed = lastInput.xMove * speed;
      xVelocity = glm::mix(xVelocity, targetSpeed, airSpeed);
    }
  }
  // 방향
  if (!lastInput.bProne && !bLock)
  {
    if (lastInput.xMove > 0.0f)
      player->SetRight(true);
    else if (lastInput.xMove < 0.0f)
      player->SetRight(false);
  }

  rigidBody->SetLinearVelocity({ xVelocity, yVelocity, 0.f });

  lastProcessedSeq = lastInput.seq;
  lastTick = lastInput.tick;
}
```
```c++
// 서버측에서 보내는 정보
if (bSend)
{
  PlayerStatePacket packet;

  packet.px = pos.x;
  packet.py = pos.y;
  packet.vx = v.x;
  packet.vy = v.y;
  packet.lastProcessedInputSeq = lastProcessedSeq;
  packet.playerUUID = player->GetUserUUID().ToString();
  packet.serverTick = serverTick;
  packet.timestamp = lastTick;
  packet.bGround = bGround;
  packet.floor = floorY;
  packet.bProne = lastInput.bProne;
  packet.bLock = bLock;
  packet.bRight = player->IsRight();
  server->BroadCast(packet);

  lastSent.pos = serverPos;
  lastSent.vel = serverVel;
  lastSent.seq = lastProcessedSeq;

  serverTick = 0;
  bSend = false;
}
```
</details>

해당 패킷을 받은 클라이언트는 위치를 보정하게 됩니다. 하지만 위치 정보를 보내고 결과를 받을 때만 위치를 변경하면 끊김이 느껴지게 됩니다.</br>
따라서 플레이어는 입력 받고 가만히 있는 것이 아닌, 미리 움직임과 동시에 패킷을 받았을 때 위치를 기록하여 매프레임 선형 보간을 하게 됩니다.</br>
자세히 설명하자면, 서버에서 받은 마지막 위치 정보와 속도 값을 기준으로 현재 위치와 속도 값을 선형 보간합니다.</br>
이로서 끊김 없는 부드러운 움직임을 보여줄 수 있습니다.</br>
<details><summary>코드 펼치기</summary>

```c++
// 클라이언측 단순 보간
glm::vec2 corrected = serverPos;
glm::vec2 correctedVel = serverVel;

const float difSqr = glm::distance2(glm::vec2{ gameObject.transform->GetWorldPosition() }, serverPos);
if (difSqr < 1.0f * 1.0f)
{
  corrected = glm::mix(glm::vec2{ gameObject.transform->GetWorldPosition() }, serverPos, 0.2f);
  correctedVel = glm::mix(glm::vec2{ rigidBody->GetLinearVelocity() }, serverVel, 1.0f);
}
gameObject.transform->SetWorldPosition({ corrected.x, corrected.y, player->IsLocal() ? 0.025f : 0.02f });
gameObject.transform->UpdateMatrix();
rigidBody->SetLinearVelocity({ correctedVel.x, correctedVel.y, 0.f });
rigidBody->ResetPhysicsTransform();
```
</details>

## 바닥 감지
바닥을 감지해야 점프를 할 수 있게 만들 수 있기 때문에 플랫포머 게임에서 바닥 감지는 중요합니다.</br>
여기선 raycast + 충돌체크를 혼합하여 구현하였습니다.</br>
여기서 핵심은 플레이어의 발 보다 살짝 위에서 레이캐스팅을 수행하는 것입니다.</br>
그래야 바닥을 뚫고 아래로 내려갔을 때 다른 콜라이더를 감지하지 않습니다.

<details><summary>코드 펼치기</summary>

```c++
// 먼저 바닥 위치와 콜라이더 정보를 저장합니다.
const game::Vec3 pos = rigidBody->GetPhysicsPosition();

phys::Ray ray{ {pos.x, pos.y + 0.4f, pos.z}, Vec3{0.0f, -1.0f, 0.f}, rayDistance };
auto hits = world.GetPhysWorld()->RayCast(ray, tag::groundTag);
if (!hits.empty())
{
  floorY = hits.front().hitPoint.y;
  floor = RigidBody::GetRigidBodyFromHandle(hits.front().rigidBodyHandle);
}
else
{
  floor.Reset();
  floorY = -1000.0f;
  bGround = false;
}
```
```c++
// 레이캐스팅으로 확인했던 콜라이더와 같다면 그건 바닥!
SH_USER_API void PlayerMovement2D::OnCollisionEnter(Collider& collider)
{
  if (!floor.IsValid())
    return;
  if (floor->GetCollider() == &collider)
    bGround = true;
}
```
</details>

## 플레이어 입장 / 플레이어 퇴장

플레이어가 접속하면 서버측에서 모든 유저에게 통일된 UUID를 부여합니다. </br>
이것이 UserUUID로 네트워크 상에서 플레이어 객체가 로컬인지 원격인지 구분 할 수 있게 됩니다.

<details><summary>코드 펼치기</summary>
  
```c++
void MapleWorld::ProcessPlayerJoin(const PlayerJoinWorldPacket& packet, const Endpoint& endpoint)
{
  auto userPtr = server->GetUser(endpoint);
  if (userPtr == nullptr)
    return;

  const auto& userWorldUUID = userPtr->GetCurrentWorldUUID();
  if (userWorldUUID.IsEmpty())
  {
    // 이 월드에 조인 했음
    if (GetUUID() == core::UUID{ packet.worldUUID })
    {
      if (playerSpawnPoint == nullptr)
      {
        SH_ERROR("Invalid spawn point!");
        return;
      }
      userPtr->SetCurrentWorldUUID(GetUUID());

      const auto& spawnPos = playerSpawnPoint->GetWorldPosition();
      // 접속한 플레이어에게 다른 플레이어 동기화
      for (auto& [uuid, playerPtr] : players)
      {
        const auto& playerPos = playerPtr->gameObject.transform->position;
        PlayerSpawnPacket packet;
        packet.x = playerPos.x;
        packet.y = playerPos.y;
        packet.playerUUID = playerPtr->GetUserUUID().ToString();

        server->Send(packet, endpoint.ip, endpoint.port);
      }
      // 접속한 플레이어 생성
      {
        auto player = SpawnPlayer(userPtr->GetUserUUID(), spawnPos.x, spawnPos.y);
        players[userPtr->GetUserUUID().ToString()] = player;

        PlayerSpawnPacket packet;
        packet.x = spawnPos.x;
        packet.y = spawnPos.y;
        packet.playerUUID = userPtr->GetUserUUID().ToString();

        server->BroadCast(packet);
      }
    }
  }
}
```
</details>
플레이어가 비정상적으로 종료 됐을 경우를 감지하는 방법으로, 5초마다 하트비트 패킷을 전송해 응답이 없으면 접속을 종료 하는 로직을 구현했습니다.
<details><summary>코드 펼치기</summary>

```c++
SH_USER_API void Player::Update()
{
// 서버는 플레이어 객체의 목숨을 1초당 1씩 감소 시킵니다.
#if SH_SERVER
  static float t = 0.f;
  t += world.deltaTime;
  if (t >= 1.f)
  {
    heartbeat--;
    t = 0.f;
  }
#else // 클라이언트는 1초마다 하트비트 패킷을 쏩니다.
  if (bLocal)
  {
    static float t = 0.f;
    t += world.deltaTime;
    if (t >= 1.f)
    {
      HeartbeatPacket packet{};
      MapleClient::GetInstance()->SendPacket(packet);
      t = 0.f;
    }
  }
#endif
```
```c++
packetEventSubscriber.SetCallback
(
  [&](const PacketEvent& evt)
  {
    const std::string& ip = evt.senderIp;
    const uint16_t port = evt.senderPort;
    const Endpoint endpoint{ ip, port };
    uint32_t id = evt.packet->GetId();

    if (id == PlayerJoinWorldPacket::ID)
      ProcessPlayerJoin(static_cast<const PlayerJoinWorldPacket&>(*evt.packet), endpoint);
    else if (id == PlayerLeavePacket::ID)
      ProcessPlayerLeave(static_cast<const PlayerLeavePacket&>(*evt.packet), endpoint);
    else if (id == HeartbeatPacket::ID)
      ProcessHeartbeat(endpoint); // 하트비트 패킷을 클라이언트에서 받았다면..
  }
);
```
```c++
void MapleWorld::ProcessHeartbeat(const Endpoint& ep)
{
  User* user = server->GetUser(ep);
  if (user == nullptr)
    return;

  auto it = players.find(user->GetUserUUID().ToString());
  if (it == players.end())
    return;

  Player& player = *it->second;
  player.IncreaseHeartbeat(); // 목숨을 1증가 시킵니다.
}
```
</details>

## 결합도 낮추기
서버나 클라이언트측 컴포넌트는 패킷을 어떻게 받을까요?</br>
단순하게 생각하면 패킷을 받는 클래스를 만들고, 거기에 다른 컴포넌트들을 등록하고 함수를 호출하여 컴포넌트를 호출하는 구조가 있겠습니다.</br>
하지만 그러면 결합도가 너무 심해지므로, 이벤트 버스 패턴을 도입했습니다.</br>
버스로 이벤트를 보내고, 해당 버스를 구독하고 있는 콜백 함수를 호출하여 결합도를 낮춥니다.</br>
<details><summary>코드 펼치기</summary>
  
```c++
// core::EventBus bus;
auto receivedPacket = client.GetReceivedPacket();
while (receivedPacket != nullptr)
{
  PacketEvent evt{}; // 패킷 이벤트를 만들고..
  evt.packet = receivedPacket.get();
  
  bus.Publish(evt); // 버스에 태운다
  
  receivedPacket = client.GetReceivedPacket();
}
```
```c++
//core::EventSubscriber<PacketEvent> packetSubscriber;
packetSubscriber.SetCallback
(
  [&](const PacketEvent& evt)
  {
    if (evt.packet->GetId() == MobSpawnPacket::ID)
      ProcessMobSpawn(static_cast<const MobSpawnPacket&>(*evt.packet));
    else if (evt.packet->GetId() == MobStatePacket::ID)
      ProcessState(static_cast<const MobStatePacket&>(*evt.packet));
    else if (evt.packet->GetId() == MobHitPacket::ID)
      ProcessHit(static_cast<const MobHitPacket&>(*evt.packet));

  }
);
```
</details>

## 몹
몹의 AI는 전략 패턴을 이용해 유연하게 수정할 수 있게 만들었습니다.</br>
몹의 모든 행동 패턴은 AI컴포넌트가 담당하며, 서버에서 처리 후 클라이언트에는 위치 정보와 체력 정보만을 보냅니다.</br>
현재 기본으로 구현된 AI는 FSM(유한상태머신)구조의 AI로, 보스몹 같이 복잡한 AI는 행동 트리를 통해 구현 할 수 있을 것입니다.

<details><summary>코드 펼치기</summary>

```c++
// Mob.h
PROPERTY(ai)
AIStrategy* ai = nullptr;

// Mob.cpp
// Mob::Update()
if (core::IsValid(ai) && !bStun)
  ai->Run(*this);
```
```c++
SH_USER_API void BasicAI::Run(Mob& mob)
{
#if SH_SERVER
  stateTimer -= world.deltaTime;

  switch (state)
  {
  case State::Idle:
    UpdateIdle();
    break;
  case State::Move:
    UpdateMove();
    break;
  case State::Chase:
    UpdateChase(mob);
    break;
  }
  ...
#endif
}
```

</details>

## 스킬 & 몹 히트 시스템
현재 프로젝트는 기본 공격까지 구현 돼 있습니다.</br>
미래를 생각하여 기본 공격도 스킬로 구현했습니다. 이는 유연한 구조로, 나중에 코드의 확장을 쉽게 해줄 것입니다.</br>
플레이어가 키를 누르면 자연스럽게 보이기 위해 우선적으로 애니메이션을 보여줍니다.</br>
그 후 SkillUsingPacket을 서버에 보내 스킬이 발동 됐음을 알립니다.</br>

서버에서 패킷을 전송 받으면 스킬이 발동되고, SkillStatePacket을 클라이언트에게 브로드캐스팅 합니다.</br>
또한 Skill에서 지정한 히트박스가 활성화 돼서 몹과의 충돌을 체크합니다.</br>
충돌이 됐다면 몹이 MobHitPacket을 클라이언트에게 브로드캐스팅 합니다.

<img width="162" height="70" alt="image" src="https://github.com/user-attachments/assets/f430b59a-da8f-4884-93be-aca2e7c07646" /></br>
<img width="341" height="686" alt="image" src="https://github.com/user-attachments/assets/e5c680f2-eb26-43a7-8b81-a9a3f67e635b" />
<img width="343" height="453" alt="image" src="https://github.com/user-attachments/assets/834c76a1-12ab-444f-8afa-d9a9c4470486" />

<details><summary>코드 펼치기</summary>

```c++
// 클라이언트측 Use()
#if !SH_SERVER
  static MapleClient* client = MapleClient::GetInstance();
  if (bCanUse && !bUsing)
  {
    SkillUsingPacket packet{};
    packet.userUUID = client->GetUser().GetUserUUID().ToString();
    packet.skillId = id;
    client->SendPacket(packet);
    PlayAnim();
    animator->SetLock(true);
  }
```
```c++
// 서버측 Use
if (bCanUse && !bUsing)
{
  SH_INFO("Use!");
  if (core::IsValid(playerMovement) && !bCanMove)
    playerMovement->Lock();
  delay = delayMs;
  cooldown = cooldownMs;
  bCanUse = false;
  bUsing = true;
#if SH_SERVER
  hitboxt = hitBoxMs; // 히트 박스 타이머, 해당 시점 이후에 히트 박스가 활성화 됩니다.

  SkillStatePacket packet{};
  packet.userUUID = skillManager->GetPlayer()->GetUserUUID().ToString();
  packet.skillId = id;
  packet.bUsing = true;

  static MapleServer* server = MapleServer::GetInstance();
  server->BroadCast(packet);
#endif
}
#endif
```
```c++
// 몹 마다 존재하는 히트박스 컴포넌트.
SH_USER_API void MobHitbox::OnTriggerEnter(Collider& other)
{
#if SH_SERVER
  if (!core::IsValid(mob) || collider == nullptr)
    return;

  GameObject& obj = other.gameObject;
  auto skillHitboxPtr = obj.GetComponent<SkillHitbox>();
  if (!core::IsValid(skillHitboxPtr))
    return;

  Player* playerPtr = skillHitboxPtr->GetPlayer();
  Skill* skillPtr = skillHitboxPtr->GetSkill();
  if (core::IsValid(playerPtr) && core::IsValid(skillPtr))
    mob->Hit(*skillPtr, *playerPtr); // 몹에게 Hit함수를 호출
#endif
}
```
```c++
// 서버측 코드
SH_USER_API void Mob::Hit(Skill& skill, Player& player)
{
  SH_INFO_FORMAT("hit from {}", player.GetUserUUID().ToString());

  MobHitPacket packet{};
  packet.skillId = skill.GetId();
  packet.mobUUID = GetUUID().ToString(); // 몹은 모든 플레이어에게 UUID는 똑같게 보임
  packet.userUUID = player.GetUserUUID().ToString();

  MapleServer::GetInstance()->BroadCast(packet); // 브로드캐스팅

  // 경직, 넉백 효과
  bStun = true;
  stunCount = 1000;

  const auto& playerPos = player.gameObject.transform->GetWorldPosition();
  const auto& mobPos = gameObject.transform->GetWorldPosition();
  float dx = (mobPos.x - playerPos.x) < 0 ? -1.f : 1.f;

  auto v = rigidbody->GetLinearVelocity();
  rigidbody->SetLinearVelocity({ 0.f, v.y, v.z });
  rigidbody->AddForce({ dx * 100.f, 0.f, 0.f });
}
```

</details>

몹의 스폰은 특이한 방식으로 이뤄지는데, 맵에 처음 배치한 몹이 '스포너' 객체가 됩니다.</br>
스포너 객체는 서버나 클라이언트 어디에서든 UUID가 같다는 점을 이용해 해당 위치에서 젠이 되는 몹에게 서버,클라이언트가 동일한 UUID를 부여합니다.</br>

<details><summary>코드 펼치기</summary>

```c++
// 서버측 Awake코드
if (bSpawner) // 처음 만들어진 객체는 bSpawner = true
{
  clone = Prefab::CreatePrefab(gameObject); // 자기 복사본을 만들고
  SpawnMob(); // 스폰 할 때 씁니다.
  gameObject.SetActive(false); // 자신은 비활성화 됩니다.
}
else
{
  SH_INFO("Hello!"); // 일반 객체가 소환 됐으므로
  MobSpawnPacket packet{}; // 몹 스폰 패킷을 클라이언트에 브로드캐스팅 합니다.
  packet.spawnerUUID = spawnerUUID.ToString();
  packet.mobUUID = GetUUID().ToString(); // 몹 UUID를 담아 클라이언트도 동일한 몹을 생성하게 합니다.

  MapleServer::GetInstance()->BroadCast(packet);
}
```
```c++
// 클라이언측에서 패킷을 받으면 몹을 생성
void Mob::ProcessMobSpawn(const MobSpawnPacket& packet)
{
  if (!bSpawner) // 스포너가 아니면 무시
    return;
  if (packet.spawnerUUID != GetUUID().ToString()) // 이 스포너가 아니라도 무시
    return;

  if (clone == nullptr)
    return;

  auto cloneObj = clone->AddToWorld(world);
  auto mob = cloneObj->GetComponent<Mob>();
  mob->SetUUID(core::UUID{ packet.mobUUID }); // 패킷으로 받은 UUID를 스폰한 몹에게 부여 - 서버와 클라이언트가 같은 UUID를 가지게 됨
  mob->spawnerUUID = GetUUID();
  mob->bSpawner = false;
  mob->serverPos = { gameObject.transform->GetWorldPosition() };
  
  if (core::IsValid(rigidbody))
    mob->serverVel = { rigidbody->GetLinearVelocity() };
}
```

</details>

## 패킷 ID 자동 부여
패킷마다 ID를 부여하는 것이 중요한데, 프로그래머가 일일이 부여하게 되면 중복 되는 경우가 생길 수도 있어서 자동으로 부여하게 했습니다.</br>
패킷 클래스의 이름을 컴파일 시간에 fnv-1a 해시 알고리즘을 이용해 해시값으로 변환하여 저장합니다.</br>

<details><summary>코드 펼치기</summary>

```c++
class MobStatePacket : public network::Packet
{
  SPACKET(MobStatePacket, ID) // 엔진측에 패킷을 등록하는 매크로
public:
  constexpr static uint32_t ID = (core::reflection::TypeTraits::GetTypeHash<MobStatePacket>() >> 32); // 클래스 이름으로 해시를 만든다.
  ...
}
```

</details>
 
## 애니메이션
사실 이 부분은 엔진에서 지원해야 하는 부분이지만, 아직 구현이 안 돼서 컴포넌트 기반으로 해결했습니다.</br>
코드를 통해 유한 상태 기계를 짜고 에디터에서 편집 할 수 있으면 더 완벽해 질 것으로 보이며, 추후 엔진에 구현 할 예정입니다.</br>

<details><summary>코드 펼치기</summary>

```c++
// 애니메이션을 지정하고..
PROPERTY(idle)
Animation* idle = nullptr;
PROPERTY(walk)
Animation* walk = nullptr;
PROPERTY(jump)
Animation* jump = nullptr;
PROPERTY(prone)
Animation* prone = nullptr;
...
// 현재 포즈에 맞춰 애니메이션 컴포넌트의 Play함수를 호출합니다.
switch (pose)
{
case Pose::Idle:
  if (core::IsValid(idle))
  {
    curAnim = idle;
    idle->Play(*meshRenderer);
  }
  break;
case Pose::Walk:
  if (core::IsValid(walk))
  {
    curAnim = walk;
    walk->Play(*meshRenderer);
  }
  break;
case Pose::Jump:
  if (core::IsValid(jump))
  {
    curAnim = jump;
    jump->Play(*meshRenderer);
  }
  break;
case Pose::Prone:
  if (core::IsValid(prone))
  {
    curAnim = prone;
    prone->Play(*meshRenderer);
  }
  break;
}
```
```c++
// 애니메이션은 루프를 돌며 텍스쳐를 변경합니다.
...
if (t >= delay / 1000.f)
{
  if (bLoop)
  {
    idx = (idx + 1) % textures.size();
    if (delays.size() > idx)
      delay = delays[idx];
  }
  else
  {
    idx = (idx + 1);
    if (idx >= textures.size())
    {
      idx = textures.size() - 1;
      bPlaying = false;
    }
  }
  t = 0.0f;
}
if (meshRenderer.IsValid())
  meshRenderer->GetMaterialPropertyBlock()->SetProperty("tex", textures[idx]);
```

</<details>
