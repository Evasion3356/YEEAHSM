#include "ForceStoreOnDismount.h"
#include "Logger.h"

#include <natives.h>

namespace
{
    bool g_enabled = false;
}

namespace ForceStoreOnDismount
{
    void Tick()
    {
        if (!g_enabled)
            return;

        Ped ped = PLAYER::PLAYER_PED_ID();
        WEAPON::_SET_INSTANTLY_STORE_LONGARMS_ON_DISMOUNT(ped, true, 0);
    }

    bool Toggle()
    {
        g_enabled = !g_enabled;
        Logger::Log(g_enabled
            ? "== ForceStoreOnDismount ENABLED (forcing WeaponComponent+0x1B5 via 0xB832F1A686B9B810) =="
            : "== ForceStoreOnDismount DISABLED ==");
        return g_enabled;
    }

    bool IsEnabled()
    {
        return g_enabled;
    }
}
