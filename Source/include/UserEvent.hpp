#pragma once
#include "Core/IEvent.h"
#include "Core/Reflection/TypeTraits.hpp"

#include "Network/Packet.h"

#include <memory>

namespace sh::game
{
	class UserEvent : public core::IEvent
	{
	public:
		auto GetTypeHash() const -> std::size_t
		{
			return core::reflection::TypeTraits::GetTypeHash<UserEvent>();
		};
	public:
		enum class Type
		{
			JoinUser,
			LeaveUser,
			KickUser
		} type;
		const User* user = nullptr;
	};
}//namespace;