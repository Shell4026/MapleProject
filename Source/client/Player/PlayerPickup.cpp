#include "Player/PlayerPickup.h"
#include "Packet/KeyPacket.hpp"
#include "MapleClient.h"

#include "Game/GameObject.h"
#include "Game/Input.h"

namespace sh::game
{
	PlayerPickup::PlayerPickup(GameObject& owner) :
		Component(owner)
	{
	}
	SH_USER_API void PlayerPickup::Awake()
	{
		player = gameObject.GetComponent<Player>();
		if (player == nullptr)
		{
			SH_ERROR("Not found Player component!");
		}
	}
	SH_USER_API void PlayerPickup::BeginUpdate()
	{
		if (player == nullptr || !player->IsLocal())
			return;
		if (Input::GetKeyPressed(Input::KeyCode::Z))
		{
			KeyPacket packet{};
			packet.userUUID = player->GetUserUUID();
			packet.keycode = static_cast<int>(Input::KeyCode::Z);
			packet.bPressed = true;
			MapleClient::GetInstance()->SendPacket(packet);
		}
		else if (Input::GetKeyReleased(Input::KeyCode::Z))
		{
			KeyPacket packet{};
			packet.userUUID = player->GetUserUUID();
			packet.keycode = static_cast<int>(Input::KeyCode::Z);
			packet.bPressed = false;
			MapleClient::GetInstance()->SendPacket(packet);
		}
	}
	SH_USER_API void PlayerPickup::Update()
	{
	}
	SH_USER_API void PlayerPickup::OnTriggerEnter(Collider& collider)
	{
	}
	SH_USER_API void PlayerPickup::OnTriggerExit(Collider& collider)
	{
	}
}//namespace