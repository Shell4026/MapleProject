#include "User.h"

namespace sh::game
{
	User::User(const std::string& ip, uint16_t port) :
		ip(ip), port(port), 
		uuid(core::UUID::Generate()), currentWorld(core::UUID::GenerateEmptyUUID())
	{
	}
	User::User(std::string&& ip, uint16_t port) :
		ip(std::move(ip)), port(port), 
		uuid(core::UUID::Generate()), currentWorld(core::UUID::Generate())
	{
	}
	User::User(const User& other) :
		ip(other.ip), port(other.port),
		uuid(other.uuid), currentWorld(other.currentWorld)
	{
	}
	User::User(User&& other) noexcept :
		ip(other.ip), port(other.port),
		uuid(std::move(other.uuid)), currentWorld(std::move(other.currentWorld))
	{
	}
	SH_USER_API void User::SetNickname(const std::string& name)
	{
		nickname = name;
	}
	SH_USER_API void User::SetNickname(std::string&& name)
	{
		nickname = std::move(name);
	}
	SH_USER_API auto User::GetNickName() const -> const std::string&
	{
		return nickname;
	}
	SH_USER_API void User::SetUserUUID(const core::UUID& uuid)
	{
		this->uuid = uuid;
	}
	SH_USER_API auto User::GetUserUUID() const -> const core::UUID&
	{
		return uuid;
	}
	SH_USER_API void User::SetCurrentWorldUUID(const core::UUID& worldUUID)
	{
		currentWorld = worldUUID;
	}
	SH_USER_API auto User::GetCurrentWorldUUID() const -> const core::UUID&
	{
		return currentWorld;
	}
}//namespace