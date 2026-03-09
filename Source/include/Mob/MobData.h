#pragma once
#include "Export.h"

#include "Sound/SoundClip.h"

#include "Game/ScriptableObject.h"

#include <cstdint>
#include <string>
namespace sh::game
{
    class MobData : public ScriptableObject
    {
        SRPO(MobData)
    public:
        SH_USER_API auto GetMobId() const -> int { return mobId; }
        SH_USER_API auto GetMaxHp() const -> uint32_t { return maxHp; }
#if !SH_SERVER
        SH_USER_API auto GetMobName() const -> const std::string& { return mobName; }
        SH_USER_API auto GetDieSound() const -> const  sound::SoundClip* { return dieSound; }
#endif
    private:
        PROPERTY(mobId)
        int mobId = 0;
        PROPERTY(maxHp)
        uint32_t maxHp = 10;
#if !SH_SERVER
        PROPERTY(mobName)
        std::string mobName = "Unknown";
        PROPERTY(dieSound)
        const sound::SoundClip* dieSound = nullptr;
#endif
    };
}//namespace
