#include "EntityRouter.h"
#include "Player/PlayerMovement.h"
#include "Player/PlayerPickup.h"
#include "Packet/PlayerInputPacket.hpp"
#include "Packet/KeyPacket.hpp"

#include "Game/Input.h"
namespace sh::game
{
	EntityRouter::EntityRouter()
	{
		packetSubscriber.SetCallback(
			[this](const network::PacketEvent& evt)
			{
				if (evt.packet->GetId() == PlayerInputPacket::ID)
				{
					const auto& packet = static_cast<const PlayerInputPacket&>(*evt.packet);
					Player* player = GetPlayer(packet.user);
					if (player != nullptr)
						player->GetMovement()->ProcessInput(packet);
				}
				else if (evt.packet->GetId() == KeyPacket::ID)
				{
					const auto& keyPacket = static_cast<const KeyPacket&>(*evt.packet);

					if (keyPacket.keycode == static_cast<int>(Input::KeyCode::Z))
					{
						Player* player = GetPlayer(keyPacket.userUUID);
						if (player == nullptr)
							return;

						if (keyPacket.bPressed)
							player->GetPickup()->SetPickupState(true);
						else
							player->GetPickup()->SetPickupState(false);
					}
				}
			}
		);
	}
	SH_USER_API void EntityRouter::RegisterPlayer(Player& player)
	{
		auto it = players.find(player.GetUserUUID());
		if (it != players.end())
			return;
		players.insert({ player.GetUserUUID(), &player });
	}
	SH_USER_API void EntityRouter::UnRegisterPlayer(Player* player)
	{
		if (player == nullptr)
			return;
		players.erase(player->GetUserUUID());
	}
	SH_USER_API void EntityRouter::UnRegisterPlayer(const core::UUID& userUUID)
	{
		players.erase(userUUID);
	}
	SH_USER_API auto sh::game::EntityRouter::GetPlayer(const core::UUID& uuid) const -> Player*
	{
		auto it = players.find(uuid);
		if (it == players.end())
			return nullptr;
		return it->second;
	}
}//namespace