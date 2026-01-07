#include "Item/ItemDB.h"

#include "Core/SObject.h"

#include "Game/TextObject.h"
namespace sh::game
{
    auto ItemDB::GetItemInfo(int id) const -> const core::Json*
    {
        auto it = items.find(id);
        if (it == items.end())
            return nullptr;
        return &it->second;
    }
    ItemDB::ItemDB()
	{
        TextObject* itemDB = static_cast<TextObject*>(core::SObject::GetSObjectUsingResolver(core::UUID{ "5a5005649a0ad5aef3872acee30b2cfb" }));
        if (itemDB == nullptr)
        {
            SH_ERROR("ItemDB is not valid!");
            return;
        }
        core::Json itemJson = core::Json::parse(itemDB->text);
        if (!itemJson.contains("items"))
            return;

        for (const auto& json : itemJson["items"])
        {
            if (!json.contains("id"))
                continue;
            int id = json["id"];
            items[id] = json;
        }
	}
}//namespace