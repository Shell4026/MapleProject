#pragma once
#include "../Export.h"
#include "AIStrategy.h"

#include "Core/SContainer.hpp"

#include "Game/Component/Component.h"
#include "Game/Vector.h"

#include <random>
namespace sh::game
{
	/// @brief FSM기반 기본 AI
	class BasicAI : public AIStrategy
	{
		COMPONENT(BasicAI, "user")
	public:
		SH_USER_API BasicAI(GameObject& owner);

		SH_USER_API void Run(Mob& mob) override;
		SH_USER_API void OnAttacked(Player& player) override;
		SH_USER_API auto GetState() const -> uint32_t override;
	private:
		enum class State
		{
			Idle,
			Move,
			Attack,
			Chase
		};
		void UpdateIdle();
		void UpdateMove();
		void UpdateChase(Mob& mob);
		void ChangeState(State state);
	private:
		PROPERTY(idleRange)
		game::Vec2 idleRange{ 1.f, 3.f };
		PROPERTY(walkRange)
		game::Vec2 walkRange{ 1.f, 5.f };

		State state = State::Idle;

		float stateTimer = 0.0f;
		float dir = 0.0f;

		std::mt19937 rng;

		core::SObjWeakPtr<Player> target = nullptr;
	};
}//namespace