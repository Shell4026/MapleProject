#pragma once
#include "Export.h"

#include "Core/EventSubscriber.h"

#include "Network/PacketEvent.hpp"

#include "Game/Component/Component.h"
#include "Game/Component/Render/MeshRenderer.h"
#include "Game/Component/Phys/RigidBody.h"

#include "Render/Texture.h"

#include <cstdint>
namespace sh::game
{
	class PlayerJoinPacket;
	class FootholdMovement;
	class MapleWorld;

	class Item : public Component
	{
		COMPONENT(Item, "user")
	public:
		SH_USER_API Item(GameObject& owner);

		SH_USER_API void Awake() override;
		SH_USER_API void OnDisable() override;
		SH_USER_API void FixedUpdate() override;
		SH_USER_API void Update() override;

		SH_USER_API void SetTexture(const render::Texture* texture);
		SH_USER_API void SetCurrentWorld(MapleWorld& world);

		SH_USER_API auto GetTexture() const -> const render::Texture* { return texture; }
		SH_USER_API auto GetRigidBody() const -> RigidBody* { return rb; }
		SH_USER_API auto GetCurrentWorld() const -> MapleWorld* { return mapleWorld; }
		SH_USER_API auto GetMovement() const -> FootholdMovement* { return movement; }
	public:
		static constexpr const char* PREFAB_UUID = "ee1bee2efec5a690531dd2812a6192b7";
	public:
		PROPERTY(itemId)
		int itemId = 0;
		core::UUID owner = core::UUID::GenerateEmptyUUID();
		uint64_t instanceId = 0;
	private:
		PROPERTY(texture)
		const render::Texture* texture = nullptr;
		PROPERTY(renderer)
		MeshRenderer* renderer = nullptr;
		PROPERTY(rb)
		RigidBody* rb = nullptr;
		PROPERTY(mapleWorld, core::PropertyOption::sobjPtr, core::PropertyOption::invisible)
		MapleWorld* mapleWorld = nullptr;
		PROPERTY(movement, core::PropertyOption::sobjPtr)
		FootholdMovement* movement = nullptr;

		core::EventSubscriber<network::PacketEvent> packetSubscriber;
	};
}//namespace