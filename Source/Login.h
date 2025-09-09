#pragma once
#include "Export.h"
#if !SH_SERVER
#include "MapleClient.h"

#include "Game/Component/NetworkComponent.h"

#include <string>
namespace sh::game
{
	class Login : public NetworkComponent
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
