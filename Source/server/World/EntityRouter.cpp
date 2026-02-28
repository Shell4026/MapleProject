#include "World/EntityRouter.h"
#include "World/MapleWorld.h"
#include "Player/PlayerMovement.h"
#include "Player/PlayerPickup.h"
#include "Skill/SkillManager.h"
#include "Packet/PlayerInputPacket.hpp"
#include "Packet/KeyPacket.hpp"
#include "Packet/SkillUsingPacket.hpp"

#include "Game/Input.h"
namespace sh::game
{
	EntityRouter::EntityRouter()
	{
		packetSubscriber.SetCallback(
			[this](const network::PacketEvent& evt)
			{
				const uint32_t packetId = evt.packet->GetId();
				if (packetId == PlayerInputPacket::ID)
				{
					const auto& packet = static_cast<const PlayerInputPacket&>(*evt.packet);
					Player* const player = GetPlayer(packet.user);
					if (player != nullptr)
					{
						if (packet.bUp)
						{
							if (MapleWorld* currentWorld = player->GetCurrentWorld(); currentWorld != nullptr)
								SH_INFO_FORMAT("transfer: {}", currentWorld->TryTransferByPortal(*player));
						}
					}
				}
				else if (packetId == KeyPacket::ID)
				{
					const auto& keyPacket = static_cast<const KeyPacket&>(*evt.packet);

					if (keyPacket.keycode == static_cast<int>(Input::KeyCode::Z))
					{
						Player* const player = GetPlayer(keyPacket.userUUID);
						if (player == nullptr)
							return;

						if (keyPacket.bPressed)
							player->GetPickup()->SetPickupState(true);
						else
							player->GetPickup()->SetPickupState(false);
					}
				}
				else if (packetId == SkillUsingPacket::ID)
				{
					const auto& packet = static_cast<const SkillUsingPacket&>(*evt.packet);
					Player* const player = GetPlayer(packet.userUUID);
					if (player != nullptr)
						player->GetSkillManager()->ProcessPacket(packet);
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
