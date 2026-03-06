#include "Player/PlayerMovement.h"
#include "World/MapleWorld.h"
#include "World/MapleClient.h"
#include "Skill/SkillManager.h"

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

	SH_USER_API void PlayerMovement::TickBegin(uint64_t tick)
	{
		if (player->IsLocal())
			ProcessLocalInput(tick);
		else
			InterpolateRemote();
	}
	SH_USER_API void PlayerMovement::TickFixed(uint64_t tick)
	{
		bJumpTriggeredThisTick = false;
		bLandedThisTick = false;

		if (!player->IsLocal())
		{
			skillMoveThisTick = {};
			return;
		}

		if (state.bLock)
		{
			state.xMove = 0;
			state.bJump = false;
			state.bUp = false;
			state.bProne = false;
		}

		const bool bWasGround = IsGround();
		// 예측
		if (!state.bProne)
		{
			if (state.xMove > 0)
			{
				bRight = true;
				AddForce(14.f, 0.f);
				//vel.x = GetSpeed();
			}
			else if (state.xMove < 0)
			{
				bRight = false;
				AddForce(-14.f, 0.f);
				//vel.x = -GetSpeed();
			}
			//else
			//	vel.x = 0.f;

			if (state.bJump && IsGround())
			{
				vel.y = GetJumpSpeed();
				SetIsGround(false);
				bJumpTriggeredThisTick = true;
			}
		}

		StepMovement();
		if (!bWasGround && IsGround())
			bLandedThisTick = true;

		StateHistory stateHistory{};
		stateHistory.tick = tick;
		stateHistory.pos = { gameObject.transform->GetWorldPosition().x, gameObject.transform->GetWorldPosition().y };
		stateHistory.vel = vel;
		stateHistory.state = state;
		stateHistory.skillMove = skillMoveThisTick;

		history.push_back(stateHistory);
		while (history.size() > 180)
			history.pop_front();

		skillMoveThisTick = {};
	}
	SH_USER_API void PlayerMovement::TickUpdate(uint64_t tick)
	{
	}
	void PlayerMovement::ProcessLocalInput(uint64_t tick)
	{
		int xInput = 0;
		bool bJump = Input::GetKeyDown(Input::KeyCode::F);
		bool bUp = Input::GetKeyDown(Input::KeyCode::Up);
		bool bProne = Input::GetKeyDown(Input::KeyCode::Down);
	
		if (Input::GetKeyDown(Input::KeyCode::Right))
			xInput += 1;
		if (Input::GetKeyDown(Input::KeyCode::Left))
			xInput -= 1;

		if (state.bLock)
		{
			xInput = 0;
			bProne = false;
			bJump = false;
			bUp = false;
		}

		const bool bChanged =
			bJump != state.bJump ||
			bUp != state.bUp ||
			bProne != state.bProne ||
			xInput != state.xMove;

		state.seq = curSeq;
		state.xMove = xInput;
		state.bJump = bJump;
		state.bUp = bUp;
		state.bProne = bProne;

		if (bChanged)
		{
			static MapleClient& client = *MapleClient::GetInstance();
			curSeq = nextSeq++;

			PlayerInputPacket packet{};
			packet.user = client.GetUser().GetUserUUID();
			packet.seq = state.seq;
			packet.tick = tick;
			packet.inputX = state.xMove;
			packet.bJump = state.bJump;
			packet.bUp = state.bUp;
			packet.bProne = state.bProne;

			//SH_INFO_FORMAT("send seq: {}, tick: {}", packet.seq, packet.tick);
			client.SendPacket(packet);

			bPendingSend = false;
		}
	}
	void PlayerMovement::Reconciliation(const PlayerStatePacket& packet)
	{
		if (history.empty())
			return;
		if (packet.lastProcessedInputSeq < history.front().state.seq) // 오래된 패킷임
		{
			//SH_INFO_FORMAT("packet seq: {}, seq: {}", packet.lastProcessedInputSeq, history.front().seq);
			return;
		}

		while (!history.empty() && history.front().state.seq < packet.lastProcessedInputSeq)
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
		if (difSqr < 0.03f * 0.03f) // 3픽셀 오차까진 허용
			return;

		// 즉시 이동
		{
			const auto& oldPos = gameObject.transform->GetWorldPosition();
			gameObject.transform->SetWorldPosition(packet.px, packet.py, oldPos.z);
			gameObject.transform->UpdateMatrix();

			vel.x = packet.vx;
			vel.y = packet.vy;
			SetIsGround(packet.bGround);
			state.bLock = packet.bLock;
			state.bProne = packet.bProne;
			state.bUp = packet.bUp;
			bRight = packet.bRight;

			SetExpectedGround();
		}
		const uint64_t startTick = static_cast<uint64_t>(std::distance(history.begin(), it));
		// 리플레이
		for (uint64_t t = startTick; t < history.size(); ++t)
		{
			StateHistory& lastHistory = history[t];

			state = lastHistory.state;
			if (lastHistory.skillMove.bTeleport)
			{
				const auto& curPos = gameObject.transform->GetWorldPosition();
				gameObject.transform->SetWorldPosition(
					curPos.x + lastHistory.skillMove.teleportDelta.x,
					curPos.y + lastHistory.skillMove.teleportDelta.y,
					curPos.z
				);
				gameObject.transform->UpdateMatrix();
				vel.x = 0.f;
				vel.y = 0.f;
				if (lastHistory.skillMove.bSetExpectedGround)
					SetExpectedGround();
			}
			if (lastHistory.skillMove.bImpulse)
				SetVelocity(lastHistory.skillMove.impulse.x, lastHistory.skillMove.impulse.y);

			if (state.bLock)
			{
				state.xMove = 0;
				state.bJump = false;
				state.bUp = false;
				state.bProne = false;
			}
			
			if(!state.bProne)
			{
				if (lastHistory.state.xMove > 0)
				{
					AddForce(14.f, 0.f);
					//vel.x = GetSpeed();
					bRight = true;
				}
				else if (lastHistory.state.xMove < 0)
				{
					AddForce(-14.f, 0.f);
					//vel.x = -GetSpeed();
					bRight = false;
				}

				if (lastHistory.state.bJump && IsGround())
				{
					SetIsGround(false);
					vel.y = GetJumpSpeed();
				}
			}
			
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
		state.bUp = packet.bUp;
		state.bProne = packet.bProne;
		bRight = packet.bRight;

		if (SkillManager* const skillManager = player->GetSkillManager(); skillManager != nullptr)
			skillManager->SyncRemoteState(packet.skillId, packet.bSkillUsing);

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
