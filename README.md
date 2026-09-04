# YEEAHS

**Y**et **A**nother **H**orse **S**tow mod... except this one actually works.

Red Dead Redemption 2 keeps taking Arthur's weapons away and stowing them on his horse — when
you dismount, and (via a completely separate internal code path) when you walk into camp. There
are dozens of existing mods on Nexus that claim to fix this; none of the ones tested here did it
correctly. YEEAHS patches the actual shared internal engine function responsible for the
behavior, found through live reverse engineering rather than guessing at script-level
workarounds.

Curious how it was actually found? Read **[JOURNEY.md](JOURNEY.md)** — a full, honest account of
the investigation: the dead ends, the wrong turns, and who (human or AI) was responsible for
each step.

## What it does

Hooks `sub_14089EE14` — the single internal function, confirmed live via a manual `RETN` patch
in a debugger, that both the horse-dismount weapon-strip system and the camp-arrival
weapon-strip system ultimately call to physically move a weapon onto the horse — and discards
every call to it. Your weapons stay exactly where you put them, whether you dismount or walk
into camp.

## Install

1. Build `YEEAHS.asi` (see below), or grab a release build if one's provided.
2. Drop it into your Red Dead Redemption 2 install folder, next to your ASI loader
   (e.g. `dinput8.dll`).

No other runtime dependencies — MinHook is statically linked in, and the mod doesn't touch
ScriptHookRDR2 or any other third-party DLL at all.

## Build

Requires Visual Studio 2022 or newer with the Desktop C++ workload.

```
git clone --recurse-submodules <this repo>
```

Open `YEEAHS.sln`, build `Release|x64`. Output lands in `bin\x64\Release\YEEAHS.asi`.

(`--recurse-submodules` matters — MinHook is vendored as a git submodule under
`deps/minhook`, pinned to a specific upstream commit rather than copy-pasted in.)

## Why it needs a retry loop to apply

RDR2's executable is encrypted at rest and only reaches its final, patchable in-memory form
after some runtime unpacking step completes — which may not have finished by the time an ASI
loader injects this DLL. `PatchWorker` retries hook installation on a background thread every
500ms for up to 2 minutes rather than assuming the first attempt will succeed. A 10+ specific
byte AOB pattern matching against still-packed memory by chance is effectively impossible, so
this only guards against false negatives (not there *yet*), never false positives.

## Project layout

| File | Purpose |
|---|---|
| `src/main.cpp` | `DllMain` — starts/stops the patch worker, nothing else |
| `src/PatchWorker.*` | Background retry thread, safe against the unpacking race |
| `src/StowWeaponsHook.*` | The actual fix — hooks and discards `sub_14089EE14` |
| `src/PatternScan.*` | Minimal AOB scanner (PE-header-based, no psapi dependency) |
| `src/NativeSigHook.h` | Generic MinHook wrapper for a single native/internal function |
| `src/Logger.*` | Thread-safe file logger (`YEEAHS.log`, next to the `.asi`) |

Several investigation-only files (`DismountWeaponStripPatch`, `LongarmsStoreOnDismountHooks`,
`CampScriptCheck`, `HidePedWeaponsHook`, `SetCurrentPedWeaponHook`, and others) remain on disk
but are excluded from the build — they're kept for reference since they're part of the actual
investigation trail documented in `JOURNEY.md`, not because they're needed at runtime.

## Credits

Reverse engineering, live debugging, and the actual breakthrough: **gir489**.
Tooling, scaffolding, and script archaeology: **Claude Code** (Anthropic).
See [JOURNEY.md](JOURNEY.md) for the detailed, step-by-step attribution.
