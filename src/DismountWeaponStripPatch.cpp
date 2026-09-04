#include "DismountWeaponStripPatch.h"
#include "BytePatch.h"

namespace
{
    constexpr const char* kFindPattern = "8A 86 8D 01 00 00 A8 06 75 1C";

    BytePatch g_patch(
        "DismountWeaponStripGate",
        kFindPattern,
        { 0x30, 0xC0, 0x0F, 0x1F, 0x40, 0x00}); // xor al,al ; nop x4

}

namespace DismountWeaponStripPatch
{
    bool Install(bool quiet)
    {
        if (!g_patch.Resolve(quiet))
            return false;

        if (!g_patch.Apply())
        {
            Logger::Log("DismountWeaponStripGate: resolved but failed to apply.");
            return false;
        }

        Logger::Log("DismountWeaponStripGate: applied (weapons should stay on dismount).");
        return true;
    }

    void Remove()
    {
        if (g_patch.IsApplied())
            g_patch.Revert();
    }
}
