#include "Login.h"
#include "Packet/PlayerJoinPacket.h"
#include "MapleClient.h"

#include "Game/ImGUImpl.h"
#include "Game/World.h"
#include "Game/GameManager.h"

namespace sh::game
{
	Login::Login(GameObject& owner) :
		NetworkComponent(owner)
	{
		ImGui::SetCurrentContext(world.GetUiContext().GetContext());
	}
	SH_USER_API void Login::Start()
	{
		
	}
	SH_USER_API void Login::Update()
	{
		const float windowWidth = world.renderer.GetWidth();
		const float windowHeight = world.renderer.GetHeight();
		const float width = 500;
		const float height = 300;

		ImGui::SetNextWindowPos({ windowWidth / 2 - width / 2, windowHeight / 2 - height / 2 }, ImGuiCond_::ImGuiCond_Appearing);
		ImGui::SetNextWindowSize({ width, height }, ImGuiCond_::ImGuiCond_Appearing);
		ImGui::Begin("Login", nullptr, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove);

		ImGui::Text(u8"닉네임");
		ImGui::InputText("##inputNickname", &nickname);
		if (ImGui::Button(u8"접속"))
		{
			auto client = MapleClient::GetInstance();
			assert(client != nullptr);
			PlayerJoinPacket packet{};
			packet.SetNickname(nickname);

			client->SendPacket(packet);
		}

		ImGui::End();

		world.GetUiContext().SyncDirty();
	}
	SH_USER_API void Login::Deserialize(const core::Json& json)
	{
		Super::Deserialize(json);
	}
}//namespace