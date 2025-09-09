#pragma once
#include "Export.h"

#include "Core/UUID.h"

#include <string>
#include <cstdint>
namespace sh::game
{
	class User
	{
	public:
		SH_USER_API User(const std::string& ip, uint16_t port);
		SH_USER_API User(std::string&& ip, uint16_t port);

		SH_USER_API void SetNickname(const std::string& name);
		SH_USER_API void SetNickname(std::string&& name);
		SH_USER_API auto GetNickName() const -> const std::string&;
		SH_USER_API void SetUUID(const core::UUID& uuid);
		SH_USER_API auto GetUUID() const -> const core::UUID&;
		SH_USER_API void SetCurrentWorldUUID(const core::UUID& worldUUID);
		SH_USER_API auto GetCurrentWorldUUID() const -> const core::UUID&;
	public:
		const std::string ip;
		const uint16_t port;
	private:
		core::UUID uuid;
		core::UUID currentWorld;

		std::string nickname;
	};
}//namepsace