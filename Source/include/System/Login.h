#pragma once
#include "Export.h"
#if !SH_SERVER
#include "World/MapleClient.h"

#include "Game/Component/Component.h"

#include <string>
namespace sh::game
{
	class Login : public Component
	{
		COMPONENT(Login, "user")
	public:
		SH_USER_API Login(GameObject& owner);

		SH_USER_API void Start() override;
		SH_USER_API void Update() override;

		SH_USER_API void Deserialize(const core::Json& json) override;
	private:
		std::string nickname;

		PROPERTY(nextWorld)
		World* nextWorld = nullptr;
	};
}//namespace
#endif
