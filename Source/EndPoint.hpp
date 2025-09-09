#pragma once
#include "Core/Util.h"

#include <string>
#include <cstdint>
struct Endpoint
{
	std::string ip;
	uint16_t port;

	auto operator==(const Endpoint& other) const noexcept -> bool
	{
		return ip == other.ip && port == other.port;
	}
};

namespace std
{
	template<>
	struct std::hash<Endpoint>
	{
		auto operator()(const Endpoint& ep) const -> std::size_t
		{
			return sh::core::Util::CombineHash(std::hash<std::string>{}(ep.ip), std::hash<uint16_t>{}(ep.port));
		}
	};
}//namespace