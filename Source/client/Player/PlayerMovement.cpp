#include "Player/PlayerMovement.h"
#include "MapleWorld.h"
#include "MapleClient.h"

#include "Game/World.h"
#include "Game/Input.h"
namespace sh::game
{
	// 클라이언트 코드
	SH_USER_API void PlayerMovement::Awake()
	{
		if (player == nullptr)
			SH_ERROR("Player is nullptr!");
		if (player->IsLocal())
		{
			// 이벤트들은 BeginUpdate전에 이뤄짐
			packetSubscriber.SetCallback(
				[this](const network::PacketEvent& evt)
				{
					if (evt.packet->GetId() == PlayerStatePacket::ID)
						Reconciliation(static_cast<const PlayerStatePacket&>(*evt.packet));
				}
			);
			MapleClient::GetInstance()->bus.Subscribe(packetSubscriber);
		}
		else
		{

		}
		foothold = player->GetCurrentWorld()->GetFoothold();
	}
	SH_USER_API void PlayerMovement::BeginUpdate()
	{
		if (player->IsLocal())
			ProcessLocalInput();
	}
	SH_USER_API void PlayerMovement::FixedUpdate()
	{
		ApplyGravity();
		ApplyPos();
		CheckGround();
		++tick;
	}
	SH_USER_API void PlayerMovement::Update()
	{
	}
	void sh::game::PlayerMovement::ProcessLocalInput()
	{
		static MapleClient& client = *MapleClient::GetInstance();

		if (bInputLock)
			return;

		int xInput = 0;
		bool bJump = false;
		bool bProne = false;

		if (Input::GetKeyDown(Input::KeyCode::Down))
			bProne = true;
		if (Input::GetKeyDown(Input::KeyCode::F))
			bJump = true;
	
		if (Input::GetKeyDown(Input::KeyCode::Right))
			xInput += 1;
		if (Input::GetKeyDown(Input::KeyCode::Left))
			xInput -= 1;

		const bool bInputChanged = 
			bJump != lastInput.bJump ||
			bProne != lastInput.bProne ||
			xInput != lastInput.xMove;

		lastInput.xMove = xInput;
		lastInput.bJump = bJump;
		lastInput.bProne = bProne;

		if (bInputChanged)
		{
			SH_INFO("send");
			PlayerInputPacket packet{};
			packet.user = client.GetUser().GetUserUUID();
			packet.seq = inputSeqCounter++;
			packet.tick = tick;
			packet.inputX = xInput;
			packet.bJump = bJump;
			packet.bProne = bProne;
			client.SendPacket(packet);

			SentState state{};
			state.seq = packet.seq;
			state.tick = tick;
			state.xMove = packet.inputX;
			state.bProne = packet.bProne;
			state.bJump = packet.bJump;
			sentStates.push_back(state);
		}

		// 예측
		if (xInput > 0)
			velX = speed;
		else if (xInput < 0)
			velX = -speed;
		else
			velX = 0.f;

		if (bJump && bGround)
		{
			velY = jumpSpeed;
			bGround = false;
		}
	}
	void PlayerMovement::Reconciliation(const PlayerStatePacket& packet)
	{
		const auto& pos = gameObject.transform->GetWorldPosition();
		gameObject.transform->SetWorldPosition(packet.px, packet.py, pos.z);
		gameObject.transform->UpdateMatrix();

		velX = packet.vx;
		velY = packet.vy;
		bGround = packet.bGround;
		bProne = packet.bProne;
		bInputLock = packet.bLock;

		const auto& curPos = gameObject.transform->GetWorldPosition();
		ground = foothold->GetExpectedFallContact({ curPos.x, curPos.y + offset });

		if (sentStates.empty())
		{
			SentState base{};
			base.seq = 0;
			base.tick = 0;
			base.xMove = 0;
			base.bJump = false;
			base.bProne = false;
			sentStates.push_back(base);
		}

		while (sentStates.size() > 1)
		{
			const bool bAcked = (sentStates[1].seq <= packet.lastProcessedInputSeq);
			const bool bBeforeOrAtSnapshot = (sentStates[1].tick <= packet.serverTick);
			if (bAcked && bBeforeOrAtSnapshot)
				sentStates.pop_front();
			else
				break;
		}

		if (packet.serverTick >= tick) // 서버가 제일 최신 상태임
			return;

		std::size_t stateIdx = 0;
		while (stateIdx + 1 < sentStates.size() && sentStates[stateIdx + 1].tick <= packet.serverTick)
			++stateIdx;

		for (uint64_t t = packet.serverTick; t < tick; ++t)
		{
			while (stateIdx + 1 < sentStates.size() && sentStates[stateIdx + 1].tick <= t)
				++stateIdx;
	
			SentState& pastState = sentStates[stateIdx];
	
			if (pastState.xMove > 0)
				velX = speed;
			else if (pastState.xMove < 0)
				velX = -speed;
			else
				velX = 0.f;

			if (pastState.bJump && bGround)
			{
				bGround = false;
				velY = jumpSpeed;
			}

			ApplyGravity();
			ApplyPos();
			CheckGround();
		}
	}
}//namespace