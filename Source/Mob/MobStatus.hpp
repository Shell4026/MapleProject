#pragma once

#include <cstdint>
namespace sh::game
{
    struct MobStatus
    {
        uint32_t hp = 15;
        uint32_t maxHp = 15;

        float stunRemainingMs = 0.f;

        bool bStun = false;

        void Reset(uint32_t inMaxHp)
        {
            maxHp = inMaxHp;
            hp = inMaxHp;
            bStun = false;
            stunRemainingMs = 0.f;
        }

        void Tick(float dt)
        {
            if (!bStun) 
                return;
            stunRemainingMs -= dt * 1000.f;
            if (stunRemainingMs <= 0.f)
            {
                bStun = false;
                stunRemainingMs = 0.f;
            }
        }

        void ApplyDamage(uint32_t dmg)
        {
            if (dmg >= hp) 
                hp = 0;
            else hp -= dmg;
        }

        void ApplyStun(float ms)
        {
            bStun = true;
            if (ms > stunRemainingMs) 
                stunRemainingMs = ms;
        }
    };
}//namespace