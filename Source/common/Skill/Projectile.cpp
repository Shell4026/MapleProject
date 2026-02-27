#include "Skill/Projectile.h"
#include "Skill/ProjectileInstance.h"

#include "Game/GameObject.h"
namespace sh::game
{
    SH_USER_API void Projectile::SpawnProjectile(World& world, Entity* owner, float x, float y, bool bRight)
    {
        if (prefab == nullptr)
            return;
        GameObject* const objPtr = prefab->AddToWorld(world);
        objPtr->transform->SetWorldPosition(x, y, 0.2f);
        if (bRight)
        {
            SH_INFO("right");
            objPtr->transform->SetRotation({ 0.f, 180.f, 0.f });
        }
        objPtr->transform->UpdateMatrix();
        ProjectileInstance* instancePtr = objPtr->GetComponent<ProjectileInstance>();
        instancePtr->Init(*this, owner);
    }
}//namespace