#pragma once
#if !SH_SERVER
#include "Export.h"
#include "User.h"

#include "Game/Component/UdpClient.h"

#include "Core/EventBus.h"
namespace sh::game
{
	class MapleClient : public UdpClient
	{
		COMPONENT(MapleClient, "user")
	public:
		SH_USER_API MapleClient(GameObject& owner);

		SH_USER_API void OnDestroy() override;
		SH_USER_API void Awake() override;
		SH_USER_API void Start() override;
		SH_USER_API void BeginUpdate() override;
		SH_USER_API void Update() override;

		SH_USER_API static auto GetUser() -> User&;
		SH_USER_API static auto GetInstance() -> MapleClient*;
	public:
		core::EventBus bus;
	private:
		static User user;
		static MapleClient* instance;
	};
}//namespace
#endif
