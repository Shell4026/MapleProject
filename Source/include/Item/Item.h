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

	class Item : public Component
	{
		COMPONENT(Item, "user")
	public:
		SH_USER_API Item(GameObject& owner);

		SH_USER_API void Awake() override;
		SH_USER_API void OnDisable() override;
		SH_USER_API void BeginUpdate() override;

		SH_USER_API void SetTexture(const render::Texture* texture);

		SH_USER_API auto GetTexture() const -> const render::Texture* { return texture; }
		SH_USER_API auto GetRigidBody() const -> RigidBody* { return rb; }
	public:
		static constexpr const char* PREFAB_UUID = "ee1bee2efec5a690531dd2812a6192b7";
	public:
		PROPERTY(itemId)
		int itemId;
		core::UUID owner = core::UUID::GenerateEmptyUUID();
		uint64_t instanceId = 0;
	private:
		PROPERTY(texture)
		const render::Texture* texture = nullptr;
		PROPERTY(renderer)
		MeshRenderer* renderer = nullptr;
		PROPERTY(rb)
		RigidBody* rb = nullptr;
		PROPERTY(trigger)
		RigidBody* trigger = nullptr;

		core::EventSubscriber<network::PacketEvent> packetSubscriber;
	};
}//namespace