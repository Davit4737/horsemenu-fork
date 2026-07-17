#pragma once
#include "core/frontend/Notifications.hpp"
#include "game/backend/FiberPool.hpp"
#include "game/backend/ScriptMgr.hpp"
#include "game/rdr/Entity.hpp"
#include "game/rdr/Natives.hpp"

namespace YimMenu
{
    Entity SpawnObjectEntity(uint32_t hash, Vector3 coords, float heading = 0.f);
    void SpawnObject(uint32_t hash, Vector3 coords);
}
