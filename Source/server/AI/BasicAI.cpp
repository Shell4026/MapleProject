#include "AI/BasicAI.h"
#include "Mob/Mob.h"

#include "Game/GameObject.h"
#include "Game/World.h"
#include "Game/Component/Phys/RigidBody.h"

namespace sh::game
{
	std::mt19937 BasicAI::rng{ std::random_device{}() };

	BasicAI::BasicAI(GameObject& owner) :
		AIStrategy(owner)
	{
	}
	SH_USER_API void BasicAI::Run(Mob& mob)
	{
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

		RigidBody* rigidBody = mob.GetRigidbody();
		if (!core::IsValid(rigidBody))
			return;

		if (state == State::Move || state == State::Chase)
		{
			auto vel = rigidBody->GetLinearVelocity();
			vel.x = dir * mob.GetSpeed();
			rigidBody->SetLinearVelocity(vel);
		}
		else
		{
			auto vel = rigidBody->GetLinearVelocity();
			vel.x = 0.f;
			rigidBody->SetLinearVelocity(vel);
		}
	}
	SH_USER_API void BasicAI::OnAttacked(Player& player)
	{
		
	}
	SH_USER_API auto BasicAI::GetState() const -> uint32_t
	{
		return static_cast<uint32_t>(state);
	}
	SH_USER_API void BasicAI::Reset()
	{
		state = State::Idle;
		stateTimer = 0.0f;
		dir = 0.0f;
		target = nullptr;
	}
	void BasicAI::UpdateIdle()
	{
		if (stateTimer <= 0.0f)
		{
			std::uniform_int_distribution<int> dist(0, 1);
			if (dist(rng) == 0)
				ChangeState(State::Idle);
			else
				ChangeState(State::Move);
		}
	}
	void BasicAI::UpdateMove()
	{
		if (stateTimer <= 0.0f)
		{
			std::uniform_int_distribution<int> dist(0, 1);
			if (dist(rng) == 0)
				ChangeState(State::Idle);
			else
				ChangeState(State::Move);
		}
	}
	void BasicAI::UpdateChase(Mob& mob)
	{
		if (!target.IsValid())
		{
			target = nullptr;
			ChangeState(State::Idle);
			return;
		}

		float dx = target->gameObject.transform->GetWorldPosition().x - mob.gameObject.transform->GetWorldPosition().x;
		if (std::fabs(dx) < 0.05f)
		{
			dir = 0.0f;
		}
		else
		{
			dir = (dx > 0) ? 1.0f : -1.0f;
		}
	}
	void BasicAI::ChangeState(State state)
	{
		this->state = state;
		switch (state)
		{
		case State::Idle:
		{
			std::uniform_real_distribution<float> distf(idleRange.x, idleRange.y);
			stateTimer = distf(rng);
			dir = 0.0f;
			break;
		}
		case State::Move:
		{
			std::uniform_real_distribution<float> distf(walkRange.x, walkRange.y);
			stateTimer = distf(rng);
			std::uniform_int_distribution<int> dist(-1, 1);
			dir = static_cast<float>(dist(rng));
			break;
		}
		case State::Chase:
		{
			stateTimer = 9999.0f;
			break;
		}
		}//switch
	}
}//namespace