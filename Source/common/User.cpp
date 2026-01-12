#include "User.h"

namespace sh::game
{
	User::User(int64_t id, const std::string& ip, uint16_t port, const core::UUID& uuid) :
		id(id), ip(ip), port(port), uuid(uuid),
		currentWorld(core::UUID::GenerateEmptyUUID())
	{
	}
	User::User(int64_t id, std::string&& ip, uint16_t port, const core::UUID& uuid) :
		id(id), ip(std::move(ip)), port(port), uuid(uuid),
		currentWorld(core::UUID::Generate())
	{
	}
	User::User(const User& other) :
		id(other.id), ip(other.ip), port(other.port),
		uuid(other.uuid), currentWorld(other.currentWorld)
	{
	}
	User::User(User&& other) noexcept :
		id(other.id), ip(other.ip), port(other.port),
		uuid(std::move(other.uuid)), currentWorld(std::move(other.currentWorld))
	{
	}
	SH_USER_API auto User::operator=(const User& other) -> User&
	{
		id = other.id;
		ip = other.ip;
		port = other.port;
		uuid = other.uuid;
		currentWorld = other.currentWorld;
		nickname = other.nickname;
		return *this;
	}
	SH_USER_API auto User::operator=(User&& other) noexcept -> User&
	{
		id = other.id;
		ip = std::move(other.ip);
		port = other.port;
		uuid = other.uuid;
		currentWorld = other.currentWorld;
		nickname = std::move(other.nickname);
		return *this;
	}
	SH_USER_API void User::SetNickname(const std::string& name)
	{
		nickname = name;
	}
	SH_USER_API void User::SetNickname(std::string&& name) noexcept
	{
		nickname = std::move(name);
	}
	SH_USER_API void User::SetCurrentWorldUUID(const core::UUID& worldUUID)
	{
		currentWorld = worldUUID;
	}
}//namespace