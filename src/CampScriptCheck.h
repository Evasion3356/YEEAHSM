#pragma once

// Calls SCRIPT::HAS_SCRIPT_LOADED("player_camp") and
// SCRIPT::DOES_SCRIPT_EXIST("player_camp") via the normal ScriptHookRDR2
// invoke<>() path, called INLINE from inside our MinHook detours. Safe
// because the detour fires synchronously as part of the game's own native
// dispatch -- we're on the same thread/context real script natives expect,
// even though this DLL never registered its own script thread.
//
// Logs both results every call so we can see which one (if either) actually
// tracks "player_camp is loaded" once real data comes in -- the discard
// decision currently uses HAS_SCRIPT_LOADED as primary, per its doc comment
// in natives.h ("Returns if a script has been loaded into the game").
namespace CampScriptCheck
{
    // Logs both native results (tagged with `context`) and returns whether
    // the caller should discard/no-op its native call.
    bool ShouldDiscard(const char* context);
}
