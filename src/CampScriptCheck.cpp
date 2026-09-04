#include "CampScriptCheck.h"
#include "Logger.h"

#include <natives.h>

namespace CampScriptCheck
{
    bool ShouldDiscard(const char* context)
    {
        BOOL loaded = SCRIPT::HAS_SCRIPT_LOADED("player_camp");
        BOOL exists = SCRIPT::DOES_SCRIPT_EXIST("player_camp");

        Logger::LogFormatted(
            "CampScriptCheck[%s]: HAS_SCRIPT_LOADED(\"player_camp\")=%d  DOES_SCRIPT_EXIST(\"player_camp\")=%d  (deciding via HAS_SCRIPT_LOADED)",
            context, (int)loaded, (int)exists);

        return loaded != 0;
    }
}
