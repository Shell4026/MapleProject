#include "BasicAI.h"
#include "../Mob.h"

#include "Game/GameObject.h"
#include "Game/World.h"
#include "Game/Component/RigidBody.h"

namespace sh::game
{
	BasicAI::BasicAI(GameObject& owner) :
		AIStrategy(owner)
	{
		rng.seed(std::random_device{}());
	}
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

		RigidBody* rigidBody = mob.GetRigidbody();
		if (!core::IsValid(rigidBody))
			return;

		PlayerAnimation* anim = mob.GetAnim();

		if (state == State::Move || state == State::Chase)
		{
			auto vel = rigidBody->GetLinearVelocity();
			vel.x = dir * mob.GetSpeed();
			rigidBody->SetLinearVelocity(vel);

			if (core::IsValid(anim))
				anim->SetPose(PlayerAnimation::Pose::Walk);
		}
		else
		{
			auto vel = rigidBody->GetLinearVelocity();
			vel.x = 0.f;
			rigidBody->SetLinearVelocity(vel);

			if (core::IsValid(anim))
				anim->SetPose(PlayerAnimation::Pose::Idle);
		}
#endif
	}
	SH_USER_API void BasicAI::OnAttacked(Player& player)
	{
	}
	SH_USER_API auto BasicAI::GetState() const -> uint32_t
	{
		return static_cast<uint32_t>(state);
	}
	void BasicAI::UpdateIdle()
	{
#if SH_SERVER
		if (stateTimer <= 0.0f)
		{
			std::uniform_int_distribution<int> dist(0, 1);
			if (dist(rng) == 0)
				ChangeState(State::Idle);
			else
				ChangeState(State::Move);
		}
#endif
	}
	void BasicAI::UpdateMove()
	{
#if SH_SERVER
		if (stateTimer <= 0.0f)
		{
			std::uniform_int_distribution<int> dist(0, 1);
			if (dist(rng) == 0)
				ChangeState(State::Idle);
			else
				ChangeState(State::Move);
		}
#endif
	}
	void BasicAI::UpdateChase(Mob& mob)
	{
#if SH_SERVER
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
#endif
	}
	void BasicAI::ChangeState(State state)
	{
#if SH_SERVER
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
#endif
	}
}//namespace