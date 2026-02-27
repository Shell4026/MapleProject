#include "Player/PlayerMovement.h"
#include "MapleWorld.h"
#include "MapleClient.h"

#include "Game/World.h"
#include "Game/Input.h"
namespace sh::game
{
	// 클라 사이드
	SH_USER_API void PlayerMovement::Awake()
	{
		serverPos = { gameObject.transform->GetWorldPosition().x, gameObject.transform->GetWorldPosition().y };

		if (player == nullptr)
			SH_ERROR("Player is nullptr!");

		// 이벤트들은 BeginUpdate전에 이뤄짐
		packetSubscriber.SetCallback(
			[this](const network::PacketEvent& evt)
			{
				if (evt.packet->GetId() == PlayerStatePacket::ID)
				{
					auto& packet = static_cast<const PlayerStatePacket&>(*evt.packet);
					if (player->GetUUID() == packet.playerUUID)
					{
						if (player->IsLocal())
							Reconciliation(packet);
						else
							ProcessRemote(packet);
					}
				}
			}
		);
		MapleClient::GetInstance()->bus.Subscribe(packetSubscriber);
	}
	SH_USER_API void PlayerMovement::BeginUpdate()
	{
		if (player->IsLocal())
			ProcessLocalInput();
		else
			InterpolateRemote();
	}
	SH_USER_API void PlayerMovement::FixedUpdate()
	{
		if (!player->IsLocal())
			return;

		// 예측
		if (!bProne)
		{
			if (lastInput.xMove > 0)
			{
				bRight = true;
				AddForce(14.f, 0.f);
				//vel.x = GetSpeed();
			}
			else if (lastInput.xMove < 0)
			{
				bRight = false;
				AddForce(-14.f, 0.f);
				//vel.x = -GetSpeed();
			}
			//else
			//	vel.x = 0.f;

			if (lastInput.bJump && IsGround())
			{
				vel.y = GetJumpSpeed();
				SetIsGround(false);
			}
		}
		else
		{
			vel.x = 0.f;
		}

		StepMovement();

		StateHistory state{};
		state.seq = lastInput.seq;
		state.tick = tick;
		state.pos = { gameObject.transform->GetWorldPosition().x, gameObject.transform->GetWorldPosition().y };
		state.vel = vel;
		state.xMove = lastInput.xMove;
		state.bProne = lastInput.bProne;
		state.bJump = lastInput.bJump;
		history.push_back(state);
		while (history.size() > 180)
			history.pop_front();

		++tick;
	}
	SH_USER_API void PlayerMovement::Update()
	{
	}
	void PlayerMovement::ProcessLocalInput()
	{
		int xInput = 0;
		bool bJump = false;
		bProne = false;

		if (Input::GetKeyDown(Input::KeyCode::Down))
			bProne = true;
		if (Input::GetKeyDown(Input::KeyCode::F))
			bJump = true;
	
		if (Input::GetKeyDown(Input::KeyCode::Right))
			xInput += 1;
		if (Input::GetKeyDown(Input::KeyCode::Left))
			xInput -= 1;

		if (bInputLock)
		{
			xInput = 0;
			bProne = false;
			bJump = false;
		}

		const bool bChanged =
			bJump != lastInput.bJump ||
			bProne != lastInput.bProne ||
			xInput != lastInput.xMove;

		if (bChanged)
		{
			static MapleClient& client = *MapleClient::GetInstance();
			curSeq = nextSeq++;

			PlayerInputPacket packet{};
			packet.user = client.GetUser().GetUserUUID();
			packet.seq = curSeq;
			packet.tick = tick;
			packet.inputX = xInput;
			packet.bJump = bJump;
			packet.bProne = bProne;

			//SH_INFO_FORMAT("send seq: {}, tick: {}", packet.seq, packet.tick);
			client.SendPacket(packet);

			bPendingSend = false;
		}

		lastInput.seq = curSeq;
		lastInput.xMove = xInput;
		lastInput.bJump = bJump;
		lastInput.bProne = bProne;
	}
	void PlayerMovement::Reconciliation(const PlayerStatePacket& packet)
	{
		if (history.empty())
			return;
		if (packet.lastProcessedInputSeq < history.front().seq) // 오래된 패킷임
		{
			//SH_INFO_FORMAT("packet seq: {}, seq: {}", packet.lastProcessedInputSeq, history.front().seq);
			return;
		}

		while (!history.empty() && history.front().seq < packet.lastProcessedInputSeq)
			history.pop_front();

		if (history.empty())
		{
			//SH_INFO("empty");
			return;
		}

		const uint64_t targetTick = packet.clientTickAtState;
		auto it = std::lower_bound(history.begin(), history.end(), targetTick,
			[](const StateHistory& history, uint64_t tick) { return history.tick < tick; });

		if (it == history.end() || it->tick != targetTick)
			return;

		const auto& pastHistory = *it;
		const float dx = (pastHistory.pos.x - packet.px);
		const float dy = (pastHistory.pos.y - packet.py);
		const float difSqr = dx * dx + dy * dy;
		if (difSqr < 0.05f * 0.05f) // 5픽셀 오차까진 허용
			return;

		// 즉시 이동
		{
			const auto& oldPos = gameObject.transform->GetWorldPosition();
			gameObject.transform->SetWorldPosition(packet.px, packet.py, oldPos.z);
			gameObject.transform->UpdateMatrix();

			vel.x = packet.vx;
			vel.y = packet.vy;
			SetIsGround(packet.bGround);
			bProne = packet.bProne;
			bInputLock = packet.bLock;
			bRight = packet.bRight;

			SetExpectedGround();
		}
		// 리플레이
		for (uint64_t t = std::distance(history.begin(), it); t < history.size(); ++t)
		{
			StateHistory& lastHistory = history[t];

			if (lastHistory.xMove > 0)
			{
				AddForce(14.f, 0.f);
				//vel.x = GetSpeed();
				bRight = true;
			}
			else if (lastHistory.xMove < 0)
			{
				AddForce(-14.f, 0.f);
				//vel.x = -GetSpeed();
				bRight = false;
			}
			//else
			//	vel.x = 0.f;

			if (lastHistory.bJump && IsGround())
			{
				SetIsGround(false);
				vel.y = GetJumpSpeed();
			}
			bProne = lastHistory.bProne;

			StepMovement();

			lastHistory.pos = { gameObject.transform->GetWorldPosition().x, gameObject.transform->GetWorldPosition().y };
			lastHistory.vel = vel;
		}
	}
	void PlayerMovement::ProcessRemote(const PlayerStatePacket& packet)
	{
		serverPos = { packet.px, packet.py };
		vel.x = packet.vx;
		vel.y = packet.vy;
		bProne = packet.bProne;
		if (vel.x > 0.01f)
			bRight = true;
		else if (vel.x < -0.01f)
			bRight = false;

		SetIsGround(packet.bGround);
	}
	void PlayerMovement::InterpolateRemote()
	{
		auto pos = gameObject.transform->GetWorldPosition();
		pos = glm::mix(glm::vec2{ pos }, glm::vec2{ serverPos }, 0.2f);
		gameObject.transform->SetWorldPosition(pos);
		gameObject.transform->UpdateMatrix();
	}
}//namespace